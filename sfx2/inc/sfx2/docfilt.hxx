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
#ifndef _SFX_DOCFILT_HACK_HXX
#define _SFX_DOCFILT_HACK_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/plugin/PluginDescription.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <tools/wldcrd.hxx>

#include <comphelper/documentconstants.hxx>

#include <sfx2/sfxdefs.hxx>

//========================================================================
class SfxFilterContainer;
class SotStorage;
class SFX2_DLLPUBLIC SfxFilter
{
friend class SfxFilterContainer;

    WildCard        aWildCard;
    sal_uIntPtr         lFormat;
    String          aTypeName;
    String          aUserData;
    SfxFilterFlags  nFormatType;
    sal_uInt16          nDocIcon;
    String          aServiceName;
    String          aMimeType;
    String          aFilterName;
    String          aPattern;
    sal_uIntPtr           nVersion;
    String          aUIName;
    String          aDefaultTemplate;

public:
                    SfxFilter( const String &rName,
                               const String &rWildCard,
                               SfxFilterFlags nFormatType,
                               sal_uInt32 lFormat,
                               const String &rTypeName,
                               sal_uInt16 nDocIcon,
                               const String &rMimeType,
                               const String &rUserData,
                               const String& rServiceName );
                    ~SfxFilter();

    bool IsAllowedAsTemplate() const { return nFormatType & SFX_FILTER_TEMPLATE; }
    bool IsOwnFormat() const { return nFormatType & SFX_FILTER_OWN; }
    bool IsOwnTemplateFormat() const { return nFormatType & SFX_FILTER_TEMPLATEPATH; }
    bool IsAlienFormat() const { return nFormatType & SFX_FILTER_ALIEN; }
    bool CanImport() const { return nFormatType & SFX_FILTER_IMPORT; }
    bool CanExport() const { return nFormatType & SFX_FILTER_EXPORT; }
    bool IsInternal() const { return nFormatType & SFX_FILTER_INTERNAL; }
    SfxFilterFlags  GetFilterFlags() const  { return nFormatType; }
    const String&   GetFilterName() const { return aFilterName; }
    const String&   GetMimeType() const { return aMimeType; }
    const String&   GetName() const { return  aFilterName; }
    const WildCard& GetWildcard() const { return aWildCard; }
    const String&   GetRealTypeName() const { return aTypeName; }
    sal_uIntPtr         GetFormat() const { return lFormat; }
    const String&   GetTypeName() const { return aTypeName; }
    const String&   GetUIName() const { return aUIName; }
    sal_uInt16          GetDocIconId() const { return nDocIcon; }
    const String&   GetUserData() const { return aUserData; }
    const String&   GetDefaultTemplate() const { return aDefaultTemplate; }
    void            SetDefaultTemplate( const String& rStr ) { aDefaultTemplate = rStr; }
    sal_Bool            UsesStorage() const { return GetFormat() != 0; }
    void            SetURLPattern( const String& rStr ) { aPattern = rStr; aPattern.ToLowerAscii(); }
    String          GetURLPattern() const { return aPattern; }
    void            SetUIName( const String& rName ) { aUIName = rName; }
    void            SetVersion( sal_uIntPtr nVersionP ) { nVersion = nVersionP; }
    sal_uIntPtr           GetVersion() const { return nVersion; }
    String          GetSuffixes() const;
    String          GetDefaultExtension() const;
    const String&   GetServiceName() const { return aServiceName; }

    static const SfxFilter* GetDefaultFilter( const String& rName );
    static const SfxFilter* GetFilterByName( const String& rName );
    static const SfxFilter* GetDefaultFilterFromFactory( const String& rServiceName );

    static String   GetTypeFromStorage( const SotStorage& rStg );
    static String   GetTypeFromStorage( const com::sun::star::uno::Reference< com::sun::star::embed::XStorage >& xStorage,
                                        sal_Bool bTemplate = sal_False,
                                        String* pName=0 )
                        throw ( ::com::sun::star::beans::UnknownPropertyException,
                                ::com::sun::star::lang::WrappedTargetException,
                                ::com::sun::star::uno::RuntimeException );
};

#endif

