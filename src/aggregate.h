
// Compiler implementation of the D programming language
// Copyright (c) 1999-2013 by Digital Mars
// All Rights Reserved
// written by Walter Bright
// http://www.digitalmars.com
// License for redistribution is by either the Artistic License
// in artistic.txt, or the GNU General Public License in gnu.txt.
// See the included readme.txt for details.

#ifndef DMD_AGGREGATE_H
#define DMD_AGGREGATE_H

#ifdef __DMC__
#pragma once
#endif /* __DMC__ */

#include "root.h"

#include "dsymbol.h"
#include "declaration.h"

class Identifier;
class Type;
class TypeFunction;
class Expression;
class FuncDeclaration;
class CtorDeclaration;
class DtorDeclaration;
class InvariantDeclaration;
class NewDeclaration;
class DeleteDeclaration;
class InterfaceDeclaration;
class TypeInfoClassDeclaration;
class VarDeclaration;
struct dt_t;

enum Sizeok
{
    SIZEOKnone,         // size of aggregate is not computed yet
    SIZEOKdone,         // size of aggregate is set correctly
    SIZEOKfwd,          // error in computing size of aggregate
};

class AggregateDeclaration : public ScopeDsymbol
{
public:
    Type *type;
    StorageClass storage_class;
    PROT protection;
    Type *handle;               // 'this' type
    unsigned structsize;        // size of struct
    unsigned alignsize;         // size of struct for alignment purposes
    int hasUnions;              // set if aggregate has overlapping fields
    VarDeclarations fields;     // VarDeclaration fields
    Sizeok sizeok;         // set when structsize contains valid data
    Dsymbol *deferred;          // any deferred semantic2() or semantic3() symbol
    bool isdeprecated;          // !=0 if deprecated

#if DMDV2
    Dsymbol *enclosing;         /* !=NULL if is nested
                                 * pointing to the dsymbol that directly enclosing it.
                                 * 1. The function that enclosing it (nested struct and class)
                                 * 2. The class that enclosing it (nested class only)
                                 * 3. If enclosing aggregate is template, its enclosing dsymbol.
                                 * See AggregateDeclaraton::makeNested for the details.
                                 */
    VarDeclaration *vthis;      // 'this' parameter if this aggregate is nested
#endif
    // Special member functions
    FuncDeclarations invs;              // Array of invariants
    FuncDeclaration *inv;               // invariant
    NewDeclaration *aggNew;             // allocator
    DeleteDeclaration *aggDelete;       // deallocator

#if DMDV2
    Dsymbol *ctor;                      // CtorDeclaration or TemplateDeclaration
    CtorDeclaration *defaultCtor;       // default constructor - should have no arguments, because
                                        // it would be stored in TypeInfo_Class.defaultConstructor
    Dsymbol *aliasthis;                 // forward unresolved lookups to aliasthis
    bool noDefaultCtor;         // no default construction
#endif

    FuncDeclarations dtors;     // Array of destructors
    FuncDeclaration *dtor;      // aggregate destructor

    Expression *getRTInfo;      // pointer to GC info generated by object.RTInfo(this)

    AggregateDeclaration(Loc loc, Identifier *id);
    void setScope(Scope *sc);
    void semantic2(Scope *sc);
    void semantic3(Scope *sc);
    void inlineScan();
    unsigned size(Loc loc);
    static void alignmember(structalign_t salign, unsigned size, unsigned *poffset);
    static unsigned placeField(unsigned *nextoffset,
        unsigned memsize, unsigned memalignsize, structalign_t memalign,
        unsigned *paggsize, unsigned *paggalignsize, bool isunion);
    Type *getType();
    int firstFieldInUnion(int indx); // first field in union that includes indx
    int numFieldsInUnion(int firstIndex); // #fields in union starting at index
    bool isDeprecated();         // is aggregate deprecated?
    FuncDeclaration *buildDtor(Scope *sc);
    FuncDeclaration *buildInv(Scope *sc);
    bool isNested();
    void makeNested();
    bool isExport();

    void emitComment(Scope *sc);
    void toJson(JsonOut *json);
    void toDocBuffer(OutBuffer *buf, Scope *sc);

    FuncDeclaration *hasIdentityOpAssign(Scope *sc);
    FuncDeclaration *hasIdentityOpEquals(Scope *sc);

    const char *mangle(bool isv = false);

