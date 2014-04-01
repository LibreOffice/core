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

#ifndef INCLUDED_TDOC_STORAGE_HXX
#define INCLUDED_TDOC_STORAGE_HXX

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
            const com::sun::star::uno::Reference<
                com::sun::star::uno::XComponentContext > & rxContext,
            const rtl::Reference< OfficeDocumentsManager > & xDocsMgr );
        virtual ~StorageElementFactory();

        com::sun::star::uno::Reference< com::sun::star::embed::XStorage >
        createTemporaryStorage()
            throw ( com::sun::star::uno::Exception,
                    com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::embed::XStorage >
        createStorage( const OUString & rUri, StorageAccessMode eMode )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
        createInputStream( const OUString & rUri,
                           const OUString & rPassword )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::packages::WrongPasswordException,
                    com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >
        createOutputStream( const OUString & rUri,
                            const OUString & rPassword,
                            bool bTruncate )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::packages::WrongPasswordException,
                    com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::io::XStream >
        createStream( const OUString & rUri,
                      const OUString & rPassword,
                      bool bTruncate )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::packages::WrongPasswordException,
                    com::sun::star::uno::RuntimeException );

    private:
        friend class Storage;

        void releaseElement( Storage * pElement ) SAL_THROW(());

        com::sun::star::uno::Reference< com::sun::star::embed::XStorage >
        queryParentStorage( const OUString & rUri,
                            StorageAccessMode eMode )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::embed::XStorage >
        queryStorage( const com::sun::star::uno::Reference<
                        com::sun::star::embed::XStorage > & xParentStorage,
                      const OUString & rUri,
                      StorageAccessMode eMode )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::io::XStream >
        queryStream( const com::sun::star::uno::Reference<
                        com::sun::star::embed::XStorage > & xParentStorage,
                     const OUString & rPassword,
                     const OUString & rUri,
                     StorageAccessMode eMode,
                     bool bTruncate /* ignored for read-only streams */ )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::packages::WrongPasswordException,
                    com::sun::star::uno::RuntimeException );

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
        com::sun::star::uno::Reference<
            com::sun::star::uno::XComponentContext > m_xContext;
    };

} // namespace tdoc_ucp

#endif /* !INCLUDED_TDOC_STORAGE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
