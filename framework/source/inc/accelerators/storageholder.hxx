/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_ACCELERATORS_STORAGEHOLDER_HXX_
#define __FRAMEWORK_ACCELERATORS_STORAGEHOLDER_HXX_

#include <accelerators/istoragelistener.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <general.h>
#include <stdtypes.h>

#include <com/sun/star/embed/XStorage.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

// namespace

namespace css = ::com::sun::star;

namespace framework
{

//===============================================
// definitions

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
        typedef ::boost::unordered_map< ::rtl::OUString                    ,
                                 TStorageInfo                       ,
                                 ::rtl::OUStringHash                ,
                                 ::std::equal_to< ::rtl::OUString > > TPath2StorageInfo;

    //-------------------------------------------
    // member
    private:

        /** @short  TODO */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

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
        StorageHolder(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

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
        virtual css::uno::Reference< css::embed::XStorage > openPath(const ::rtl::OUString& sPath    ,
                                                                           sal_Int32        nOpenMode);

        //---------------------------------------
        /** @short  TODO
         */
        virtual StorageHolder::TStorageList getAllPathStorages(const ::rtl::OUString& sPath);

        //---------------------------------------
        /** @short  TODO
         */
        virtual void commitPath(const ::rtl::OUString& sPath);

        //---------------------------------------
        /** @short  TODO
         */
        virtual void closePath(const ::rtl::OUString& sPath);

        //---------------------------------------
        /** @short  TODO
         */
        virtual void notifyPath(const ::rtl::OUString& sPath);

        //---------------------------------------
        /** @short  TODO
         */
        virtual void addStorageListener(      IStorageListener* pListener,
                                        const ::rtl::OUString&  sPath    );

        //---------------------------------------
        /** @short  TODO
         */
        virtual void removeStorageListener(      IStorageListener* pListener,
                                           const ::rtl::OUString&  sPath    );

        //---------------------------------------
        /** @short  TODO
         */
        virtual ::rtl::OUString getPathOfStorage(const css::uno::Reference< css::embed::XStorage >& xStorage);

        //---------------------------------------
        /** @short  TODO
         */
        virtual css::uno::Reference< css::embed::XStorage > getParentStorage(const css::uno::Reference< css::embed::XStorage >& xChild);

        //---------------------------------------
        /** @short  TODO
         */
        virtual css::uno::Reference< css::embed::XStorage > getParentStorage(const ::rtl::OUString& sChildPath);

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
                                                                                      const ::rtl::OUString&                             sSubStorage   ,
                                                                                            sal_Int32                                    eOpenMode     ,
                                                                                            sal_Bool                                     bAllowFallback);

        static css::uno::Reference< css::io::XStream > openSubStreamWithFallback(const css::uno::Reference< css::embed::XStorage >& xBaseStorage  ,
                                                                                 const ::rtl::OUString&                             sSubStream    ,
                                                                                       sal_Int32                                    eOpenMode     ,
                                                                                       sal_Bool                                     bAllowFallback);

        //---------------------------------------
        // helper
        private:

            //-----------------------------------
            /** @short  TODO
             */
            static ::rtl::OUString impl_st_normPath(const ::rtl::OUString& sPath);

            //-----------------------------------
            /** @short  TODO
             */
            static OUStringList impl_st_parsePath(const ::rtl::OUString& sPath);
};

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_STORAGEHOLDER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
