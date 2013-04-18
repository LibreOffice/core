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
#ifndef COMPHELPER_STORAGEHELPER_HXX
#define COMPHELPER_STORAGEHELPER_HXX

#include <boost/scoped_ptr.hpp>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include "comphelper/comphelperdllapi.h"


#define PACKAGE_STORAGE_FORMAT_STRING   OUString( "PackageFormat" )
#define ZIP_STORAGE_FORMAT_STRING       OUString( "ZipFormat" )
#define OFOPXML_STORAGE_FORMAT_STRING   OUString( "OFOPXMLFormat" )

#define PACKAGE_ENCRYPTIONDATA_SHA256UTF8 OUString( "PackageSHA256UTF8EncryptionKey" )
#define PACKAGE_ENCRYPTIONDATA_SHA1UTF8   OUString( "PackageSHA1UTF8EncryptionKey" )
#define PACKAGE_ENCRYPTIONDATA_SHA1MS1252 OUString( "PackageSHA1MS1252EncryptionKey" )

namespace com { namespace sun { namespace star {
    namespace beans { struct NamedValue; }
    namespace embed { class XStorage; }
    namespace io {
        class XInputStream;
        class XOutputStream;
        class XStream;
    }
    namespace lang { class XSingleServiceFactory; }
    namespace uno { class XComponentContext; }
} } }

namespace comphelper {

// Unfortunately - the impl.s of XStorage like to invalidate all
// their sub streams and storages when you release references, so
// it is necessary to keep references to all storages down the
// path - this is 'beautiful' (TM). So we need this ugly hack:
class COMPHELPER_DLLPUBLIC LifecycleProxy
{
private:
    class Impl;
public:
    ::boost::scoped_ptr<Impl> m_pBadness;
    LifecycleProxy();
    ~LifecycleProxy();
    // commit the storages: necessary for writes to streams to take effect!
    void commitStorages();
};

class COMPHELPER_DLLPUBLIC OStorageHelper
{
public:
    static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >
        GetStorageFactory(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
                            = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >() )
        throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >
        GetFileSystemStorageFactory(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
                            = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >() )
        throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        GetTemporaryStorage(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
                            = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >() )
        throw ( ::com::sun::star::uno::Exception );

    /// this one will only return Storage
    static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        GetStorageFromURL(
            const OUString& aURL,
            sal_Int32 nStorageMode,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
                            = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >() )
        throw ( ::com::sun::star::uno::Exception );

    /// this one will return either Storage or FileSystemStorage
    static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        GetStorageFromURL2(
            const OUString& aURL,
            sal_Int32 nStorageMode,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
                            = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >() )

        throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        GetStorageFromInputStream(
            const ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream >& xStream,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
                            = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >() )
        throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        GetStorageFromStream(
            const ::com::sun::star::uno::Reference < ::com::sun::star::io::XStream >& xStream,
            sal_Int32 nStorageMode = ::com::sun::star::embed::ElementModes::READWRITE,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
                            = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >() )
        throw ( ::com::sun::star::uno::Exception );

    static void CopyInputToOutput(
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xInput,
            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& xOutput )
        throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
        GetInputStreamFromURL(
            const OUString& aURL,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& context )
        throw ( ::com::sun::star::uno::Exception );

    static void SetCommonStorageEncryptionData(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aEncryptionData )
        throw ( ::com::sun::star::uno::Exception );

    // the following method supports only storages of OOo formats
    static sal_Int32 GetXStorageFormat(
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage )
        throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        GetStorageOfFormatFromURL(
            const OUString& aFormat,
            const OUString& aURL,
            sal_Int32 nStorageMode,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
                            = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >(),
            sal_Bool bRepairStorage = sal_False )
        throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        GetStorageOfFormatFromInputStream(
            const OUString& aFormat,
            const ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream >& xStream,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
                            = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >(),
            sal_Bool bRepairStorage = sal_False )
        throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        GetStorageOfFormatFromStream(
            const OUString& aFormat,
            const ::com::sun::star::uno::Reference < ::com::sun::star::io::XStream >& xStream,
            sal_Int32 nStorageMode = ::com::sun::star::embed::ElementModes::READWRITE,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
                            = ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >(),
            sal_Bool bRepairStorage = sal_False )
        throw ( ::com::sun::star::uno::Exception );

    static ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >
        CreatePackageEncryptionData(
            const OUString& aPassword );

    static sal_Bool IsValidZipEntryFileName( const OUString& aName, sal_Bool bSlashAllowed );
    static sal_Bool IsValidZipEntryFileName( const sal_Unicode *pChar, sal_Int32 nLength, sal_Bool bSlashAllowed );

    static sal_Bool PathHasSegment( const OUString& aPath, const OUString& aSegment );

    // Methods to allow easy use of hierachical names inside storages

    static ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > GetStorageAtPath(
        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > &xStorage,
        const OUString& aPath, sal_uInt32 nOpenMode, LifecycleProxy &rNastiness );
    static ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > GetStreamAtPath(
        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > &xStorage,
        const OUString& aPath, sal_uInt32 nOpenMode, LifecycleProxy &rNastiness );
    static ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream > GetStreamAtPackageURL(
        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > &xStorage,
        const OUString& rURL, sal_uInt32 const nOpenMode,
        LifecycleProxy & rNastiness );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
