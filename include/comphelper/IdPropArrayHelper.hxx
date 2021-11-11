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

#include <sal/config.h>

#include <mutex>
#include <cppuhelper/propshlp.hxx>
#include <cassert>
#include <unordered_map>

namespace comphelper
{

    typedef std::unordered_map< sal_Int32, ::cppu::IPropertyArrayHelper* > OIdPropertyArrayMap;
    template <class TYPE>
    class OIdPropertyArrayUsageHelper
    {
    public:
        OIdPropertyArrayUsageHelper();
        virtual ~OIdPropertyArrayUsageHelper()
        {
            std::unique_lock aGuard(theMutex());
            assert(s_nRefCount > 0 && "OIdPropertyArrayUsageHelper::~OIdPropertyArrayUsageHelper : suspicious call : have a refcount of 0 !");
            if (!--s_nRefCount)
            {
                // delete the element
                for (auto const& elem : *s_pMap)
                    delete elem.second;
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
            @return                         a pointer to the newly created array helper. Must not be NULL.
        */
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 nId) const = 0;
    private:
        static sal_Int32                        s_nRefCount;
        static OIdPropertyArrayMap*             s_pMap;
        static std::mutex& theMutex()
        {
            static std::mutex SINGLETON;
            return SINGLETON;
        }
    };

    template<class TYPE>
    sal_Int32                       OIdPropertyArrayUsageHelper< TYPE >::s_nRefCount    = 0;

    template<class TYPE>
    OIdPropertyArrayMap*            OIdPropertyArrayUsageHelper< TYPE >::s_pMap = nullptr;

    template <class TYPE>
    OIdPropertyArrayUsageHelper<TYPE>::OIdPropertyArrayUsageHelper()
    {
        std::unique_lock aGuard(theMutex());
        // create the map if necessary
        if (!s_pMap)
            s_pMap = new OIdPropertyArrayMap;
        ++s_nRefCount;
    }

    template <class TYPE>
    ::cppu::IPropertyArrayHelper* OIdPropertyArrayUsageHelper<TYPE>::getArrayHelper(sal_Int32 nId)
    {
        assert(s_nRefCount && "OIdPropertyArrayUsageHelper::getArrayHelper : suspicious call : have a refcount of 0 !");
        std::unique_lock aGuard(theMutex());
        // do we have the array already?
        auto& rEntry = (*s_pMap)[nId];
        if (!rEntry)
        {
            rEntry = createArrayHelper(nId);
            assert(rEntry && "OIdPropertyArrayUsageHelper::getArrayHelper : createArrayHelper returned nonsense !");
        }
        return (*s_pMap)[nId];
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
