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

#ifndef INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_STORAGE_HXX
#define INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_STORAGE_HXX

#include <map>

#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "salhelper/simplereferenceobject.hxx"

#include "com/sun/star/embed/XStorage.hpp"

namespace tdoc_ucp {

    enum StorageAccessMode
    {
        READ, // Note: might be writable as well
        READ_WRITE_NOCREATE,
        READ_WRITE_CREATE
    };

    class Storage;
    class OfficeDocumentsManager;

    class StorageElementFactory : public salhelper::SimpleReferenceObject
    {
    public:
        StorageElementFactory(
            const css::uno::Reference< css::uno::XComponentContext > & rxContext,
            const rtl::Reference< OfficeDocumentsManager > & xDocsMgr );
        virtual ~StorageElementFactory();

        css::uno::Reference< css::embed::XStorage >
        createTemporaryStorage()
            throw ( css::uno::Exception,
                    css::uno::RuntimeException );

        css::uno::Reference< css::embed::XStorage >
        createStorage( const OUString & rUri, StorageAccessMode eMode )
            throw ( css::embed::InvalidStorageException,
                    css::lang::IllegalArgumentException,
                    css::io::IOException,
                    css::embed::StorageWrappedTargetException,
                    css::uno::RuntimeException );

        css::uno::Reference< css::io::XInputStream >
        createInputStream( const OUString & rUri,
                           const OUString & rPassword )
            throw ( css::embed::InvalidStorageException,
                    css::lang::IllegalArgumentException,
                    css::io::IOException,
                    css::embed::StorageWrappedTargetException,
                    css::packages::WrongPasswordException,
                    css::uno::RuntimeException );

        css::uno::Reference< css::io::XOutputStream >
        createOutputStream( const OUString & rUri,
                            const OUString & rPassword,
                            bool bTruncate )
            throw ( css::embed::InvalidStorageException,
                    css::lang::IllegalArgumentException,
                    css::io::IOException,
                    css::embed::StorageWrappedTargetException,
                    css::packages::WrongPasswordException,
                    css::uno::RuntimeException );

        css::uno::Reference< css::io::XStream >
        createStream( const OUString & rUri,
                      const OUString & rPassword,
                      bool bTruncate )
            throw ( css::embed::InvalidStorageException,
                    css::lang::IllegalArgumentException,
                    css::io::IOException,
                    css::embed::StorageWrappedTargetException,
                    css::packages::WrongPasswordException,
                    css::uno::RuntimeException );

    private:
        friend class Storage;

        void releaseElement( Storage * pElement );

        css::uno::Reference< css::embed::XStorage >
        queryParentStorage( const OUString & rUri,
                            StorageAccessMode eMode )
            throw ( css::embed::InvalidStorageException,
                    css::lang::IllegalArgumentException,
                    css::io::IOException,
                    css::embed::StorageWrappedTargetException,
                    css::uno::RuntimeException );

        css::uno::Reference< css::embed::XStorage >
        queryStorage( const css::uno::Reference<
                        css::embed::XStorage > & xParentStorage,
                      const OUString & rUri,
                      StorageAccessMode eMode )
            throw ( css::embed::InvalidStorageException,
                    css::lang::IllegalArgumentException,
                    css::io::IOException,
                    css::embed::StorageWrappedTargetException,
                    css::uno::RuntimeException );

        css::uno::Reference< css::io::XStream >
        queryStream( const css::uno::Reference<
                        css::embed::XStorage > & xParentStorage,
                     const OUString & rPassword,
                     const OUString & rUri,
                     StorageAccessMode eMode,
                     bool bTruncate /* ignored for read-only streams */ )
            throw ( css::embed::InvalidStorageException,
                    css::lang::IllegalArgumentException,
                    css::io::IOException,
                    css::embed::StorageWrappedTargetException,
                    css::packages::WrongPasswordException,
                    css::uno::RuntimeException );

        struct ltstrbool
        {
            bool operator()(
                const std::pair< OUString, bool > & s1,
                const std::pair< OUString, bool > & s2 ) const
            {
                if ( s1.first < s2.first )
                    return true;
                else if ( s1.first == s2.first )
                    return ( !s1.second && s2.second );
                else
                    return false;
            }
        };

        // key: pair< storageuri, iswritable >
        typedef std::map<
            std::pair< OUString, bool >, Storage *, ltstrbool > StorageMap;

        StorageMap m_aMap;
        osl::Mutex m_aMutex;
        rtl::Reference< OfficeDocumentsManager > m_xDocsMgr;
        css::uno::Reference< css::uno::XComponentContext > m_xContext;
    };

} // namespace tdoc_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_TDOC_TDOC_STORAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
