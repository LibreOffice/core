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
#ifndef INCLUDED_COMPHELPER_IDPROPARRAYHELPER_HXX
#define INCLUDED_COMPHELPER_IDPROPARRAYHELPER_HXX

#include <sal/config.h>

#include <map>

#include <cppuhelper/component.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <osl/diagnose.h>
#include <rtl/instance.hxx>
#include <cppuhelper/propshlp.hxx>

namespace cppu { class IPropertyArrayHelper; }

namespace comphelper
{

    //  OIdPropertyArrayUsageHelper

    template <typename TYPE> struct OIdPropertyArrayUsageHelperMutex
            : public rtl::Static< ::osl::Mutex, OIdPropertyArrayUsageHelperMutex<TYPE> > {};

    typedef std::map< sal_Int32, ::cppu::IPropertyArrayHelper*, std::less< sal_Int32 > > OIdPropertyArrayMap;
    template <class TYPE>
    class OIdPropertyArrayUsageHelper
    {
    protected:
        static sal_Int32                        s_nRefCount;
        static OIdPropertyArrayMap*             s_pMap;

    public:
        OIdPropertyArrayUsageHelper();
        virtual ~OIdPropertyArrayUsageHelper()
        {
            ::osl::MutexGuard aGuard(OIdPropertyArrayUsageHelperMutex<TYPE>::get());
            OSL_ENSURE(s_nRefCount > 0, "OIdPropertyArrayUsageHelper::~OIdPropertyArrayUsageHelper : suspicious call : have a refcount of 0 !");
            if (!--s_nRefCount)
            {
                // delete the element
                for (OIdPropertyArrayMap::iterator i = s_pMap->begin(); i != s_pMap->end(); ++i)
                    delete (*i).second;
                delete s_pMap;
                s_pMap = nullptr;
            }
        }

        /** call this in the getInfoHelper method of your derived class. The method returns the array helper of the
            class, which is created if necessary.
        */
        ::cppu::IPropertyArrayHelper* getArrayHelper(sal_Int32 nId);

    protected:
        /** used to implement the creation of the array helper which is shared amongst all instances of the class.
            This method needs to be implemented in derived classes.
            <BR>
            The method gets called with Mutex acquired.
            <BR>
            as long as IPropertyArrayHelper has no virtual destructor, the implementation of ~OPropertyArrayUsageHelper
            assumes that you created an ::cppu::OPropertyArrayHelper when deleting s_pProps.
            @return                         an pointer to the newly created array helper. Must not be NULL.
        */
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 nId) const = 0;
    };


    template<class TYPE>
    sal_Int32                       OIdPropertyArrayUsageHelper< TYPE >::s_nRefCount    = 0;

    template<class TYPE>
    OIdPropertyArrayMap*            OIdPropertyArrayUsageHelper< TYPE >::s_pMap = nullptr;


    template <class TYPE>
    OIdPropertyArrayUsageHelper<TYPE>::OIdPropertyArrayUsageHelper()
    {
        ::osl::MutexGuard aGuard(OIdPropertyArrayUsageHelperMutex<TYPE>::get());
        // create the map if necessary
        if (s_pMap == nullptr)
            s_pMap = new OIdPropertyArrayMap();
        ++s_nRefCount;
    }


    template <class TYPE>
    ::cppu::IPropertyArrayHelper* OIdPropertyArrayUsageHelper<TYPE>::getArrayHelper(sal_Int32 nId)
    {
        OSL_ENSURE(s_nRefCount, "OIdPropertyArrayUsageHelper::getArrayHelper : suspicious call : have a refcount of 0 !");
        ::osl::MutexGuard aGuard(OIdPropertyArrayUsageHelperMutex<TYPE>::get());
        // do we have the array already?
        if (! (*s_pMap)[nId] )
        {
            (*s_pMap)[nId] = createArrayHelper(nId);
            OSL_ENSURE((*s_pMap)[nId], "OIdPropertyArrayUsageHelper::getArrayHelper : createArrayHelper returned nonsense !");
        }
        return (*s_pMap)[nId];
    }
}
#endif // INCLUDED_COMPHELPER_IDPROPARRAYHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
