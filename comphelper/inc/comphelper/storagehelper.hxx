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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include "comphelper/comphelperdllapi.h"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::embed;
using namespace com::sun::star::io;
using namespace com::sun::star::beans;
using rtl::OUString;

#define PACKAGE_STORAGE_FORMAT_STRING   OUString( RTL_CONSTASCII_USTRINGPARAM( "PackageFormat" ) )
#define ZIP_STORAGE_FORMAT_STRING       OUString( RTL_CONSTASCII_USTRINGPARAM( "ZipFormat" ) )
#define OFOPXML_STORAGE_FORMAT_STRING   OUString( RTL_CONSTASCII_USTRINGPARAM( "OFOPXMLFormat" ) )

#define PACKAGE_ENCRYPTIONDATA_SHA256UTF8 OUString( RTL_CONSTASCII_USTRINGPARAM( "PackageSHA256UTF8EncryptionKey" ) )
#define PACKAGE_ENCRYPTIONDATA_SHA1UTF8   OUString( RTL_CONSTASCII_USTRINGPARAM( "PackageSHA1UTF8EncryptionKey" ) )
#define PACKAGE_ENCRYPTIONDATA_SHA1MS1252 OUString( RTL_CONSTASCII_USTRINGPARAM( "PackageSHA1MS1252EncryptionKey" ) )

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
    static Reference< XSingleServiceFactory >
        GetStorageFactory(
            const Reference< XMultiServiceFactory >& xSF = Reference< XMultiServiceFactory >() )
        throw ( Exception );

    static Reference< XSingleServiceFactory >
        GetFileSystemStorageFactory(
            const Reference< XMultiServiceFactory >& xSF = Reference< XMultiServiceFactory >() )
        throw ( Exception );

    static Reference< XStorage >
        GetTemporaryStorage(
            const Reference< XMultiServiceFactory >& xFactory = Reference< XMultiServiceFactory >() )
        throw ( Exception );

    // this one will only return Storage
    static Reference< XStorage >
        GetStorageFromURL(
            const OUString& aURL, sal_Int32 nStorageMode,
            const Reference< XMultiServiceFactory >& xFactory = Reference< XMultiServiceFactory >() )
        throw ( Exception );

    // this one will return either Storage or FileSystemStorage
    static Reference< XStorage >
        GetStorageFromURL2(
            const OUString& aURL, sal_Int32 nStorageMode,
            const Reference< XMultiServiceFactory >& xFactory = Reference< XMultiServiceFactory >() )
        throw ( Exception );

    static Reference< XStorage >
        GetStorageFromInputStream(
            const Reference < XInputStream >& xStream,
            const Reference< XMultiServiceFactory >& xFactory = Reference< XMultiServiceFactory >() )
        throw ( Exception );

    static Reference< XStorage >
        GetStorageFromStream(
            const Reference < XStream >& xStream,
            sal_Int32 nStorageMode = ElementModes::READWRITE,
            const Reference< XMultiServiceFactory >& xFactory = Reference< XMultiServiceFactory >() )
        throw ( Exception );

    static void CopyInputToOutput(
            const Reference< XInputStream >& xInput,
            const Reference< XOutputStream >& xOutput )
        throw ( Exception );

    static Reference< XInputStream >
        GetInputStreamFromURL(
            const OUString& aURL,
            const Reference< XMultiServiceFactory >& xFactory = Reference< XMultiServiceFactory >() )
        throw ( Exception );

    static void SetCommonStorageEncryptionData(
            const Reference< XStorage >& xStorage,
            const Sequence< NamedValue >& aEncryptionData )
        throw ( Exception );

    // the following method supports only storages of OOo formats
    static sal_Int32 GetXStorageFormat(
            const Reference< XStorage >& xStorage )
        throw ( Exception );

    static Reference< XStorage >
        GetStorageOfFormatFromURL(
            const OUString& aFormat,
            const OUString& aURL,
            sal_Int32 nStorageMode,
            const Reference< XMultiServiceFactory >& xFactory
                            = Reference< XMultiServiceFactory >(),
            sal_Bool bRepairStorage = sal_False )
        throw ( Exception );

    static Reference< XStorage >
        GetStorageOfFormatFromInputStream(
            const OUString& aFormat,
            const Reference < XInputStream >& xStream,
            const Reference< XMultiServiceFactory >& xFactory
                            = Reference< XMultiServiceFactory >(),
            sal_Bool bRepairStorage = sal_False )
        throw ( Exception );

    static Reference< XStorage >
        GetStorageOfFormatFromStream(
            const OUString& aFormat,
            const Reference < XStream >& xStream,
            sal_Int32 nStorageMode = ElementModes::READWRITE,
            const Reference< XMultiServiceFactory >& xFactory = Reference< XMultiServiceFactory >(),
            sal_Bool bRepairStorage = sal_False )
        throw ( Exception );

    static Sequence< NamedValue >
        CreatePackageEncryptionData(
            const OUString& aPassword,
            const Reference< XMultiServiceFactory >& xSF = Reference< XMultiServiceFactory >() );

    static sal_Bool IsValidZipEntryFileName( const OUString& aName, sal_Bool bSlashAllowed );
    static sal_Bool IsValidZipEntryFileName( const sal_Unicode *pChar, sal_Int32 nLength, sal_Bool bSlashAllowed );

    static sal_Bool PathHasSegment( const OUString& aPath, const OUString& aSegment );

    // Methods to allow easy use of hierachical names inside storages

    static Reference< XStorage > GetStorageAtPath(
        const Reference< XStorage > &xStorage,
        const OUString& aPath, sal_uInt32 nOpenMode, LifecycleProxy &rNastiness );

    static Reference< XStream > GetStreamAtPath(
        const Reference< XStorage > &xStorage,
        const OUString& aPath, sal_uInt32 nOpenMode, LifecycleProxy &rNastiness );

    static Reference< XStream > GetStreamAtPackageURL(
        const Reference< XStorage > &xStorage,
        const OUString& rURL, sal_uInt32 const nOpenMode,
        LifecycleProxy & rNastiness );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