    // For access checking
    virtual PROT getAccess(Dsymbol *smember);   // determine access to smember
    int isFriendOf(AggregateDeclaration *cd);
    int hasPrivateAccess(Dsymbol *smember);     // does smember have private access to members of this class?
    void accessCheck(Loc loc, Scope *sc, Dsymbol *smember);

    PROT prot();

    // Back end
    Symbol *stag;               // tag symbol for debug data
    Symbol *sinit;
    Symbol *toInitializer();

    AggregateDeclaration *isAggregateDeclaration() { return this; }
};

class StructDeclaration : public AggregateDeclaration
{
public:
    int zeroInit;               // !=0 if initialize with 0 fill
#if DMDV2
    int hasIdentityAssign;      // !=0 if has identity opAssign
    int hasIdentityEquals;      // !=0 if has identity opEquals
    FuncDeclaration *cpctor;    // generated copy-constructor, if any
    FuncDeclarations postblits; // Array of postblit functions
    FuncDeclaration *postblit;  // aggregate postblit

    FuncDeclaration *xeq;       // TypeInfo_Struct.xopEquals
    FuncDeclaration *xcmp;      // TypeInfo_Struct.xopCmp
    static FuncDeclaration *xerreq;      // object.xopEquals
    static FuncDeclaration *xerrcmp;     // object.xopCmp

    structalign_t alignment;    // alignment applied outside of the struct
#endif

    // For 64 bit Efl function call/return ABI
    Type *arg1type;
    Type *arg2type;

    StructDeclaration(Loc loc, Identifier *id);
    Dsymbol *syntaxCopy(Dsymbol *s);
    void semantic(Scope *sc);
    Dsymbol *search(Loc, Identifier *ident, int flags);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    const char *mangle(bool isv = false);
    const char *kind();
    void finalizeSize(Scope *sc);
    bool isPOD();
#if DMDV1
    Expression *cloneMembers();
#endif
#if DMDV2
    int needOpAssign();
    int needOpEquals();
    FuncDeclaration *buildOpAssign(Scope *sc);
    FuncDeclaration *buildPostBlit(Scope *sc);
    FuncDeclaration *buildCpCtor(Scope *sc);
    FuncDeclaration *buildOpEquals(Scope *sc);
    FuncDeclaration *buildXopEquals(Scope *sc);
    FuncDeclaration *buildXopCmp(Scope *sc);
#endif
    void toDocBuffer(OutBuffer *buf, Scope *sc);

    PROT getAccess(Dsymbol *smember);   // determine access to smember

    void toObjFile(int multiobj);                       // compile to .obj file
    void toDt(dt_t **pdt);
    void toDebug();                     // to symbolic debug info

    StructDeclaration *isStructDeclaration() { return this; }
};

class UnionDeclaration : public StructDeclaration
{
public:
    UnionDeclaration(Loc loc, Identifier *id);
    Dsymbol *syntaxCopy(Dsymbol *s);
    const char *kind();

    UnionDeclaration *isUnionDeclaration() { return this; }
};

struct BaseClass
{
    Type *type;                         // (before semantic processing)
    PROT protection;               // protection for the base interface

    ClassDeclaration *base;
    unsigned offset;                    // 'this' pointer offset
    FuncDeclarations vtbl;              // for interfaces: Array of FuncDeclaration's
                                        // making up the vtbl[]

    size_t baseInterfaces_dim;
    BaseClass *baseInterfaces;          // if BaseClass is an interface, these
                                        // are a copy of the InterfaceDeclaration::interfaces

    BaseClass();
    BaseClass(Type *type, PROT protection);

    int fillVtbl(ClassDeclaration *cd, FuncDeclarations *vtbl, int newinstance);
    void copyBaseInterfaces(BaseClasses *);
};

#if DMDV2
#define CLASSINFO_SIZE_64  0x98         // value of ClassInfo.size
#define CLASSINFO_SIZE  (0x3C+12+4)     // value of ClassInfo.size
#else
#define CLASSINFO_SIZE  (0x3C+12+4)     // value of ClassInfo.size
#define CLASSINFO_SIZE_64  (0x98)       // value of ClassInfo.size
#endif

struct ClassFlags
{
    typedef unsigned Type;
    enum Enum
    {
        isCOMclass = 0x1,
        noPointers = 0x2,
        hasOffTi = 0x4,
        hasCtor = 0x8,
        hasGetMembers = 0x10,
        hasTypeInfo = 0x20,
        isAbstract = 0x40,
        isCPPclass = 0x80,
    };
};

