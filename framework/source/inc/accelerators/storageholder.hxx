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

#ifndef __FRAMEWORK_ACCELERATORS_STORAGEHOLDER_HXX_
#define __FRAMEWORK_ACCELERATORS_STORAGEHOLDER_HXX_

#include <accelerators/istoragelistener.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <general.h>
#include <stdtypes.h>

#include <com/sun/star/embed/XStorage.hpp>


namespace framework
{


//-----------------------------------------------
/**
    TODO document me
 */
class StorageHolder : private ThreadHelpBase // attention! Must be the first base class to guarentee right initialize lock ...
{
    //-------------------------------------------
    // types
    public:

        /** @short  TODO */
        typedef ::std::vector< css::uno::Reference< css::embed::XStorage > > TStorageList;

        typedef ::std::vector< IStorageListener* > TStorageListenerList;

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
        typedef ::boost::unordered_map< OUString                    ,
                                 TStorageInfo                       ,
                                 OUStringHash                ,
                                 ::std::equal_to< OUString > > TPath2StorageInfo;

    //-------------------------------------------
    // member
    private:

        /** @short  TODO */
        css::uno::Reference< css::embed::XStorage > m_xRoot;

        /** @short  TODO */
        TPath2StorageInfo m_lStorages;

    //-------------------------------------------
    // interface
    public:

        //---------------------------------------
        /** @short  TODO
         */
        StorageHolder();

        //---------------------------------------
        /** @short  TODO
         */
        virtual ~StorageHolder();

        //---------------------------------------
        /** @short  TODO
         */
        virtual void forgetCachedStorages();

        //---------------------------------------
        /** @short  TODO
         */
        virtual void setRootStorage(const css::uno::Reference< css::embed::XStorage >& xRoot);

        //---------------------------------------
        /** @short  TODO
         */
        virtual css::uno::Reference< css::embed::XStorage > getRootStorage() const;

        //---------------------------------------
        /** @short  TODO
                    open or get!
         */
        virtual css::uno::Reference< css::embed::XStorage > openPath(const OUString& sPath    ,
                                                                           sal_Int32        nOpenMode);

        //---------------------------------------
        /** @short  TODO
         */
        virtual StorageHolder::TStorageList getAllPathStorages(const OUString& sPath);

        //---------------------------------------
        /** @short  TODO
         */
        virtual void commitPath(const OUString& sPath);

        //---------------------------------------
        /** @short  TODO
         */
        virtual void closePath(const OUString& sPath);

        //---------------------------------------
        /** @short  TODO
         */
        virtual void notifyPath(const OUString& sPath);

        //---------------------------------------
        /** @short  TODO
         */
        virtual void addStorageListener(      IStorageListener* pListener,
                                        const OUString&  sPath    );

        //---------------------------------------
        /** @short  TODO
         */
        virtual void removeStorageListener(      IStorageListener* pListener,
                                           const OUString&  sPath    );

        //---------------------------------------
        /** @short  TODO
         */
        virtual OUString getPathOfStorage(const css::uno::Reference< css::embed::XStorage >& xStorage);

        //---------------------------------------
        /** @short  TODO
         */
        virtual css::uno::Reference< css::embed::XStorage > getParentStorage(const css::uno::Reference< css::embed::XStorage >& xChild);

        //---------------------------------------
        /** @short  TODO
         */
        virtual css::uno::Reference< css::embed::XStorage > getParentStorage(const OUString& sChildPath);

        //---------------------------------------
        /** @short  TODO
         */
        void operator=(const StorageHolder& rCopy);

        //---------------------------------------
        /** @short  opens a sub element of the specified base storage.

            @descr  First this method try to open the requested sub element
                    using the given open mode. If it failed there is second step,
                    which tries to do the same again ... but removing a might existing
                    WRITE flag from the open mode. The user can supress this fallback
                    handling by setting the parameter bAllowFallback to sal_False.

            @param  xBaseStorage
                    the storage, where the sub element should be searched.

            @param  sSubElement
                    the full name of the sub element.
                    e.g. "default.xml"

            @param  eOpenMode
                    a flag field, which set the open mode for this operation.

            @param  bAllowFallback
                    if eOpenMode contains an ELEMENT_WRITE flag this parameter
                    allow to remove it and try it with the rest of eOpenMode flags
                    again.
         */
        static css::uno::Reference< css::embed::XStorage > openSubStorageWithFallback(const css::uno::Reference< css::embed::XStorage >& xBaseStorage  ,
                                                                                      const OUString&                             sSubStorage   ,
                                                                                            sal_Int32                                    eOpenMode     ,
                                                                                            sal_Bool                                     bAllowFallback);

        static css::uno::Reference< css::io::XStream > openSubStreamWithFallback(const css::uno::Reference< css::embed::XStorage >& xBaseStorage  ,
                                                                                 const OUString&                             sSubStream    ,
                                                                                       sal_Int32                                    eOpenMode     ,
                                                                                       sal_Bool                                     bAllowFallback);

        //---------------------------------------
        // helper
        private:

            //-----------------------------------
            /** @short  TODO
             */
            static OUString impl_st_normPath(const OUString& sPath);

            //-----------------------------------
            /** @short  TODO
             */
            static OUStringList impl_st_parsePath(const OUString& sPath);
};

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_STORAGEHOLDER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
