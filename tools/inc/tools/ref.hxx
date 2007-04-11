/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ref.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:16:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _REF_HXX
#define _REF_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

//=========================================================================

#define PRV_SV_DECL_REF_SIGNATURE( ClassName, Ref )                     \
    inline               ClassName##Ref() { pObj = 0; }                 \
    inline               ClassName##Ref( const ClassName##Ref & rObj ); \
    inline               ClassName##Ref( ClassName * pObjP );           \
    inline void          Clear();                                       \
    inline               ~ClassName##Ref();                             \
    inline ClassName##Ref & operator = ( const ClassName##Ref & rObj ); \
    inline ClassName##Ref & operator = ( ClassName * pObj );            \
    inline BOOL            Is() const { return pObj != NULL; }          \
    inline ClassName *     operator &  () const { return pObj; }        \
    inline ClassName *     operator -> () const { return pObj; }        \
    inline ClassName &     operator *  () const { return *pObj; }       \
    inline operator ClassName * () const { return pObj; }

#define PRV_SV_IMPL_REF_COUNTERS( ClassName, Ref, AddRef, AddNextRef, ReleaseRef, Init, pRefbase ) \
inline ClassName##Ref::ClassName##Ref( const ClassName##Ref & rObj )        \
    { pObj = rObj.pObj; if( pObj ) { Init pRefbase->AddNextRef; } }         \
