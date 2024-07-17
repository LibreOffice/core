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

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include <comphelper/comphelperdllapi.h>
#include <memory>
#include <string_view>

inline constexpr OUString PACKAGE_STORAGE_FORMAT_STRING = u"PackageFormat"_ustr;
inline constexpr OUString ZIP_STORAGE_FORMAT_STRING = u"ZipFormat"_ustr;
inline constexpr OUString OFOPXML_STORAGE_FORMAT_STRING = u"OFOPXMLFormat"_ustr;

inline constexpr OUString PACKAGE_ENCRYPTIONDATA_SHA256UTF8 = u"PackageSHA256UTF8EncryptionKey"_ustr;
inline constexpr OUString PACKAGE_ENCRYPTIONDATA_SHA1UTF8 = u"PackageSHA1UTF8EncryptionKey"_ustr;
inline constexpr OUString PACKAGE_ENCRYPTIONDATA_SHA1MS1252  = u"PackageSHA1MS1252EncryptionKey"_ustr;
inline constexpr OUString PACKAGE_ENCRYPTIONDATA_SHA1CORRECT  = u"PackageSHA1CorrectEncryptionKey"_ustr;

namespace com::sun::star {
    namespace beans { struct NamedValue; }
    namespace embed { class XStorage; }
    namespace io {
        class XInputStream;
        class XOutputStream;
        class XStream;
    }
    namespace lang { class XSingleServiceFactory; }
    namespace uno { class XComponentContext; }
    namespace awt { class XWindow; }
}

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
    /// @throws css::uno::Exception
    static css::uno::Reference< css::lang::XSingleServiceFactory >
        GetStorageFactory(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >() );

    /// @throws css::uno::Exception
    static css::uno::Reference< css::lang::XSingleServiceFactory >
        GetFileSystemStorageFactory(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >() );

    /// @throws css::uno::Exception
    static css::uno::Reference< css::embed::XStorage >
        GetTemporaryStorage(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >() );

    /// this one will only return Storage
    ///
    /// @throws css::uno::Exception
    static css::uno::Reference< css::embed::XStorage >
        GetStorageFromURL(
            const OUString& aURL,
            sal_Int32 nStorageMode,
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >() );

    /// this one will return either Storage or FileSystemStorage
    ///
    /// @throws css::uno::Exception
    static css::uno::Reference< css::embed::XStorage >
        GetStorageFromURL2(
            const OUString& aURL,
            sal_Int32 nStorageMode,
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >() );

    /// @throws css::uno::Exception
    static css::uno::Reference< css::embed::XStorage >
        GetStorageFromInputStream(
            const css::uno::Reference < css::io::XInputStream >& xStream,
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >() );

    /// @throws css::uno::Exception
    static css::uno::Reference< css::embed::XStorage >
        GetStorageFromStream(
            const css::uno::Reference < css::io::XStream >& xStream,
            sal_Int32 nStorageMode = css::embed::ElementModes::READWRITE,
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >() );

    /// @throws css::uno::Exception
    static void CopyInputToOutput(
            const css::uno::Reference< css::io::XInputStream >& xInput,
            const css::uno::Reference< css::io::XOutputStream >& xOutput );

    /// @throws css::uno::Exception
    static css::uno::Reference< css::io::XInputStream >
        GetInputStreamFromURL(
            const OUString& aURL,
            const css::uno::Reference< css::uno::XComponentContext >& context );

    /// @throws css::uno::Exception
    static void SetCommonStorageEncryptionData(
            const css::uno::Reference< css::embed::XStorage >& xStorage,
            const css::uno::Sequence< css::beans::NamedValue >& aEncryptionData );

    // the following method supports only storages of OOo formats
    /// @throws css::uno::Exception
    static sal_Int32 GetXStorageFormat(
            const css::uno::Reference< css::embed::XStorage >& xStorage );

    /// @throws css::uno::Exception
    static css::uno::Reference< css::embed::XStorage >
        GetStorageOfFormatFromURL(
            const OUString& aFormat,
            const OUString& aURL,
            sal_Int32 nStorageMode,
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >() );

    /// @throws css::uno::Exception
    static css::uno::Reference< css::embed::XStorage >
        GetStorageOfFormatFromInputStream(
            const OUString& aFormat,
            const css::uno::Reference < css::io::XInputStream >& xStream,
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >(),
            bool bRepairStorage = false );

    /// @throws css::uno::Exception
    static css::uno::Reference< css::embed::XStorage >
        GetStorageOfFormatFromStream(
            const OUString& aFormat,
            const css::uno::Reference < css::io::XStream >& xStream,
            sal_Int32 nStorageMode = css::embed::ElementModes::READWRITE,
            const css::uno::Reference< css::uno::XComponentContext >& rxContext
                            = css::uno::Reference< css::uno::XComponentContext >(),
            bool bRepairStorage = false );

    static css::uno::Sequence< css::beans::NamedValue >
        CreatePackageEncryptionData(
            std::u16string_view aPassword );

    static css::uno::Sequence< css::beans::NamedValue >
    CreateGpgPackageEncryptionData(const css::uno::Reference<css::awt::XWindow>& xParentWindow);

    static bool IsValidZipEntryFileName( std::u16string_view aName, bool bSlashAllowed );

    static bool PathHasSegment( std::u16string_view aPath, std::u16string_view aSegment );

    // Methods to allow easy use of hierarchical names inside storages

    static css::uno::Reference< css::embed::XStorage > GetStorageAtPath(
        const css::uno::Reference< css::embed::XStorage > &xStorage,
        std::u16string_view aPath, sal_uInt32 nOpenMode, LifecycleProxy const &rNastiness );
    static css::uno::Reference< css::io::XStream > GetStreamAtPath(
        const css::uno::Reference< css::embed::XStorage > &xStorage,
        std::u16string_view aPath, sal_uInt32 nOpenMode, LifecycleProxy const &rNastiness );
    static css::uno::Reference< css::io::XStream > GetStreamAtPackageURL(
        const css::uno::Reference< css::embed::XStorage > &xStorage,
        const OUString& rURL, sal_uInt32 const nOpenMode,
        LifecycleProxy const & rNastiness );

    static OUString
    GetODFVersionFromStorage(const css::uno::Reference<css::embed::XStorage>& xStorage);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
