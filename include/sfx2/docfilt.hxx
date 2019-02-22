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
#ifndef INCLUDED_SFX2_DOCFILT_HXX
#define INCLUDED_SFX2_DOCFILT_HXX

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <comphelper/documentconstants.hxx>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/types.h>
#include <sot/formats.hxx>
#include <sfx2/dllapi.h>
#include <tools/wldcrd.hxx>

#include <memory>

class SfxFilterContainer;
class SotStorage;

class SFX2_DLLPUBLIC SfxFilter
{
    friend class SfxFilterContainer;

    WildCard        aWildCard;

    OUString aTypeName;
    OUString aUserData;
    OUString aServiceName;
    OUString aMimeType;
    OUString maFilterName;
    OUString aUIName;
    OUString aDefaultTemplate;

    /**
     * Custom provider name in case the filter is provided via external
     * libraries.  Empty for conventional filter types.
     */
    OUString const maProvider;

    SfxFilterFlags  nFormatType;
    sal_Int32       nVersion;
    SotClipboardFormatId lFormat;
    bool mbEnabled;

public:
    SfxFilter( const OUString& rProvider, const OUString& rFilterName );

    SfxFilter( const OUString &rName,
               const OUString &rWildCard,
               SfxFilterFlags nFormatType,
               SotClipboardFormatId lFormat,
               const OUString &rTypeName,
               const OUString &rMimeType,
               const OUString &rUserData,
               const OUString& rServiceName,
               bool bEnabled = true );
    ~SfxFilter();

    bool IsAllowedAsTemplate() const { return bool(nFormatType & SfxFilterFlags::TEMPLATE); }
    bool IsOwnFormat() const { return bool(nFormatType & SfxFilterFlags::OWN); }
    /// If the filter supports digital signatures.
    bool GetSupportsSigning() const { return bool(nFormatType & SfxFilterFlags::SUPPORTSSIGNING); }
    bool GetGpgEncryption() const { return bool(nFormatType & SfxFilterFlags::GPGENCRYPTION); }
    bool IsOwnTemplateFormat() const { return bool(nFormatType & SfxFilterFlags::TEMPLATEPATH); }
    /// not our built-in format
    bool IsAlienFormat() const { return bool(nFormatType & SfxFilterFlags::ALIEN); }
    /// an unusual/legacy file to be loading
    bool IsExoticFormat() const { return bool(nFormatType & SfxFilterFlags::EXOTIC); }
    bool CanImport() const { return bool(nFormatType & SfxFilterFlags::IMPORT); }
    bool CanExport() const { return bool(nFormatType & SfxFilterFlags::EXPORT); }
    SfxFilterFlags  GetFilterFlags() const  { return nFormatType; }
    const OUString& GetFilterName() const { return maFilterName; }
    const OUString& GetMimeType() const { return aMimeType; }
    const OUString& GetName() const { return  maFilterName; }
    const WildCard& GetWildcard() const { return aWildCard; }
    const OUString& GetRealTypeName() const { return aTypeName; }
    SotClipboardFormatId GetFormat() const { return lFormat; }
    const OUString& GetTypeName() const { return aTypeName; }
    const OUString& GetUIName() const { return aUIName; }
    const OUString& GetUserData() const { return aUserData; }
    const OUString& GetDefaultTemplate() const { return aDefaultTemplate; }
    void            SetDefaultTemplate( const OUString& rStr ) { aDefaultTemplate = rStr; }
    bool            UsesStorage() const { return GetFormat() != SotClipboardFormatId::NONE; }
    void            SetUIName( const OUString& rName ) { aUIName = rName; }
    void            SetVersion( sal_Int32 nVersionP ) { nVersion = nVersionP; }
    sal_Int32       GetVersion() const { return nVersion; }
    OUString GetSuffixes() const;
    OUString GetDefaultExtension() const;
    const OUString& GetServiceName() const { return aServiceName; }
    const OUString& GetProviderName() const { return maProvider;}

    static std::shared_ptr<const SfxFilter> GetDefaultFilter( const OUString& rName );
    static std::shared_ptr<const SfxFilter> GetFilterByName( const OUString& rName );
    static std::shared_ptr<const SfxFilter> GetDefaultFilterFromFactory( const OUString& rServiceName );

    static OUString GetTypeFromStorage( const SotStorage& rStg );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    static OUString GetTypeFromStorage(
        const css::uno::Reference<css::embed::XStorage>& xStorage );
    bool IsEnabled() const  { return mbEnabled; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
