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
#pragma once

#include <osl/diagnose.h>
#include <osl/thread.h>
#include <systools/win32/comtools.hxx>

#include <map>
#include <vector>

#include "Aolevariant.hxx"

namespace rtl
{
    class OUString;
}
namespace connectivity::ado
{
    // Template class WpOLEBase<class T>
    // ==================================
    //
    // Objects of this class contain a pointer to an interface of the type T.

    template<class T> class WpOLEBase
    {
    protected:
        sal::systools::COMReference<T> pInterface;

    public:
        WpOLEBase(T* pInt = nullptr) : pInterface(pInt){}

        WpOLEBase(const WpOLEBase<T>& aWrapper)
            : pInterface( aWrapper.pInterface )
        {
        }

        //inline
        WpOLEBase<T>& operator=(const WpOLEBase<T>& rhs)
        {
            pInterface = rhs.pInterface;
            return *this;
        };

        operator T*() const { return pInterface.get(); }
        T** operator&() { return &pInterface; }
        bool IsValid() const { return pInterface.is(); }
        void set(T* p) { pInterface = p; }
        void clear() { pInterface.clear(); }
    };


    // Template class WpOLECollection<class Ts, class WrapT>
    // ===============================================================
    //
    // This class (derived from WpOLEBase<Ts>), abstracts away the properties
    // common to DAO collections:
    //
    // They are accessed via an interface Ts (e.g. DAOFields) and can return
    // Items of the type wrapped by WrapT (actually: with the interface, e.g.
    // DAOField) via get_Item (here GetItem).
    //
    // This wrapper class exposes an object of the class WrapT.

    template<class Ts, class WrapT> class WpOLECollection : public WpOLEBase<Ts>
    {
    public:
        using WpOLEBase<Ts>::pInterface;
        using WpOLEBase<Ts>::IsValid;
        // Ctors, operator=
        // They only call the superclass
        WpOLECollection() = default;
        WpOLECollection(const WpOLECollection& rhs) : WpOLEBase<Ts>(rhs) {}
        WpOLECollection& operator=(const WpOLECollection& rhs)
            {WpOLEBase<Ts>::operator=(rhs); return *this;};


        void Refresh(){pInterface->Refresh();}

        sal_Int32 GetItemCount() const
        {
            sal_Int32 nCount = 0;
            return pInterface ? (SUCCEEDED(pInterface->get_Count(&nCount)) ? nCount : sal_Int32(0)) : sal_Int32(0);
        }

        WrapT GetItem(sal_Int32 index) const
        {
            OSL_ENSURE(index >= 0 && index<GetItemCount(),"Wrong index for field!");
            WrapT aRet;
            pInterface->get_Item(OLEVariant(index), &aRet);
            return aRet;
        }

        WrapT GetItem(const OLEVariant& index) const
        {
            WrapT aRet;
            pInterface->get_Item(index, &aRet);
            return aRet;
        }

        WrapT GetItem(const OUString& sStr) const
        {
            WrapT aRet;
            if (FAILED(pInterface->get_Item(OLEVariant(sStr), &aRet)))
            {
#if OSL_DEBUG_LEVEL > 0
                OString sTemp("Unknown Item: " + OString(sStr.getStr(),sStr.getLength(),osl_getThreadTextEncoding()));
                OSL_FAIL(sTemp.getStr());
#endif
            }
            return aRet;
        }
        void fillElementNames(::std::vector< OUString>& _rVector)
        {
            if(IsValid())
            {
                Refresh();
                sal_Int32 nCount = GetItemCount();
                _rVector.reserve(nCount);
                for(sal_Int32 i=0;i< nCount;++i)
                {
                    WrapT aElement = GetItem(i);
                    if(aElement.IsValid())
                        _rVector.push_back(aElement.get_Name());
                }
            }
        }
    };

    template<class Ts, class WrapT> class WpOLEAppendCollection:
            public WpOLECollection<Ts,WrapT>
    {

    public:
        // Ctors, operator=
        // They only call the superclass
        using WpOLEBase<Ts>::pInterface;
        WpOLEAppendCollection() = default;
        WpOLEAppendCollection(const WpOLEAppendCollection& rhs) : WpOLECollection<Ts, WrapT>(rhs) {}
        WpOLEAppendCollection& operator=(const WpOLEAppendCollection& rhs)
            {WpOLEBase<Ts>::operator=(rhs); return *this;};


        bool Append(const WrapT& aWrapT)
        {
            return SUCCEEDED(pInterface->Append(OLEVariant(aWrapT)));
        };

        bool Delete(const OUString& sName)
        {
            return SUCCEEDED(pInterface->Delete(OLEVariant(sName)));
        };


    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
