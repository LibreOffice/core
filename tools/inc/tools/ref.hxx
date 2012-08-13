/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _REF_HXX
#define _REF_HXX

#include "tools/toolsdllapi.h"
#include <vector>

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
    inline               ClassName##Ref() { pObj = 0; }                 \
    inline               ClassName##Ref( const ClassName##Ref & rObj ); \
    inline               ClassName##Ref( ClassName * pObjP );           \
    inline void          Clear();                                       \
    inline               ~ClassName##Ref();                             \
    inline ClassName##Ref & operator = ( const ClassName##Ref & rObj ); \
    inline ClassName##Ref & operator = ( ClassName * pObj );            \
    inline sal_Bool        Is() const { return pObj != NULL; }          \
    inline ClassName *     operator &  () const { return pObj; }        \
    inline ClassName *     operator -> () const { return pObj; }        \
    inline ClassName &     operator *  () const { return *pObj; }       \
    inline operator ClassName * () const { return pObj; }

#define PRV_SV_DECL_REF( ClassName )            \
PRV_SV_DECL_REF_LOCK( ClassName, Ref )

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
    PRV_SV_DECL_REF_LOCK( ClassName, Lock )     \
};

#define SV_IMPL_REF( ClassName )                                \
PRV_SV_IMPL_REF_COUNTERS( ClassName, Ref, AddRef(), AddNextRef(),\
                          ReleaseReference(), EMPTYARG, pObj )

#define SV_IMPL_LOCK( ClassName )                                   \
PRV_SV_IMPL_REF_COUNTERS( ClassName, Lock, OwnerLock( sal_True ),       \
                          OwnerLock( sal_True ), OwnerLock( sal_False ),    \
                          EMPTYARG, pObj )

#define SV_DECL_IMPL_REF(ClassName)             \
    SV_DECL_REF(ClassName)                      \
    SV_IMPL_REF(ClassName)

template<typename T>
class SvRefMemberList : private std::vector<T>
{
private:
    typedef typename std::vector<T> base_t;

public:
    using base_t::size;
    using base_t::front;
    using base_t::back;
    using base_t::operator[];
    using base_t::begin;
    using base_t::end;
    using typename base_t::iterator;
    using typename base_t::const_iterator;
    using base_t::rbegin;
    using base_t::rend;
    using typename base_t::reverse_iterator;
    using base_t::empty;

    inline ~SvRefMemberList() { clear(); }
    inline void clear()
    {
        for( typename base_t::const_iterator it = base_t::begin(); it != base_t::end(); ++it )
        {
              T p = *it;
              if( p )
                  p->ReleaseReference();
        }
        base_t::clear();
    }

    inline void push_back( T p )
    {
        base_t::push_back( p );
        p->AddRef();
    }

    inline void insert(const SvRefMemberList& rOther)
    {
       for( typename base_t::const_iterator it = rOther.begin(); it != rOther.end(); ++it )
       {
           push_back(*it);
       }
    }

    inline T pop_back()
    {
        T p = base_t::back();
        base_t::pop_back();
        if( p )
            p->ReleaseReference();
        return p;
    }
};

#define SV_NO_DELETE_REFCOUNT  0x80000000

class TOOLS_DLLPUBLIC SvRefBase
{
    sal_uIntPtr nRefCount;

protected:
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
    sal_uIntPtr     AddMulRef( sal_uIntPtr n ) { return nRefCount += n; }
    sal_uIntPtr     AddNextRef() { return ++nRefCount; }
    sal_uIntPtr     AddRef()
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
    sal_uIntPtr     ReleaseRef()
                    {
                        sal_uIntPtr n = --nRefCount;
                        if( !n )
                            QueryDelete();
                        return n;
                    }
    sal_uIntPtr     GetRefCount() const { return nRefCount; }
};

#ifndef EMPTYARG
#define EMPTYARG
#endif

SV_DECL_IMPL_REF(SvRefBase)

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

class SvCompatWeakBase
{
    SvCompatWeakHdlRef _xHdl;

public:
    SvCompatWeakHdl* GetHdl() { return _xHdl; }

    // does not use Initalizer due to compiler warnings
    SvCompatWeakBase( void* pObj ) { _xHdl = new SvCompatWeakHdl( pObj ); }
    ~SvCompatWeakBase() { _xHdl->ResetWeakBase(); }
};

#define SV_DECL_COMPAT_WEAK( ClassName )                            \
class ClassName##Weak                                               \
{                                                                   \
    SvCompatWeakHdlRef _xHdl;                                       \
public:                                                             \
    inline               ClassName##Weak( ) {}                      \
    inline               ClassName##Weak( ClassName* pObj ) {       \
        if( pObj ) _xHdl = pObj->GetHdl(); }                        \
    inline void          Clear() { _xHdl.Clear(); }                 \
    inline ClassName##Weak& operator = ( ClassName * pObj ) {       \
        _xHdl = pObj ? pObj->GetHdl() : 0; return *this; }          \
    inline sal_Bool            Is() const {                         \
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
