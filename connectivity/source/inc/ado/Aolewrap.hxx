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
#ifndef _CONNECTIVITY_ADO_AOLEWRAP_HXX_
#define _CONNECTIVITY_ADO_AOLEWRAP_HXX_

#include <osl/diagnose.h>
#include <osl/thread.h>
#include <map>
#include <vector>
#include "connectivity/StdTypeDefs.hxx"

namespace rtl
{
    class OUString;
}
namespace connectivity
{
    namespace ado
    {
        class OLEVariant;
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


            sal_Bool IsValid() const;
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
            WpOLEBase(T* pInt = NULL) : WpBase(pInt),pInterface(pInt){}


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

            virtual ~WpOLEBase()
            {
            }

            operator T*() const { return static_cast<T*>(pInterface); }
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
            WpOLECollection(Ts* pInt=NULL):WpOLEBase<Ts>(pInt){}
            WpOLECollection(const WpOLECollection& rhs) : WpOLEBase<Ts>(rhs) {}
            inline WpOLECollection& operator=(const WpOLECollection& rhs)
                {WpOLEBase<Ts>::operator=(rhs); return *this;};



            inline void Refresh(){pInterface->Refresh();}

            inline sal_Int32 GetItemCount() const
            {
                sal_Int32 nCount = 0;
                return pInterface ? (SUCCEEDED(pInterface->get_Count(&nCount)) ? nCount : sal_Int32(0)) : sal_Int32(0);
            }

            inline WrapT GetItem(sal_Int32 index) const
            {
                OSL_ENSURE(index >= 0 && index<GetItemCount(),"Wrong index for field!");
                T* pT = NULL;
                WrapT aRet(NULL);
                if(SUCCEEDED(pInterface->get_Item(OLEVariant(index), &pT)))
                    aRet.setWithOutAddRef(pT);
                return aRet;
            }

            inline WrapT GetItem(const OLEVariant& index) const
            {
                T* pT = NULL;
                WrapT aRet(NULL);
                if(SUCCEEDED(pInterface->get_Item(index, &pT)))
                    aRet.setWithOutAddRef(pT);
                return aRet;
            }

            inline WrapT GetItem(const OUString& sStr) const
            {
                WrapT aRet(NULL);
                T* pT = NULL;
                if (FAILED(pInterface->get_Item(OLEVariant(sStr), &pT)))
                {
#if OSL_DEBUG_LEVEL > 0
                    OString sTemp("Unknown Item: ");
                    sTemp += OString(sStr.getStr(),sStr.getLength(),osl_getThreadTextEncoding());
                    OSL_FAIL(sTemp.getStr());
#endif
                }
                else
                    aRet.setWithOutAddRef(pT);
                return aRet;
            }
            inline void fillElementNames(TStringVector& _rVector)
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
            WpOLEAppendCollection(Ts* pInt=NULL):WpOLECollection<Ts,T,WrapT>(pInt){}
            WpOLEAppendCollection(const WpOLEAppendCollection& rhs) : WpOLECollection<Ts, T, WrapT>(rhs) {}
            inline WpOLEAppendCollection& operator=(const WpOLEAppendCollection& rhs)
                {WpOLEBase<Ts>::operator=(rhs); return *this;};


            inline sal_Bool Append(const WrapT& aWrapT)
            {
                return SUCCEEDED(pInterface->Append(OLEVariant((T*)aWrapT)));
            };

            inline sal_Bool Delete(const OUString& sName)
            {
                return SUCCEEDED(pInterface->Delete(OLEVariant(sName)));
            };


        };
    }
}
#endif // _CONNECTIVITY_ADO_AOLEWRAP_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
