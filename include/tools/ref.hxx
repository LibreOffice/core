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
#include <utility>

/**
   This implements similar functionality to boost::intrusive_ptr
*/

namespace tools {

/** T must be a class that extends SvRefBase */
template<typename T> class SAL_DLLPUBLIC_RTTI SvRef final {
public:
    SvRef(): pObj(nullptr) {}

    SvRef(SvRef&& rObj)
    {
        pObj = rObj.pObj;
        rObj.pObj = nullptr;
    }

    SvRef(SvRef const & rObj): pObj(rObj.pObj)
    {
        if (pObj != nullptr) pObj->AddNextRef();
    }

    SvRef(T * pObjP): pObj(pObjP)
    {
        if (pObj != nullptr) pObj->AddFirstRef();
    }

    ~SvRef()
    {
        if (pObj != nullptr) pObj->ReleaseRef();
    }

    void clear()
    {
        if (pObj != nullptr) {
            T * pRefObj = pObj;
            pObj = nullptr;
            pRefObj->ReleaseRef();
        }
    }

    SvRef & operator =(SvRef const & rObj)
    {
        if (rObj.pObj != nullptr) {
            rObj.pObj->AddNextRef();
        }
        T * pRefObj = pObj;
        pObj = rObj.pObj;
        if (pRefObj != nullptr) {
            pRefObj->ReleaseRef();
        }
        return *this;
    }

    SvRef & operator =(SvRef && rObj)
    {
        if (pObj != nullptr) {
            pObj->ReleaseRef();
        }
        pObj = rObj.pObj;
        rObj.pObj = nullptr;
        return *this;
    }

    bool is()         const { return pObj != nullptr; }

    explicit operator bool() const { return is(); }

    T * get()         const { return pObj; }

    T * operator ->() const { assert(pObj != nullptr); return pObj; }

    T & operator *()  const { assert(pObj != nullptr); return *pObj; }

    bool operator ==(const SvRef<T> &rhs) const { return pObj == rhs.pObj; }
    bool operator !=(const SvRef<T> &rhs) const { return !(*this == rhs); }

private:
    T * pObj;
};

/**
 * This implements similar functionality to std::make_shared.
 */
template<typename T, typename... Args>
SvRef<T> make_ref(Args&& ... args)
{
    return SvRef<T>(new T(std::forward<Args>(args)...));
}

}

/** Classes that want to be referenced-counted via SvRef<T>, should extend this base class */
class TOOLS_DLLPUBLIC SvRefBase
{
    // work around a clang 3.5 optimization bug: if the bNoDelete is *first*
    // it mis-compiles "if (--nRefCount == 0)" and never deletes any object
    unsigned int nRefCount : 31;
    // the only reason this is not bool is because MSVC cannot handle mixed type bitfields
    unsigned int bNoDelete : 1;

protected:
    virtual         ~SvRefBase();

public:
                    SvRefBase() : nRefCount(0), bNoDelete(1) {}

                    SvRefBase(const SvRefBase &) : nRefCount(0), bNoDelete(1) {}

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
    void  ResetWeakBase( ) { _pObj = nullptr; }
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

    SvCompatWeakHdl<T>* GetHdl() { return _xHdl.get(); }
};

/** We only have one weak reference in LO, in include/sfx2/frame.hxx, class SfxFrameWeak.
*/
template<typename T>
class SAL_WARN_UNUSED SvCompatWeakRef
{
    tools::SvRef< SvCompatWeakHdl<T> > _xHdl;
public:
    inline               SvCompatWeakRef( ) {}
    inline               SvCompatWeakRef( T* pObj )
                         {  if( pObj ) _xHdl = pObj->GetHdl(); }
    inline SvCompatWeakRef& operator = ( T * pObj )
                         {  _xHdl = pObj ? pObj->GetHdl() : nullptr; return *this; }
    inline bool          is() const
                         { return _xHdl.is() && _xHdl->GetObj(); }
    explicit operator bool() const { return is(); }
    inline T*            operator -> () const
                         { return _xHdl.is() ? _xHdl->GetObj() : nullptr; }
    inline operator T* () const
                         { return _xHdl.is() ? _xHdl->GetObj() : nullptr; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