inline ClassName##Ref::ClassName##Ref( ClassName * pObjP )                  \
{ pObj = pObjP; if( pObj ) { Init pRefbase->AddRef; } }                     \
inline void ClassName##Ref::Clear()                                         \
{                                                                           \
    if( pObj )                                                              \
    {                                                                       \
        ClassName* const pRefObj = pRefbase;                                \
        pObj = 0;                                                           \
        pRefObj->ReleaseRef;                                                \
    }                                                                       \
}                                                                           \
inline ClassName##Ref::~ClassName##Ref()                                    \
{ if( pObj ) { pRefbase->ReleaseRef; } }                                    \
inline ClassName##Ref & ClassName##Ref::                                    \
            operator = ( const ClassName##Ref & rObj )                      \
{                                                                           \
    if( rObj.pObj ) rObj.pRefbase->AddNextRef;                              \
    ClassName* const pRefObj = pRefbase;                                    \
    pObj = rObj.pObj;                                                       \
    Init if( pRefObj ) { pRefObj->ReleaseRef; }                             \
    return *this;                                                           \
}                                                                           \
inline ClassName##Ref & ClassName##Ref::operator = ( ClassName * pObjP )    \
{ return *this = ClassName##Ref( pObjP ); }

#define PRV_SV_DECL_REF_LOCK(ClassName, Ref)    \
protected:                                      \
    ClassName * pObj;                           \
public:                                         \
PRV_SV_DECL_REF_SIGNATURE(ClassName, Ref)

#define PRV_SV_DECL_REF( ClassName )            \
PRV_SV_DECL_REF_LOCK( ClassName, Ref )

#define PRV_SV_DECL_LOCK( ClassName )           \
PRV_SV_DECL_REF_LOCK( ClassName, Lock )

#define SV_DECL_REF( ClassName )                \
class ClassName;                                \
class ClassName##Ref                            \
{                                               \
    PRV_SV_DECL_REF( ClassName )                \
};

#define SV_DECL_LOCK( ClassName )               \
class ClassName;                                \
class ClassName##Lock                           \
{                                               \
    PRV_SV_DECL_LOCK( ClassName )               \
};

#define SV_IMPL_REF( ClassName )                                \
PRV_SV_IMPL_REF_COUNTERS( ClassName, Ref, AddRef(), AddNextRef(),\
                          ReleaseReference(), EMPTYARG, pObj )

#define SV_IMPL_LOCK( ClassName )                                   \
PRV_SV_IMPL_REF_COUNTERS( ClassName, Lock, OwnerLock( TRUE ),       \
                          OwnerLock( TRUE ), OwnerLock( FALSE ),    \
                          EMPTYARG, pObj )

#define SV_DECL_COMPAT_REF( ClassName )         \
class ClassName;                                \
class ClassName##Ref                            \
{                                               \
protected:                                      \
    SvRefBase* pRefbase;                        \
    PRV_SV_DECL_REF( ClassName )                \
public:                                         \
    static void Destroy( void* );               \
};

#define SV_COMPAT_REF_INIT                                              \
pObj ? ( pRefbase =                                                     \
 (SfxPointerServer::GetServer()->HasPointer( pObj ) ? NULL :            \
 ((*(SvRefBase **)SfxPointerServer::GetServer()->CreatePointer(pObj))=  \
   new SvCompatRefBase( &Destroy, pObj ) ),                             \
   (SvRefBase *)SfxPointerServer::GetServer()->GetPointer(pObj))) : 0;

#define SV_IMPL_COMPAT_REF( ClassName )                                     \
PRV_SV_IMPL_REF_COUNTERS( ClassName, Ref, AddRef(), AddNextRef(),           \
                          ReleaseReference(), SV_COMPAT_REF_INIT, pRefbase )\
inline void ClassName##Ref::Destroy( void* pObj)                            \
{                                                                           \
    SfxPointerServer::GetServer()->ReleasePointer( pObj );                  \
    delete ( ClassName* ) pObj;                                             \
}

#define SV_DECL_IMPL_REF(ClassName)             \
    SV_DECL_REF(ClassName)                      \
    SV_IMPL_REF(ClassName)

#define SV_DECL_IMPL_LOCK( ClassName )          \
    SV_DECL_LOCK(ClassName)                     \
    SV_IMPL_LOCK(ClassName)

#define SV_DECL_IMPL_COMPAT_REF( ClassName )    \
    SV_DECL_COMPAT_REF(ClassName)               \
    SV_IMPL_COMPAT_REF(ClassName)

/************************** S v R e f L i s t ****************************/
#define PRV_SV_DECL_REF_LIST(CN,EN,vis) \
DECLARE_LIST(CN##List,EN)\
class vis CN##MemberList : public CN##List\
{\
public:\
inline CN##MemberList();\
inline CN##MemberList(USHORT nInitSz, USHORT nResize );\
inline CN##MemberList( const CN##MemberList & rRef );\
inline ~CN##MemberList();\
inline CN##MemberList & operator =\
       ( const CN##MemberList & rRef );\
inline void Clear();\
inline void Insert( EN p )\
{ CN##List::Insert( p ); p->AddRef();}\
inline void Insert( EN p, ULONG nIndex )\
{ CN##List::Insert( p, nIndex ); p->AddRef();}\
inline void Insert( EN p, EN pOld )\
{ CN##List::Insert( p, pOld ); p->AddRef();}\
inline void Append( EN p )\
{ Insert( p, LIST_APPEND );}\
inline EN   Remove();\
inline EN   Remove( ULONG nIndex );\
inline EN   Remove( EN p );\
inline EN   Replace( EN p );\
inline EN   Replace( EN p, ULONG nIndex );\
inline EN   Replace( EN pNew, EN pOld );\
inline void Append( const CN##MemberList & );\
};

#define SV_DECL_REF_LIST(CN,EN) \
PRV_SV_DECL_REF_LIST(CN,EN,/* empty */)
#define SV_DECL_REF_LIST_VISIBILITY(CN,EN,vis) \
PRV_SV_DECL_REF_LIST(CN,EN,vis)

/************************** S v R e f L i s t ****************************/
#define SV_IMPL_REF_LIST( CN, EN ) \
inline CN##MemberList::CN##MemberList(){}\
inline CN##MemberList::CN##MemberList(USHORT nInitSz, USHORT nResize )\
    : CN##List( nInitSz, nResize ){}\
inline CN##MemberList::CN##MemberList( const CN##MemberList & rRef ) \
      : CN##List( rRef ) \
{\
    ULONG nOldCount = Count(); \
    EN pEntry = First(); \
    while( pEntry ) \
    { pEntry->AddRef(); pEntry = Next(); } \
    Seek( nOldCount ); /* auch Curser gleich */ \
}\
inline CN##MemberList::~CN##MemberList() { Clear(); } \
inline CN##MemberList & CN##MemberList::operator = \
                    ( const CN##MemberList & rRef ) \
{\
    CN##MemberList & rList = (CN##MemberList &)rRef; \
    ULONG nOldCount = rList.Count(); \
    /* Count der Objekte erhoehen */ \
    EN pEntry = rList.First(); \
    while( pEntry ) \
    { pEntry->AddRef(); pEntry = rList.Next(); } \
    rList.Seek( nOldCount ); /* Curser zurueck */ \
    /* Liste kopieren */ \
    Clear(); \
    CN##List::operator = ( rRef ); \
    return *this; \
}\
inline void        CN##MemberList::Clear() \
{\
    EN pEntry = Last();\
    while( NULL != pEntry )\
        pEntry = Remove();\
}\
inline EN CN##MemberList::Remove() \
{\
    EN p = CN##List::Remove(); \
    if( p ) p->ReleaseReference(); return p; \
}\
inline EN CN##MemberList::Remove( ULONG nIndex ) \
{\
    EN p = CN##List::Remove( nIndex ); \
    if( p ) p->ReleaseReference(); return p; \
}\
inline EN CN##MemberList::Remove( EN p ) \
{\
    p = CN##List::Remove( p ); \
    if( p ) p->ReleaseReference(); return p; \
}\
inline EN CN##MemberList::Replace( EN p ) \
{\
    p->AddRef(); p = CN##List::Replace( p ); \
    if( p ) p->ReleaseReference(); return p; \
}\
inline EN CN##MemberList::Replace( EN p, ULONG nIndex ) \
{\
    p->AddRef(); p = CN##List::Replace( p, nIndex ); \
    if( p ) p->ReleaseReference(); return p; \
}\
inline EN CN##MemberList::Replace( EN pNew, EN pOld ) \
{\
    pNew->AddRef(); CN##List::Replace( pNew, pOld ); \
    if( pOld ) pOld->ReleaseReference(); return pOld; \
}\
inline void CN##MemberList::Append( const CN##MemberList & rList )\
{\
    for( ULONG i = 0; i < rList.Count(); i++ )\
        Append( rList.GetObject( i ) );\
}

/************************** S V _ D E C L _ R E F _ L I S T **************/
#define SV_DECL_IMPL_REF_LIST(ClassName,EntryName) \
    SV_DECL_REF_LIST(ClassName,EntryName) \
    SV_IMPL_REF_LIST(ClassName,EntryName)

/************************** S v M e m b e r L i s t **********************/
#define PRV_SV_DECL_MEMBER_LIST(Class,EntryName)        \
       Class##MemberList() {}                           \
inline Class##MemberList(USHORT nInitSz,USHORT nResize);\
inline void Insert( EntryName p );                      \
inline void Insert( EntryName p, ULONG nIndex );        \
inline void Insert( EntryName p, EntryName pOld );      \
inline void Append( EntryName p );                      \
inline EntryName   Remove();                            \
inline EntryName   Remove( ULONG nIndex );              \
inline EntryName   Remove( EntryName p );               \
inline EntryName   Replace( EntryName p );              \
inline EntryName   Replace( EntryName p, ULONG nIndex );\
inline EntryName   Replace( EntryName pNew, EntryName pOld );\
inline EntryName   GetCurObject() const;\
inline EntryName   GetObject( ULONG nIndex ) const;\
inline ULONG       GetPos( const EntryName ) const;\
inline ULONG       GetPos( const EntryName, ULONG nStartIndex,\
                           BOOL bForward = TRUE ) const;\
inline EntryName Seek( ULONG nIndex );\
inline EntryName Seek( EntryName p );\
inline EntryName First();\
inline EntryName Last();\
inline EntryName Next();\
inline EntryName Prev();\
inline void      Append( const Class##MemberList & rList );

#define PRV_SV_IMPL_MEMBER_LIST(ClassName,EntryName,BaseList)\
inline ClassName##MemberList::ClassName##MemberList\
                    (USHORT nInitSz,USHORT nResize)\
            : BaseList( nInitSz, nResize ){}\
inline void ClassName##MemberList::Insert( EntryName p )\
            {BaseList::Insert(p);}\
inline void ClassName##MemberList::Insert( EntryName p, ULONG nIdx )\
            {BaseList::Insert(p,nIdx);}\
inline void ClassName##MemberList::Insert( EntryName p, EntryName pOld )\
            {BaseList::Insert(p,pOld);}\
inline void ClassName##MemberList::Append( EntryName p )\
            {BaseList::Append(p);}\
inline EntryName ClassName##MemberList::Remove()\
            {return (EntryName)BaseList::Remove();}\
inline EntryName ClassName##MemberList::Remove( ULONG nIdx )\
            {return (EntryName)BaseList::Remove(nIdx);}\
inline EntryName ClassName##MemberList::Remove( EntryName p )\
            {return (EntryName)BaseList::Remove(p);}\
inline EntryName ClassName##MemberList::Replace( EntryName p )\
            {return (EntryName)BaseList::Replace(p);}\
inline EntryName ClassName##MemberList::Replace( EntryName p, ULONG nIdx )\
            {return (EntryName)BaseList::Replace(p,nIdx);}\
inline EntryName ClassName##MemberList::Replace( EntryName p, EntryName pOld )\
            {return (EntryName)BaseList::Replace(p,pOld);}\
inline EntryName   ClassName##MemberList::GetCurObject() const\
            {return (EntryName)BaseList::GetCurObject();}\
inline EntryName   ClassName##MemberList::GetObject( ULONG nIdx ) const\
            {return (EntryName)BaseList::GetObject( nIdx );}\
inline EntryName ClassName##MemberList::Seek( ULONG nIdx )\
            {return (EntryName)BaseList::Seek( nIdx );}\
inline EntryName ClassName##MemberList::Seek( EntryName p )\
            {return (EntryName)BaseList::Seek( p );}\
inline EntryName ClassName##MemberList::First()\
            {return (EntryName)BaseList::First();}\
inline EntryName ClassName##MemberList::Last()\
            {return (EntryName)BaseList::Last();}\
inline EntryName ClassName##MemberList::Next()\
            {return (EntryName)BaseList::Next();}\
inline EntryName ClassName##MemberList::Prev()\
            {return (EntryName)BaseList::Prev();}\
inline void      ClassName##MemberList::Append( const ClassName##MemberList & rList )\
            {BaseList::Append(rList);}\
inline ULONG     ClassName##MemberList::GetPos( const EntryName p) const\
            {return BaseList::GetPos( p );}\
inline ULONG     ClassName##MemberList::GetPos\
                    ( const EntryName p, ULONG nStart, BOOL bForward ) const\
            {return BaseList::GetPos( p, nStart, bForward );}

#define SV_DECL_MEMBER_LIST(ClassName,EntryName)\
class ClassName##MemberList : public SvRefBaseMemberList\
{\
public:\
    PRV_SV_DECL_MEMBER_LIST(ClassName,EntryName)\
};

#define SV_IMPL_MEMBER_LIST(ClassName,EntryName)\
    PRV_SV_IMPL_MEMBER_LIST(ClassName,EntryName,SvRefBaseMemberList)

#define SV_DECL_IMPL_MEMBER_LIST(ClassName,EntryName)\
SV_DECL_MEMBER_LIST(ClassName,EntryName)\
SV_IMPL_MEMBER_LIST(ClassName,EntryName)

/************************** S v R e f B a s e ****************************/
#define SV_NO_DELETE_REFCOUNT  0x80000000
class TOOLS_DLLPUBLIC SvRefBase
{
    UINT32 nRefCount;
#if defined (GCC) && (defined (C281) || defined (C290) || defined (C291))
public:
#else
protected:
#endif
    virtual         ~SvRefBase();
    virtual void    QueryDelete();
public:
                    SvRefBase() { nRefCount = SV_NO_DELETE_REFCOUNT; }
                    SvRefBase( const SvRefBase & /* rObj */ )
                    { nRefCount = SV_NO_DELETE_REFCOUNT; }
    SvRefBase &     operator = ( const SvRefBase & ) { return *this; }

    void            RestoreNoDelete()
                    {
                        if( nRefCount < SV_NO_DELETE_REFCOUNT )
                            nRefCount += SV_NO_DELETE_REFCOUNT;
                    }
    UINT32          AddMulRef( UINT32 n ) { return nRefCount += n; }
    UINT32          AddNextRef() { return ++nRefCount; }
    UINT32          AddRef()
                    {
                        if( nRefCount >= SV_NO_DELETE_REFCOUNT )
                            nRefCount -= SV_NO_DELETE_REFCOUNT;
                        return ++nRefCount;
                    }
    void            ReleaseReference()
                    {
                        if( !--nRefCount )
                            QueryDelete();
                    }
    UINT32          ReleaseRef()
                    {
                        UINT32 n = --nRefCount;
                        if( !n )
                            QueryDelete();
                        return n;
                    }
    UINT32          GetRefCount() const { return nRefCount; }
};

class SvCompatRefBase : public SvRefBase
{
    void (*pFunc)( void* );
    void *pObj;
protected:
    virtual         ~SvCompatRefBase();
public:
    SvCompatRefBase( void (*pFuncP)( void*), void *pObjP ) :
        pFunc( pFuncP ), pObj( pObjP ) {}
};

//#if 0 // _SOLAR__PRIVATE
#ifndef EMPTYARG
#define EMPTYARG
#endif
//#endif

SV_DECL_IMPL_REF(SvRefBase)

SV_DECL_REF_LIST(SvRefBase,SvRefBase*)

class SvWeakBase;
class SvWeakHdl : public SvRefBase
{
    friend class SvWeakBase;
    SvWeakBase* _pObj;
public:
    void ResetWeakBase( ) { _pObj = 0; }
private:
    SvWeakHdl( SvWeakBase* pObj ) : _pObj( pObj ) {}
public:
    SvWeakBase* GetObj() { return _pObj; }
};

SV_DECL_IMPL_REF( SvWeakHdl )

class SvCompatWeakHdl : public SvRefBase
{
    friend class SvCompatWeakBase;
    void* _pObj;
    SvCompatWeakHdl( void* pObj ) : _pObj( pObj ) {}
public:
    void ResetWeakBase( ) { _pObj = 0; }
    void* GetObj() { return _pObj; }
};

SV_DECL_IMPL_REF( SvCompatWeakHdl )

class SvWeakBase
{
    SvWeakHdlRef _xHdl;
public:
    SvWeakHdl* GetHdl() { return _xHdl; }

    // Wg CompilerWarnung nicht ueber Initializer
    SvWeakBase() { _xHdl = new SvWeakHdl( this ); }
    ~SvWeakBase() { _xHdl->ResetWeakBase(); }
};

class SvCompatWeakBase
{
    SvCompatWeakHdlRef _xHdl;
public:
    SvCompatWeakHdl* GetHdl() { return _xHdl; }

    // Wg CompilerWarnung nicht ueber Initializer
    SvCompatWeakBase( void* pObj ) { _xHdl = new SvCompatWeakHdl( pObj ); }
    ~SvCompatWeakBase() { _xHdl->ResetWeakBase(); }
};

#define SV_DECL_WEAK_IMPL( ClassName, HdlName )                     \
class ClassName##Weak                                               \
{                                                                   \
    HdlName _xHdl;                                                  \
public:                                                             \
    inline               ClassName##Weak( ) {}                      \
    inline               ClassName##Weak( ClassName* pObj ) {       \
        if( pObj ) _xHdl = pObj->GetHdl(); }                        \
    inline void          Clear() { _xHdl.Clear(); }                 \
    inline ClassName##Weak& operator = ( ClassName * pObj ) {       \
        _xHdl = pObj ? pObj->GetHdl() : 0; return *this; }          \
    inline BOOL            Is() const {                             \
        return _xHdl.Is() && _xHdl->GetObj(); }                     \
    inline ClassName *     operator &  () const {                   \
        return (ClassName*) ( _xHdl.Is() ? _xHdl->GetObj() : 0 ); } \
    inline ClassName *     operator -> () const {                   \
        return (ClassName*) ( _xHdl.Is() ? _xHdl->GetObj() : 0 ); } \
    inline ClassName &     operator *  () const {                   \
        return *(ClassName*) _xHdl->GetObj(); }                     \
    inline operator ClassName * () const {                          \
        return (ClassName*) (_xHdl.Is() ? _xHdl->GetObj() : 0 ); }  \
};

#define SV_DECL_WEAK( ClassName ) SV_DECL_WEAK_IMPL( ClassName, SvWeakHdlRef )
#define SV_DECL_COMPAT_WEAK( ClassName ) SV_DECL_WEAK_IMPL( ClassName, SvCompatWeakHdlRef )

SV_DECL_WEAK( SvWeakBase )

#endif // _Weak_HXX
