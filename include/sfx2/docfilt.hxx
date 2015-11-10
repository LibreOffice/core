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

#include <com/sun/star/plugin/PluginDescription.hpp>
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
    OUString aPattern;
    OUString aUIName;
    OUString aDefaultTemplate;

    /**
     * Custom provider name in case the filter is provided via external
     * libraries.  Empty for conventional filter types.
     */
    OUString maProvider;

    SfxFilterFlags  nFormatType;
    sal_uIntPtr     nVersion;
    SotClipboardFormatId lFormat;
    sal_uInt16      nDocIcon;

public:
    SfxFilter( const OUString& rProvider, const OUString& rFilterName );

    SfxFilter( const OUString &rName,
               const OUString &rWildCard,
               SfxFilterFlags nFormatType,
               SotClipboardFormatId lFormat,
               const OUString &rTypeName,
               sal_uInt16 nDocIcon,
               const OUString &rMimeType,
               const OUString &rUserData,
               const OUString& rServiceName );
    ~SfxFilter();

    bool IsAllowedAsTemplate() const { return bool(nFormatType & SfxFilterFlags::TEMPLATE); }
    bool IsOwnFormat() const { return bool(nFormatType & SfxFilterFlags::OWN); }
    bool IsOwnTemplateFormat() const { return bool(nFormatType & SfxFilterFlags::TEMPLATEPATH); }
    bool IsAlienFormat() const { return bool(nFormatType & SfxFilterFlags::ALIEN); }
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
    void SetURLPattern( const OUString& rStr );
    void            SetUIName( const OUString& rName ) { aUIName = rName; }
    void            SetVersion( sal_uIntPtr nVersionP ) { nVersion = nVersionP; }
    sal_uIntPtr           GetVersion() const { return nVersion; }
    OUString GetSuffixes() const;
    OUString GetDefaultExtension() const;
    const OUString& GetServiceName() const { return aServiceName; }
    const OUString& GetProviderName() const { return maProvider;}

    static const SfxFilter* GetDefaultFilter( const OUString& rName );
    static const SfxFilter* GetFilterByName( const OUString& rName );
    static const SfxFilter* GetDefaultFilterFromFactory( const OUString& rServiceName );

    static OUString GetTypeFromStorage( const SotStorage& rStg );
    static OUString GetTypeFromStorage(
        const css::uno::Reference<css::embed::XStorage>& xStorage,
        bool bTemplate = false, OUString* pName = nullptr )
            throw ( css::beans::UnknownPropertyException,
                    css::lang::WrappedTargetException,
                    css::uno::RuntimeException,
                    std::exception );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
