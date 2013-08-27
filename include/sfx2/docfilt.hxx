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
#ifndef _SFX_DOCFILT_HACK_HXX
#define _SFX_DOCFILT_HACK_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <tools/solar.h>
#include <tools/string.hxx>
#include <com/sun/star/plugin/PluginDescription.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <tools/wldcrd.hxx>

#include <comphelper/documentconstants.hxx>
#define SFX_FILTER_STARTPRESENTATION 0x20000000L

#include <sfx2/sfxdefs.hxx>

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
    sal_uIntPtr     lFormat;
    sal_uInt16      nDocIcon;

public:
    SfxFilter( const OUString& rProvider, const OUString& rFilterName );

    SfxFilter( const OUString &rName,
               const OUString &rWildCard,
               SfxFilterFlags nFormatType,
               sal_uInt32 lFormat,
               const OUString &rTypeName,
               sal_uInt16 nDocIcon,
               const OUString &rMimeType,
               const OUString &rUserData,
               const OUString& rServiceName );
    ~SfxFilter();

    bool IsAllowedAsTemplate() const { return nFormatType & SFX_FILTER_TEMPLATE; }
    bool IsOwnFormat() const { return nFormatType & SFX_FILTER_OWN; }
    bool IsOwnTemplateFormat() const { return nFormatType & SFX_FILTER_TEMPLATEPATH; }
    bool IsAlienFormat() const { return nFormatType & SFX_FILTER_ALIEN; }
    bool CanImport() const { return nFormatType & SFX_FILTER_IMPORT; }
    bool CanExport() const { return nFormatType & SFX_FILTER_EXPORT; }
    bool IsInternal() const { return nFormatType & SFX_FILTER_INTERNAL; }
    SfxFilterFlags  GetFilterFlags() const  { return nFormatType; }
    const OUString& GetFilterName() const { return maFilterName; }
    const OUString& GetMimeType() const { return aMimeType; }
    const OUString& GetName() const { return  maFilterName; }
    const WildCard& GetWildcard() const { return aWildCard; }
    const OUString& GetRealTypeName() const { return aTypeName; }
    sal_uIntPtr         GetFormat() const { return lFormat; }
    const OUString& GetTypeName() const { return aTypeName; }
    const OUString& GetUIName() const { return aUIName; }
    sal_uInt16          GetDocIconId() const { return nDocIcon; }
    const OUString& GetUserData() const { return aUserData; }
    const OUString& GetDefaultTemplate() const { return aDefaultTemplate; }
    void            SetDefaultTemplate( const OUString& rStr ) { aDefaultTemplate = rStr; }
    bool            UsesStorage() const { return GetFormat() != 0; }
    void SetURLPattern( const OUString& rStr );
    OUString GetURLPattern() const { return aPattern; }
    void            SetUIName( const OUString& rName ) { aUIName = rName; }
    void            SetVersion( sal_uIntPtr nVersionP ) { nVersion = nVersionP; }
    sal_uIntPtr           GetVersion() const { return nVersion; }
    OUString GetSuffixes() const;
    OUString GetDefaultExtension() const;
    const OUString& GetServiceName() const { return aServiceName; }
    const OUString& GetProviderName() const;

    static const SfxFilter* GetDefaultFilter( const OUString& rName );
    static const SfxFilter* GetFilterByName( const OUString& rName );
    static const SfxFilter* GetDefaultFilterFromFactory( const OUString& rServiceName );

    static OUString GetTypeFromStorage( const SotStorage& rStg );
    static OUString GetTypeFromStorage(
        const com::sun::star::uno::Reference<com::sun::star::embed::XStorage>& xStorage,
        bool bTemplate = false, OUString* pName = NULL )
            throw ( com::sun::star::beans::UnknownPropertyException,
                    com::sun::star::lang::WrappedTargetException,
                    com::sun::star::uno::RuntimeException );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
