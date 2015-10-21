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
#ifndef INCLUDED_COMPHELPER_STORAGEHELPER_HXX
#define INCLUDED_COMPHELPER_STORAGEHELPER_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include <comphelper/comphelperdllapi.h>
#include <memory>

#define PACKAGE_STORAGE_FORMAT_STRING   "PackageFormat"
#define ZIP_STORAGE_FORMAT_STRING       "ZipFormat"
#define OFOPXML_STORAGE_FORMAT_STRING   "OFOPXMLFormat"

#define PACKAGE_ENCRYPTIONDATA_SHA256UTF8 "PackageSHA256UTF8EncryptionKey"
#define PACKAGE_ENCRYPTIONDATA_SHA1UTF8   "PackageSHA1UTF8EncryptionKey"
#define PACKAGE_ENCRYPTIONDATA_SHA1MS1252 "PackageSHA1MS1252EncryptionKey"

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
    std::unique_ptr<Impl> m_xBadness;
    LifecycleProxy();
    ~LifecycleProxy();
    // commit the storages: necessary for writes to streams to take effect!
    void commitStorages();
};

class COMPHELPER_DLLPUBLIC OStorageHelper
{
public:
    static css::uno::Reference< css::lang::XSingleServiceFactory >
        GetStorageFactory(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >() )
        throw ( css::uno::Exception );

    static css::uno::Reference< css::lang::XSingleServiceFactory >
        GetFileSystemStorageFactory(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >() )
        throw ( css::uno::Exception );

    static css::uno::Reference< css::embed::XStorage >
        GetTemporaryStorage(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >() )
        throw ( css::uno::Exception );

    /// this one will only return Storage
    static css::uno::Reference< css::embed::XStorage >
        GetStorageFromURL(
            const OUString& aURL,
            sal_Int32 nStorageMode,
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >() )
        throw ( css::uno::Exception );

    /// this one will return either Storage or FileSystemStorage
    static css::uno::Reference< css::embed::XStorage >
        GetStorageFromURL2(
            const OUString& aURL,
            sal_Int32 nStorageMode,
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >() )

        throw ( css::uno::Exception );

    static css::uno::Reference< css::embed::XStorage >
        GetStorageFromInputStream(
            const css::uno::Reference < css::io::XInputStream >& xStream,
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >() )
        throw ( css::uno::Exception );

    static css::uno::Reference< css::embed::XStorage >
        GetStorageFromStream(
            const css::uno::Reference < css::io::XStream >& xStream,
            sal_Int32 nStorageMode = css::embed::ElementModes::READWRITE,
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >() )
        throw ( css::uno::Exception );

    static void CopyInputToOutput(
            const css::uno::Reference< css::io::XInputStream >& xInput,
            const css::uno::Reference< css::io::XOutputStream >& xOutput )
        throw ( css::uno::Exception );

    static css::uno::Reference< css::io::XInputStream >
        GetInputStreamFromURL(
            const OUString& aURL,
            const css::uno::Reference< css::uno::XComponentContext >& context )
        throw ( css::uno::Exception );

    static void SetCommonStorageEncryptionData(
            const css::uno::Reference< css::embed::XStorage >& xStorage,
            const css::uno::Sequence< css::beans::NamedValue >& aEncryptionData )
        throw ( css::uno::Exception );

    // the following method supports only storages of OOo formats
    static sal_Int32 GetXStorageFormat(
            const css::uno::Reference< css::embed::XStorage >& xStorage ) ;

    static css::uno::Reference< css::embed::XStorage >
        GetStorageOfFormatFromURL(
            const OUString& aFormat,
            const OUString& aURL,
            sal_Int32 nStorageMode,
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >() )
        throw ( css::uno::Exception );

    static css::uno::Reference< css::embed::XStorage >
        GetStorageOfFormatFromInputStream(
            const OUString& aFormat,
            const css::uno::Reference < css::io::XInputStream >& xStream,
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >(),
            bool bRepairStorage = false )
        throw ( css::uno::Exception );

    static css::uno::Reference< css::embed::XStorage >
        GetStorageOfFormatFromStream(
            const OUString& aFormat,
            const css::uno::Reference < css::io::XStream >& xStream,
            sal_Int32 nStorageMode = css::embed::ElementModes::READWRITE,
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >(),
            bool bRepairStorage = false )
        throw ( css::uno::Exception );

    static css::uno::Sequence< css::beans::NamedValue >
        CreatePackageEncryptionData(
            const OUString& aPassword );

    static bool IsValidZipEntryFileName( const OUString& aName, bool bSlashAllowed );
    static bool IsValidZipEntryFileName( const sal_Unicode *pChar, sal_Int32 nLength, bool bSlashAllowed );

    static bool PathHasSegment( const OUString& aPath, const OUString& aSegment );

    // Methods to allow easy use of hierarchical names inside storages

    static css::uno::Reference< css::embed::XStorage > GetStorageAtPath(
        const css::uno::Reference< css::embed::XStorage > &xStorage,
        const OUString& aPath, sal_uInt32 nOpenMode, LifecycleProxy &rNastiness );
    static css::uno::Reference< css::io::XStream > GetStreamAtPath(
        const css::uno::Reference< css::embed::XStorage > &xStorage,
        const OUString& aPath, sal_uInt32 nOpenMode, LifecycleProxy &rNastiness );
    static css::uno::Reference< css::io::XStream > GetStreamAtPackageURL(
        const css::uno::Reference< css::embed::XStorage > &xStorage,
        const OUString& rURL, sal_uInt32 const nOpenMode,
        LifecycleProxy & rNastiness );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
