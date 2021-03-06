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

#include <com/sun/star/embed/XStorage.hpp>

#include <unordered_map>
#include <vector>

namespace framework
{

class XMLBasedAcceleratorConfiguration;
/**
    TODO document me
 */
class StorageHolder final
{

    // types
    public:

        /** @short  TODO */
        typedef ::std::vector< css::uno::Reference< css::embed::XStorage > > TStorageList;

        typedef ::std::vector< XMLBasedAcceleratorConfiguration* > TStorageListenerList;

        struct TStorageInfo
        {
            public:
                css::uno::Reference< css::embed::XStorage > Storage;
                sal_Int32 UseCount;
                TStorageListenerList Listener;

                TStorageInfo()
                    : UseCount(0)
                {}
        };

        /** @short  TODO */
        typedef std::unordered_map< OUString,
                                    TStorageInfo > TPath2StorageInfo;

    // member
    private:
        mutable osl::Mutex m_mutex;

        /** @short  TODO */
        css::uno::Reference< css::embed::XStorage > m_xRoot;

        /** @short  TODO */
        TPath2StorageInfo m_lStorages;

    // interface
    public:

        /** @short  TODO
         */
        StorageHolder();

        /** @short  TODO
         */
        ~StorageHolder();

        /** @short  TODO
         */
        void forgetCachedStorages();

        /** @short  TODO
         */
        void setRootStorage(const css::uno::Reference< css::embed::XStorage >& xRoot);

        /** @short  TODO
         */
        css::uno::Reference< css::embed::XStorage > getRootStorage() const;

        /** @short  TODO
                    open or get!
         */
        css::uno::Reference< css::embed::XStorage > openPath(const OUString& sPath    ,
                                                                           sal_Int32        nOpenMode);

        /** @short  TODO
         */
        StorageHolder::TStorageList getAllPathStorages(const OUString& sPath);

        /** @short  TODO
         */
        void commitPath(const OUString& sPath);

        /** @short  TODO
         */
        void closePath(const OUString& sPath);

        /** @short  TODO
         */
        void notifyPath(const OUString& sPath);

        /** @short  TODO
         */
        void addStorageListener(     XMLBasedAcceleratorConfiguration* pListener,
                                        const OUString&  sPath    );

        /** @short  TODO
         */
        void removeStorageListener(   XMLBasedAcceleratorConfiguration* pListener,
                                           const OUString&  sPath    );

        /** @short  TODO
         */
        OUString getPathOfStorage(const css::uno::Reference< css::embed::XStorage >& xStorage);

        /** @short  TODO
         */
        css::uno::Reference< css::embed::XStorage > getParentStorage(const css::uno::Reference< css::embed::XStorage >& xChild);

        /** @short  TODO
         */
        css::uno::Reference< css::embed::XStorage > getParentStorage(const OUString& sChildPath);

        /** @short  TODO
         */
        StorageHolder& operator=(const StorageHolder& rCopy);

        /** @short  opens a sub element of the specified base storage.
                    If eOpenMode contains an ELEMENT_WRITE flag remove it and try it with the rest of eOpenMode flags
                    again.

            @descr  First this method try to open the requested sub element
                    using the given open mode. If it failed there is second step,
                    which tries to do the same again ... but removing a might existing
                    WRITE flag from the open mode. The user can suppress this fallback
                    handling by setting the parameter bAllowFallback to sal_False.

            @param  xBaseStorage
                    the storage, where the sub element should be searched.

            @param  sSubElement
                    the full name of the sub element.
                    e.g. "default.xml"

            @param  eOpenMode
                    a flag field, which set the open mode for this operation.

         */
        static css::uno::Reference< css::embed::XStorage > openSubStorageWithFallback(const css::uno::Reference< css::embed::XStorage >& xBaseStorage  ,
                                                                                      const OUString&                             sSubStorage   ,
                                                                                      sal_Int32                                    eOpenMode);

        // helper
        private:

            /** @short  TODO
             */
            static OUString impl_st_normPath(const OUString& sPath);

            /** @short  TODO
             */
            static std::vector<OUString> impl_st_parsePath(const OUString& sPath);
};

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
