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

#ifndef CONNECTIVITY_FIREBIRD_SUBCOMPONENT_HXX
#define CONNECTIVITY_FIREBIRD_SUBCOMPONENT_HXX

#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/weak.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <com/sun/star/lang/DisposedException.hpp>

namespace cppu {
    class IPropertyArrayHelper;
}

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace lang
            {
                class XComponent;
            }
        }
    }
}
namespace connectivity
{

    namespace firebird
    {
        void release(oslInterlockedCount& _refCount,
                     ::cppu::OBroadcastHelper& rBHelper,
                     ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface,
                     ::com::sun::star::lang::XComponent* _pObject);

        void checkDisposed(sal_Bool _bThrow) throw ( ::com::sun::star::lang::DisposedException );


        template <class TYPE>
        class OPropertyArrayUsageHelper
        {
        protected:
            static sal_Int32                        s_nRefCount;
            static ::cppu::IPropertyArrayHelper*    s_pProps;
            static ::osl::Mutex                     s_aMutex;

        public:
            OPropertyArrayUsageHelper();
            virtual ~OPropertyArrayUsageHelper()
            {   // ARGHHHHHHH ..... would like to implement this in proparrhlp_impl.hxx (as we do with all other methods)
                // but SUNPRO 5 compiler (linker) doesn't like this
                ::osl::MutexGuard aGuard(s_aMutex);
                OSL_ENSURE(s_nRefCount > 0, "OPropertyArrayUsageHelper::~OPropertyArrayUsageHelper : suspicious call : have a refcount of 0 !");
                if (!--s_nRefCount)
                {
                    delete s_pProps;
                    s_pProps = NULL;
                }
            }

            /** call this in the getInfoHelper method of your derived class. The method returns the array helper of the
                class, which is created if neccessary.
            */
            ::cppu::IPropertyArrayHelper*   getArrayHelper();

        protected:
            /** used to implement the creation of the array helper which is shared amongst all instances of the class.
                This method needs to be implemented in derived classes.
                <BR>
                The method gets called with s_aMutex acquired.
                <BR>
                as long as IPropertyArrayHelper has no virtual destructor, the implementation of ~OPropertyArrayUsageHelper
                assumes that you created an ::cppu::OPropertyArrayHelper when deleting s_pProps.
                @return                         an pointer to the newly created array helper. Must not be NULL.
            */
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const = 0;
        };

        template<class TYPE>
        sal_Int32                       OPropertyArrayUsageHelper< TYPE >::s_nRefCount  = 0;

        template<class TYPE>
        ::cppu::IPropertyArrayHelper*   OPropertyArrayUsageHelper< TYPE >::s_pProps = NULL;

        template<class TYPE>
        ::osl::Mutex                    OPropertyArrayUsageHelper< TYPE >::s_aMutex;

        //------------------------------------------------------------------
        template <class TYPE>
        OPropertyArrayUsageHelper<TYPE>::OPropertyArrayUsageHelper()
        {
            ::osl::MutexGuard aGuard(s_aMutex);
            ++s_nRefCount;
        }

        //------------------------------------------------------------------
        template <class TYPE>
        ::cppu::IPropertyArrayHelper* OPropertyArrayUsageHelper<TYPE>::getArrayHelper()
        {
            OSL_ENSURE(s_nRefCount, "OPropertyArrayUsageHelper::getArrayHelper : suspicious call : have a refcount of 0 !");
            if (!s_pProps)
            {
                ::osl::MutexGuard aGuard(s_aMutex);
                if (!s_pProps)
                {
                    s_pProps = createArrayHelper();
                    OSL_ENSURE(s_pProps, "OPropertyArrayUsageHelper::getArrayHelper : createArrayHelper returned nonsense !");
                }
            }
            return s_pProps;
        }

        class OBase_Mutex
        {
        public:
            ::osl::Mutex m_aMutex;
        };




    }
}
#endif // CONNECTIVITY_FIREBIRD_SUBCOMPONENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
