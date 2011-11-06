/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

