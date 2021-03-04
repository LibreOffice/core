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
#include <map>
#include <vector>

#include "Aolevariant.hxx"

namespace rtl
{
    class OUString;
}
namespace connectivity::ado
{
        class WpBase
        {
        protected:
            IDispatch* pIUnknown;

            void setIDispatch(IDispatch* _pIUnknown);
        public:
            WpBase();
            WpBase(IDispatch* pInt);
            //inline
            WpBase& operator=(const WpBase& rhs);
            WpBase& operator=(IDispatch* rhs);
            WpBase(const WpBase& aWrapper);
            virtual ~WpBase();
            void clear();


            bool IsValid() const;
            operator IDispatch*();

        };

        // Template class WpOLEBase<class T>
        // ==================================
        //
        // Objects of this class contain a pointer to an interface of the type T.
        // The ctors and operator= make sure, that AddRef() and Release() are being
        // called adhering to COM conventions.
        // An object can also hold no pointer (null pointer), calling IsValid() then
        // returns false.
        //
        // In order to do efficient pass-by-value, this class (as all derived classes)
        // is a thin wrapper class, avoiding virtual methods and inlining.

        template<class T> class WpOLEBase : public WpBase
        {
        protected:
            T* pInterface;

        public:
            WpOLEBase(T* pInt = nullptr) : WpBase(pInt),pInterface(pInt){}


            //inline
            WpOLEBase<T>& operator=(const WpOLEBase<T>& rhs)
            {
                WpBase::operator=(rhs);
                pInterface = rhs.pInterface;
                return *this;
            };

            WpOLEBase<T>& operator=(T* rhs)
            {
                WpBase::operator=(rhs);
                pInterface = rhs.pInterface;
                return *this;
            }

            WpOLEBase(const WpOLEBase<T>& aWrapper)
                : WpBase( aWrapper )
                , pInterface( aWrapper.pInterface )
            {
            }

            operator T*() const { return pInterface; }
            void setWithOutAddRef(T* _pInterface)
            {
                pInterface = _pInterface;
                WpBase::setIDispatch(_pInterface);
            }
        };


        // Template class WpOLECollection<class Ts, class T, class WrapT>
        // ===============================================================
        //
        // This class (derived from WpOLEBase<Ts>), abstracts away the properties
        // common to DAO collections:
        //
        // They are accessed via an interface Ts (e.g. DAOFields) and can return
        // Items of the Type T (actually: with the interface T, e.g. DAOField)
        // via get_Item (here GetItem).
        //
        // This wrapper class does not expose an interface T, however,
        // it exposes an object of the class WrapT. This must allow a construction
        // by T, preferably it is derived from WpOLEBase<T>.

        template<class Ts, class T, class WrapT> class WpOLECollection : public WpOLEBase<Ts>
        {
        public:
            using WpOLEBase<Ts>::pInterface;
            using WpOLEBase<Ts>::IsValid;
            // Ctors, operator=
            // They only call the superclass
            WpOLECollection(Ts* pInt=nullptr):WpOLEBase<Ts>(pInt){}
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
                T* pT = NULL;
                WrapT aRet(NULL);
                if(SUCCEEDED(pInterface->get_Item(OLEVariant(index), &pT)))
                    aRet.setWithOutAddRef(pT);
                return aRet;
            }

            WrapT GetItem(const OLEVariant& index) const
            {
                T* pT = NULL;
                WrapT aRet(NULL);
                if(SUCCEEDED(pInterface->get_Item(index, &pT)))
                    aRet.setWithOutAddRef(pT);
                return aRet;
            }

            WrapT GetItem(const OUString& sStr) const
            {
                WrapT aRet(NULL);
                T* pT = NULL;
                if (FAILED(pInterface->get_Item(OLEVariant(sStr), &pT)))
                {
#if OSL_DEBUG_LEVEL > 0
                    OString sTemp("Unknown Item: " + OString(sStr.getStr(),sStr.getLength(),osl_getThreadTextEncoding()));
                    OSL_FAIL(sTemp.getStr());
#endif
                }
                else
                    aRet.setWithOutAddRef(pT);
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

        template<class Ts, class T, class WrapT> class WpOLEAppendCollection:
                public WpOLECollection<Ts,T,WrapT>
        {

        public:
            // Ctors, operator=
            // They only call the superclass
            using WpOLEBase<Ts>::pInterface;
            WpOLEAppendCollection(Ts* pInt=nullptr):WpOLECollection<Ts,T,WrapT>(pInt){}
            WpOLEAppendCollection(const WpOLEAppendCollection& rhs) : WpOLECollection<Ts, T, WrapT>(rhs) {}
            WpOLEAppendCollection& operator=(const WpOLEAppendCollection& rhs)
                {WpOLEBase<Ts>::operator=(rhs); return *this;};


            bool Append(const WrapT& aWrapT)
            {
                return SUCCEEDED(pInterface->Append(OLEVariant(static_cast<T*>(aWrapT))));
            };

            bool Delete(const OUString& sName)
            {
                return SUCCEEDED(pInterface->Delete(OLEVariant(sName)));
            };


        };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