class ClassDeclaration : public AggregateDeclaration
{
public:
    static ClassDeclaration *object;
    static ClassDeclaration *throwable;
    static ClassDeclaration *exception;
    static ClassDeclaration *errorException;

    ClassDeclaration *baseClass;        // NULL only if this is Object
#if DMDV1
    CtorDeclaration *ctor;
    CtorDeclaration *defaultCtor;       // default constructor
#endif
    FuncDeclaration *staticCtor;
    FuncDeclaration *staticDtor;
    Dsymbols vtbl;                      // Array of FuncDeclaration's making up the vtbl[]
    Dsymbols vtblFinal;                 // More FuncDeclaration's that aren't in vtbl[]

    BaseClasses *baseclasses;           // Array of BaseClass's; first is super,
                                        // rest are Interface's

    size_t interfaces_dim;
    BaseClass **interfaces;             // interfaces[interfaces_dim] for this class
                                        // (does not include baseClass)

    BaseClasses *vtblInterfaces;        // array of base interfaces that have
                                        // their own vtbl[]

    TypeInfoClassDeclaration *vclassinfo;       // the ClassInfo object for this ClassDeclaration
    int com;                            // !=0 if this is a COM class (meaning
                                        // it derives from IUnknown)
#if DMDV2
    int cpp;                            // !=0 if this is a C++ interface
#endif
    int isscope;                        // !=0 if this is an auto class
    int isabstract;                     // !=0 if abstract class
    int inuse;                          // to prevent recursive attempts
    Semantic doAncestorsSemantic;  // Before searching symbol, whole ancestors should finish
                                        // calling semantic() at least once, due to fill symtab
                                        // and do addMember(). [== Semantic(Start,In,Done)]

    ClassDeclaration(Loc loc, Identifier *id, BaseClasses *baseclasses, bool inObject = false);
    Dsymbol *syntaxCopy(Dsymbol *s);
    void semantic(Scope *sc);
    void toCBuffer(OutBuffer *buf, HdrGenState *hgs);
    int isBaseOf2(ClassDeclaration *cd);

    #define OFFSET_RUNTIME 0x76543210
    virtual int isBaseOf(ClassDeclaration *cd, int *poffset);

    virtual int isBaseInfoComplete();
    Dsymbol *search(Loc, Identifier *ident, int flags);
    ClassDeclaration *searchBase(Loc, Identifier *ident);
#if DMDV2
    int isFuncHidden(FuncDeclaration *fd);
#endif
    FuncDeclaration *findFunc(Identifier *ident, TypeFunction *tf);
    void interfaceSemantic(Scope *sc);
    int isCOMclass();
    virtual int isCOMinterface();
#if DMDV2
    int isCPPclass();
    virtual int isCPPinterface();
#endif
    bool isAbstract();
    virtual int vtblOffset();
    const char *kind();
    const char *mangle(bool isv = false);
    void toDocBuffer(OutBuffer *buf, Scope *sc);

    PROT getAccess(Dsymbol *smember);   // determine access to smember

    void addLocalClass(ClassDeclarations *);

    // Back end
    void toObjFile(int multiobj);                       // compile to .obj file
    void toDebug();
    unsigned baseVtblOffset(BaseClass *bc);
    Symbol *toSymbol();
    Symbol *toVtblSymbol();
    void toDt(dt_t **pdt);
    void toDt2(dt_t **pdt, ClassDeclaration *cd);

    Symbol *vtblsym;

    ClassDeclaration *isClassDeclaration() { return (ClassDeclaration *)this; }
};

class InterfaceDeclaration : public ClassDeclaration
{
public:
    InterfaceDeclaration(Loc loc, Identifier *id, BaseClasses *baseclasses);
    Dsymbol *syntaxCopy(Dsymbol *s);
    void semantic(Scope *sc);
    int isBaseOf(ClassDeclaration *cd, int *poffset);
    int isBaseOf(BaseClass *bc, int *poffset);
    const char *kind();
    int isBaseInfoComplete();
    int vtblOffset();
#if DMDV2
    int isCPPinterface();
#endif
    virtual int isCOMinterface();

    void toObjFile(int multiobj);                       // compile to .obj file
    Symbol *toSymbol();

    InterfaceDeclaration *isInterfaceDeclaration() { return this; }
};

#endif /* DMD_AGGREGATE_H */
