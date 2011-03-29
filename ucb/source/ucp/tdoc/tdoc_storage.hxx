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
                com::sun::star::lang::XMultiServiceFactory > & xSMgr,
            const rtl::Reference< OfficeDocumentsManager > & xDocsMgr );
        ~StorageElementFactory();

        com::sun::star::uno::Reference< com::sun::star::embed::XStorage >
        createTemporaryStorage()
            throw ( com::sun::star::uno::Exception,
                    com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::embed::XStorage >
        createStorage( const rtl::OUString & rUri, StorageAccessMode eMode )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::io::XInputStream >
        createInputStream( const rtl::OUString & rUri,
                           const rtl::OUString & rPassword )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::packages::WrongPasswordException,
                    com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::io::XOutputStream >
        createOutputStream( const rtl::OUString & rUri,
                            const rtl::OUString & rPassword,
                            bool bTruncate )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::packages::WrongPasswordException,
                    com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::io::XStream >
        createStream( const rtl::OUString & rUri,
                      const rtl::OUString & rPassword,
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
        queryParentStorage( const rtl::OUString & rUri,
                            StorageAccessMode eMode )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::embed::XStorage >
        queryStorage( const com::sun::star::uno::Reference<
                        com::sun::star::embed::XStorage > & xParentStorage,
                      const rtl::OUString & rUri,
                      StorageAccessMode eMode )
            throw ( com::sun::star::embed::InvalidStorageException,
                    com::sun::star::lang::IllegalArgumentException,
                    com::sun::star::io::IOException,
                    com::sun::star::embed::StorageWrappedTargetException,
                    com::sun::star::uno::RuntimeException );

        com::sun::star::uno::Reference< com::sun::star::io::XStream >
        queryStream( const com::sun::star::uno::Reference<
                        com::sun::star::embed::XStorage > & xParentStorage,
                     const rtl::OUString & rPassword,
                     const rtl::OUString & rUri,
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
                const std::pair< rtl::OUString, bool > & s1,
                const std::pair< rtl::OUString, bool > & s2 ) const
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
            std::pair< rtl::OUString, bool >, Storage *, ltstrbool > StorageMap;

        StorageMap m_aMap;
        osl::Mutex m_aMutex;
        rtl::Reference< OfficeDocumentsManager > m_xDocsMgr;
        com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
    };

} // namespace tdoc_ucp

#endif /* !INCLUDED_TDOC_STORAGE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
