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
#ifndef INCLUDED_TOOLS_REF_HXX
#define INCLUDED_TOOLS_REF_HXX

#include <sal/config.h>
#include <cassert>
#include <tools/toolsdllapi.h>

/**
   This implements similar functionality to boost::intrusive_ptr
*/

namespace tools {

/** T must be a class that extends SvRefBase */
template<typename T> class SAL_DLLPUBLIC_RTTI SvRef {
public:
    SvRef(): pObj(0) {}

    SvRef(SvRef const & rObj): pObj(rObj.pObj)
    {
        if (pObj != 0) pObj->AddNextRef();
    }

    SvRef(T * pObjP): pObj(pObjP)
    {
        if (pObj != 0) pObj->AddFirstRef();
    }

    ~SvRef()
    {
        if (pObj != 0) pObj->ReleaseRef();
    }

    void Clear()
    {
        if (pObj != 0) {
            T * pRefObj = pObj;
            pObj = 0;
            pRefObj->ReleaseRef();
        }
    }

    SvRef & operator =(SvRef const & rObj)
    {
        if (rObj.pObj != 0) {
            rObj.pObj->AddNextRef();
        }
        T * pRefObj = pObj;
        pObj = rObj.pObj;
        if (pRefObj != 0) {
            pRefObj->ReleaseRef();
        }
        return *this;
    }

    bool Is()         const { return pObj != 0; }

    T * get()         const { return pObj; }

    T * operator &()  const { return pObj; }

    T * operator ->() const { assert(pObj != 0); return pObj; }

    T & operator *()  const { assert(pObj != 0); return *pObj; }

    operator T *()    const { return pObj; }

protected:
    T * pObj;
};

}

/** Classes that want to be referenced-counted via SvRef<T>, should extend this base class */
class TOOLS_DLLPUBLIC SvRefBase
{
    // the only reason this is not bool is because MSVC cannot handle mixed type bitfields
#if defined(__AFL_HAVE_MANUAL_INIT)
    bool bNoDelete;
#else
    unsigned int bNoDelete : 1;
#endif
    unsigned int nRefCount : 31;

protected:
    virtual         ~SvRefBase();

public:
                    SvRefBase() : bNoDelete(1), nRefCount(0) {}

                    SvRefBase( const SvRefBase & /* rObj */ )  : bNoDelete(1), nRefCount(0) {}

    SvRefBase &     operator = ( const SvRefBase & )
                    { return *this; }

    void            RestoreNoDelete()
                    { bNoDelete = 1; }

    void            AddNextRef()
                    {
                        assert( nRefCount < (1 << 30) && "Do not add refs to dead objects" );
                        ++nRefCount;
                    }

    void            AddFirstRef()
                    {
                        assert( nRefCount < (1 << 30) && "Do not add refs to dead objects" );
                        if( bNoDelete )
                            bNoDelete = 0;
                        ++nRefCount;
                    }

    void            ReleaseRef()
                    {
                        assert( nRefCount >= 1);
                        if( --nRefCount == 0 && !bNoDelete)
                        {
                            // I'm not sure about the original purpose of this line, but right now
                            // it serves the purpose that anything that attempts to do an AddRef()
                            // after an object is deleted will trip an assert.
                            nRefCount = 1 << 30;
                            delete this;
                        }
                    }

    unsigned int    GetRefCount() const
                    { return nRefCount; }
};

template<typename T>
class SvCompatWeakBase;

/** SvCompatWeakHdl acts as a intermediary between SvCompatWeakRef<T> and T.
*/
template<typename T>
class SvCompatWeakHdl : public SvRefBase
{
    friend class SvCompatWeakBase<T>;
    T* _pObj;

    SvCompatWeakHdl( T* pObj ) : _pObj( pObj ) {}

public:
    void  ResetWeakBase( ) { _pObj = 0; }
    T*    GetObj()        { return _pObj; }
};

/** We only have one place that extends this, in include/sfx2/frame.hxx, class SfxFrame.
    Its function is to notify the SvCompatWeakHdl when an SfxFrame object is deleted.
*/
template<typename T>
class SvCompatWeakBase
{
    tools::SvRef< SvCompatWeakHdl<T> > _xHdl;

public:
    /** Does not use initializer due to compiler warnings,
        because the lifetime of the _xHdl object can exceed the lifetime of this class.
     */
    SvCompatWeakBase( T* pObj ) { _xHdl = new SvCompatWeakHdl<T>( pObj ); }

    ~SvCompatWeakBase() { _xHdl->ResetWeakBase(); }

    SvCompatWeakHdl<T>* GetHdl() { return _xHdl; }
};

/** We only have one weak reference in LO, in include/sfx2/frame.hxx, class SfxFrameWeak.
*/
template<typename T>
class SvCompatWeakRef
{
    tools::SvRef< SvCompatWeakHdl<T> > _xHdl;
public:
    inline               SvCompatWeakRef( ) {}
    inline               SvCompatWeakRef( T* pObj )
                         {  if( pObj ) _xHdl = pObj->GetHdl(); }
    inline SvCompatWeakRef& operator = ( T * pObj )
                         {  _xHdl = pObj ? pObj->GetHdl() : 0; return *this; }
    inline bool          Is() const
                         { return _xHdl.Is() && _xHdl->GetObj(); }
    inline T*            operator -> () const
                         { return _xHdl.Is() ? _xHdl->GetObj() : 0; }
    inline T*            operator &  () const
                         { return _xHdl.Is() ? _xHdl->GetObj() : 0; }
    inline operator T* () const
                         { return _xHdl.Is() ? _xHdl->GetObj() : 0; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
