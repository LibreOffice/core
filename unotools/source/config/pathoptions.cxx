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

#include <sal/config.h>

#include <sal/log.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/configitem.hxx>
#include <unotools/configmgr.hxx>
#include <tools/urlobj.hxx>
#include <tools/solar.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/mutex.hxx>
#include <osl/file.hxx>
#include <unotools/bootstrap.hxx>

#include <unotools/ucbhelper.hxx>
#include <comphelper/getexpandeduri.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/thePathSettings.hpp>
#include <com/sun/star/util/PathSubstitution.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>
#include <rtl/instance.hxx>

#include "itemholder1.hxx"

#include <set>
#include <unordered_map>
#include <vector>

using namespace osl;
using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;

#define SEARCHPATH_DELIMITER  ';'
#define SIGN_STARTVARIABLE    "$("
#define SIGN_ENDVARIABLE      ")"

// Supported variables by the old SvtPathOptions implementation
#define SUBSTITUTE_INSTPATH   "$(instpath)"
#define SUBSTITUTE_PROGPATH   "$(progpath)"
#define SUBSTITUTE_USERPATH   "$(userpath)"
#define SUBSTITUTE_PATH       "$(path)"

#define STRPOS_NOTFOUND       -1

typedef std::unordered_map<OUString, sal_Int32, OUStringHash> NameToHandleMap;

typedef std::unordered_map<sal_Int32, sal_Int32> EnumToHandleMap;

typedef std::set<OUString> VarNameSet;

// class SvtPathOptions_Impl ---------------------------------------------
class SvtPathOptions_Impl
{
    private:
        // Local variables to return const references
        std::vector< OUString >             m_aPathArray;
        Reference< XFastPropertySet >       m_xPathSettings;
        Reference< XStringSubstitution >    m_xSubstVariables;
        Reference< XMacroExpander >         m_xMacroExpander;
        mutable EnumToHandleMap             m_aMapEnumToPropHandle;
        VarNameSet                          m_aSystemPathVarNames;

        LanguageTag                         m_aLanguageTag;
        OUString                            m_aEmptyString;
        mutable ::osl::Mutex                m_aMutex;

    public:
                        SvtPathOptions_Impl();

        // get the paths, not const because of using a mutex
        const OUString& GetPath( SvtPathOptions::Paths );
        const OUString& GetAddinPath() { return GetPath( SvtPathOptions::PATH_ADDIN ); }
        const OUString& GetAutoCorrectPath() { return GetPath( SvtPathOptions::PATH_AUTOCORRECT ); }
        const OUString& GetAutoTextPath() { return GetPath( SvtPathOptions::PATH_AUTOTEXT ); }
        const OUString& GetBackupPath() { return GetPath( SvtPathOptions::PATH_BACKUP ); }
        const OUString& GetBasicPath() { return GetPath( SvtPathOptions::PATH_BASIC ); }
        const OUString& GetBitmapPath() { return GetPath( SvtPathOptions::PATH_BITMAP ); }
        const OUString& GetConfigPath() { return GetPath( SvtPathOptions::PATH_CONFIG ); }
        const OUString& GetDictionaryPath() { return GetPath( SvtPathOptions::PATH_DICTIONARY ); }
        const OUString& GetFavoritesPath() { return GetPath( SvtPathOptions::PATH_FAVORITES ); }
        const OUString& GetFilterPath() { return GetPath( SvtPathOptions::PATH_FILTER ); }
        const OUString& GetGalleryPath() { return GetPath( SvtPathOptions::PATH_GALLERY ); }
        const OUString& GetGraphicPath() { return GetPath( SvtPathOptions::PATH_GRAPHIC ); }
        const OUString& GetHelpPath() { return GetPath( SvtPathOptions::PATH_HELP ); }
        const OUString& GetLinguisticPath() { return GetPath( SvtPathOptions::PATH_LINGUISTIC ); }
        const OUString& GetModulePath() { return GetPath( SvtPathOptions::PATH_MODULE ); }
        const OUString& GetPalettePath() { return GetPath( SvtPathOptions::PATH_PALETTE ); }
        const OUString& GetPluginPath() { return GetPath( SvtPathOptions::PATH_PLUGIN ); }
        const OUString& GetStoragePath() { return GetPath( SvtPathOptions::PATH_STORAGE ); }
        const OUString& GetTempPath() { return GetPath( SvtPathOptions::PATH_TEMP ); }
        const OUString& GetTemplatePath() { return GetPath( SvtPathOptions::PATH_TEMPLATE ); }
        const OUString& GetUserConfigPath() { return GetPath( SvtPathOptions::PATH_USERCONFIG ); }
        const OUString& GetWorkPath() { return GetPath( SvtPathOptions::PATH_WORK ); }
        const OUString& GetUIConfigPath() { return GetPath( SvtPathOptions::PATH_UICONFIG ); }
        const OUString& GetFingerprintPath() { return GetPath( SvtPathOptions::PATH_FINGERPRINT ); }
        const OUString& GetClassificationPath() { return GetPath( SvtPathOptions::PATH_CLASSIFICATION ); }

        // set the paths
        void            SetPath( SvtPathOptions::Paths, const OUString& rNewPath );
        void            SetAddinPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_ADDIN, rPath ); }
        void            SetAutoCorrectPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_AUTOCORRECT, rPath ); }
        void            SetAutoTextPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_AUTOTEXT, rPath ); }
        void            SetBackupPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_BACKUP, rPath ); }
        void            SetBasicPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_BASIC, rPath ); }
        void            SetBitmapPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_BITMAP, rPath ); }
        void            SetConfigPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_CONFIG, rPath ); }
        void            SetDictionaryPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_DICTIONARY, rPath ); }
        void            SetFavoritesPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_FAVORITES, rPath ); }
        void            SetFilterPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_FILTER, rPath ); }
        void            SetGalleryPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_GALLERY, rPath ); }
        void            SetGraphicPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_GRAPHIC, rPath ); }
        void            SetHelpPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_HELP, rPath ); }
        void            SetLinguisticPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_LINGUISTIC, rPath ); }
        void            SetModulePath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_MODULE, rPath ); }
        void            SetPalettePath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_PALETTE, rPath ); }
        void            SetPluginPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_PLUGIN, rPath ); }
        void            SetStoragePath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_STORAGE, rPath ); }
        void            SetTempPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_TEMP, rPath ); }
        void            SetTemplatePath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_TEMPLATE, rPath ); }
        void            SetUserConfigPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_USERCONFIG, rPath ); }
        void            SetWorkPath( const OUString& rPath ) { SetPath( SvtPathOptions::PATH_WORK, rPath ); }

        OUString   SubstVar( const OUString& rVar ) const;
        OUString   ExpandMacros( const OUString& rPath ) const;
        OUString   UsePathVariables( const OUString& rPath ) const;

        const LanguageTag& GetLanguageTag() const { return m_aLanguageTag; }
};

// global ----------------------------------------------------------------

static std::weak_ptr<SvtPathOptions_Impl> g_pOptions;

// functions -------------------------------------------------------------
struct PropertyStruct
{
    const char*             pPropName;  // The ascii name of the Office path
    SvtPathOptions::Paths   ePath;      // The enum value used by SvtPathOptions
};

struct VarNameAttribute
{
    const char*             pVarName;       // The name of the path variable
};

static const PropertyStruct aPropNames[] =
{
    { "Addin",          SvtPathOptions::PATH_ADDIN          },
    { "AutoCorrect",    SvtPathOptions::PATH_AUTOCORRECT    },
    { "AutoText",       SvtPathOptions::PATH_AUTOTEXT       },
    { "Backup",         SvtPathOptions::PATH_BACKUP         },
    { "Basic",          SvtPathOptions::PATH_BASIC          },
    { "Bitmap",         SvtPathOptions::PATH_BITMAP         },
    { "Config",         SvtPathOptions::PATH_CONFIG         },
    { "Dictionary",     SvtPathOptions::PATH_DICTIONARY     },
    { "Favorite",       SvtPathOptions::PATH_FAVORITES      },
    { "Filter",         SvtPathOptions::PATH_FILTER         },
    { "Gallery",        SvtPathOptions::PATH_GALLERY        },
    { "Graphic",        SvtPathOptions::PATH_GRAPHIC        },
    { "Help",           SvtPathOptions::PATH_HELP           },
    { "Linguistic",     SvtPathOptions::PATH_LINGUISTIC     },
    { "Module",         SvtPathOptions::PATH_MODULE         },
    { "Palette",        SvtPathOptions::PATH_PALETTE        },
    { "Plugin",         SvtPathOptions::PATH_PLUGIN         },
    { "Storage",        SvtPathOptions::PATH_STORAGE        },
    { "Temp",           SvtPathOptions::PATH_TEMP           },
    { "Template",       SvtPathOptions::PATH_TEMPLATE       },
    { "UserConfig",     SvtPathOptions::PATH_USERCONFIG     },
    { "Work",           SvtPathOptions::PATH_WORK           },
    { "UIConfig",       SvtPathOptions::PATH_UICONFIG       },
    { "Fingerprint",    SvtPathOptions::PATH_FINGERPRINT    },
    { "Classification", SvtPathOptions::PATH_CLASSIFICATION }
};

static const VarNameAttribute aVarNameAttribute[] =
{
    { SUBSTITUTE_INSTPATH },    // $(instpath)
    { SUBSTITUTE_PROGPATH },    // $(progpath)
    { SUBSTITUTE_USERPATH },    // $(userpath)
    { SUBSTITUTE_PATH },    // $(path)
};

// class SvtPathOptions_Impl ---------------------------------------------

const OUString& SvtPathOptions_Impl::GetPath( SvtPathOptions::Paths ePath )
{
    if ( ePath >= SvtPathOptions::PATH_COUNT )
        return m_aEmptyString;

    ::osl::MutexGuard aGuard( m_aMutex );

    try
    {
        OUString    aPathValue;
        OUString    aResult;
        sal_Int32   nHandle = m_aMapEnumToPropHandle[ (sal_Int32)ePath ];

        // Substitution is done by the service itself using the substition service
        Any         a = m_xPathSettings->getFastPropertyValue( nHandle );
        a >>= aPathValue;
        if( ePath == SvtPathOptions::PATH_ADDIN     ||
            ePath == SvtPathOptions::PATH_FILTER    ||
            ePath == SvtPathOptions::PATH_HELP      ||
            ePath == SvtPathOptions::PATH_MODULE    ||
            ePath == SvtPathOptions::PATH_PLUGIN    ||
            ePath == SvtPathOptions::PATH_STORAGE
          )
        {
            // These office paths have to be converted to system pates
            osl::FileBase::getSystemPathFromFileURL( aPathValue, aResult );
            aPathValue = aResult;
        }
        else if (ePath == SvtPathOptions::PATH_PALETTE)
        {
            auto ctx = comphelper::getProcessComponentContext();
            OUStringBuffer buf;
            for (sal_Int32 i = 0;;)
            {
                buf.append(
                    comphelper::getExpandedUri(
                        ctx, aPathValue.getToken(0, ';', i)));
                if (i == -1) {
                    break;
                }
                buf.append(';');
            }
            aPathValue = buf.makeStringAndClear();
        }

        m_aPathArray[ ePath ] = aPathValue;
        return m_aPathArray[ ePath ];
    }
    catch (UnknownPropertyException &)
    {
    }

    return m_aEmptyString;
}

void SvtPathOptions_Impl::SetPath( SvtPathOptions::Paths ePath, const OUString& rNewPath )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( ePath < SvtPathOptions::PATH_COUNT )
    {
        OUString    aResult;
        OUString    aNewValue;
        Any         a;

        switch ( ePath )
        {
            case SvtPathOptions::PATH_ADDIN:
            case SvtPathOptions::PATH_FILTER:
            case SvtPathOptions::PATH_HELP:
            case SvtPathOptions::PATH_MODULE:
            case SvtPathOptions::PATH_PLUGIN:
            case SvtPathOptions::PATH_STORAGE:
            {
                // These office paths have to be convert back to UCB-URL's
                osl::FileBase::getFileURLFromSystemPath( rNewPath, aResult );
                aNewValue = aResult;
            }
            break;

            default:
                aNewValue = rNewPath;
        }

        // Resubstitution is done by the service itself using the substition service
        a <<= aNewValue;
        try
        {
            m_xPathSettings->setFastPropertyValue( m_aMapEnumToPropHandle[ (sal_Int32)ePath], a );
        }
        catch (const Exception& e)
        {
            SAL_WARN("unotools.config", "SetPath: exception: " << e.Message);
        }
    }
}

OUString SvtPathOptions_Impl::ExpandMacros( const OUString& rPath ) const
{
    OUString sExpanded( rPath );

    const INetURLObject aParser( rPath );
    if ( aParser.GetProtocol() == INetProtocol::VndSunStarExpand )
        sExpanded = m_xMacroExpander->expandMacros( aParser.GetURLPath( INetURLObject::DecodeMechanism::WithCharset ) );

    return sExpanded;
}

OUString SvtPathOptions_Impl::UsePathVariables( const OUString& rPath ) const
{
    return m_xSubstVariables->reSubstituteVariables( rPath );
}

OUString SvtPathOptions_Impl::SubstVar( const OUString& rVar ) const
{
    // Don't work at parameter-string directly. Copy it.
    OUString aWorkText = rVar;

    // Convert the returned path to system path!
    bool bConvertLocal = false;

    // Search for first occurrence of "$(...".
    sal_Int32 nPosition = aWorkText.indexOf( SIGN_STARTVARIABLE );  // = first position of "$(" in string
    sal_Int32 nLength   = 0;                                        // = count of letters from "$(" to ")" in string

    // Have we found any variable like "$(...)"?
    if ( nPosition != STRPOS_NOTFOUND )
    {
        // Yes; Get length of found variable.
        // If no ")" was found - nLength is set to 0 by default! see before.
        sal_Int32 nEndPosition = aWorkText.indexOf( SIGN_ENDVARIABLE, nPosition );
        if ( nEndPosition != STRPOS_NOTFOUND )
            nLength = nEndPosition - nPosition + 1;
    }

    // Is there another path variable?
    while ( ( nPosition != STRPOS_NOTFOUND ) && ( nLength > 0 ) )
    {
        // YES; Get the next variable for replace.
        OUString aSubString = aWorkText.copy( nPosition, nLength );
        aSubString = aSubString.toAsciiLowerCase();

        // Look for special variable that needs a system path.
        VarNameSet::const_iterator pIter = m_aSystemPathVarNames.find( aSubString );
        if ( pIter != m_aSystemPathVarNames.end() )
            bConvertLocal = true;

        nPosition += nLength;

        // We must control index in string before call something at OUString!
        // The OUString-implementation don't do it for us :-( but the result is not defined otherwise.
        if ( nPosition + 1 > aWorkText.getLength() )
        {
            // Position is out of range. Break loop!
            nPosition = STRPOS_NOTFOUND;
            nLength = 0;
        }
        else
        {
            // Else; Position is valid. Search for next variable.
            nPosition = aWorkText.indexOf( SIGN_STARTVARIABLE, nPosition );
            // Have we found any variable like "$(...)"?
            if ( nPosition != STRPOS_NOTFOUND )
            {
                // Yes; Get length of found variable. If no ")" was found - nLength must set to 0!
                nLength = 0;
                sal_Int32 nEndPosition = aWorkText.indexOf( SIGN_ENDVARIABLE, nPosition );
                if ( nEndPosition != STRPOS_NOTFOUND )
                    nLength = nEndPosition - nPosition + 1;
            }
        }
    }

    aWorkText = m_xSubstVariables->substituteVariables( rVar, false );

    if ( bConvertLocal )
    {
        // Convert the URL to a system path for special path variables
        OUString aReturn;
        osl::FileBase::getSystemPathFromFileURL( aWorkText, aReturn );
        return aReturn;
    }

    return aWorkText;
}

SvtPathOptions_Impl::SvtPathOptions_Impl() :
    m_aPathArray( (sal_Int32)SvtPathOptions::PATH_COUNT ),
    m_aLanguageTag( LANGUAGE_DONTKNOW )
{
    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();

    // Create necessary services
    Reference< XPathSettings > xPathSettings = thePathSettings::get(xContext);
    m_xPathSettings.set( xPathSettings, UNO_QUERY_THROW );
    m_xSubstVariables.set( PathSubstitution::create(xContext) );
    m_xMacroExpander = theMacroExpander::get(xContext);

    // Create temporary hash map to have a mapping between property names and property handles
    Reference< XPropertySetInfo > xPropSetInfo = xPathSettings->getPropertySetInfo();
    Sequence< Property > aPathPropSeq = xPropSetInfo->getProperties();

    NameToHandleMap aTempHashMap;
    for ( sal_Int32 n = 0; n < aPathPropSeq.getLength(); n++ )
    {
        const css::beans::Property& aProperty = aPathPropSeq[n];
        aTempHashMap.insert( NameToHandleMap::value_type( aProperty.Name, aProperty.Handle ));
    }

    // Create mapping between internal enum (SvtPathOptions::Paths) and property handle
    sal_Int32 nCount = sizeof( aPropNames ) / sizeof( PropertyStruct );
    sal_Int32 i;
    for ( i = 0; i < nCount; i++ )
    {
        NameToHandleMap::const_iterator pIter =
            aTempHashMap.find( OUString::createFromAscii( aPropNames[i].pPropName ));

        if ( pIter != aTempHashMap.end() )
        {
            sal_Int32 nHandle   = pIter->second;
            sal_Int32 nEnum     = aPropNames[i].ePath;
            m_aMapEnumToPropHandle.insert( EnumToHandleMap::value_type( nEnum, nHandle ));
        }
    }

    // Create hash map for path variables that need a system path as a return value!
    nCount = sizeof( aVarNameAttribute ) / sizeof( VarNameAttribute );
    for ( i = 0; i < nCount; i++ )
    {
        m_aSystemPathVarNames.insert( OUString::createFromAscii( aVarNameAttribute[i].pVarName ) );
    }

    // Set language type!
    m_aLanguageTag.reset( ConfigManager::getLocale() );
}

// class SvtPathOptions --------------------------------------------------

namespace { struct lclMutex : public rtl::Static< ::osl::Mutex, lclMutex > {}; }

SvtPathOptions::SvtPathOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( lclMutex::get() );
    pImpl = g_pOptions.lock();
    if ( !pImpl )
    {
        pImpl = std::make_shared<SvtPathOptions_Impl>();
        g_pOptions = pImpl;
        ItemHolder1::holdConfigItem(EItem::PathOptions);
    }
}

SvtPathOptions::~SvtPathOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( lclMutex::get() );

    pImpl.reset();
}

const OUString& SvtPathOptions::GetAddinPath() const
{
    return pImpl->GetAddinPath();
}

const OUString& SvtPathOptions::GetAutoCorrectPath() const
{
    return pImpl->GetAutoCorrectPath();
}

const OUString& SvtPathOptions::GetAutoTextPath() const
{
    return pImpl->GetAutoTextPath();
}

const OUString& SvtPathOptions::GetBackupPath() const
{
    return pImpl->GetBackupPath();
}

const OUString& SvtPathOptions::GetBasicPath() const
{
    return pImpl->GetBasicPath();
}

const OUString& SvtPathOptions::GetBitmapPath() const
{
    return pImpl->GetBitmapPath();
}

const OUString& SvtPathOptions::GetConfigPath() const
{
    return pImpl->GetConfigPath();
}

const OUString& SvtPathOptions::GetDictionaryPath() const
{
    return pImpl->GetDictionaryPath();
}

const OUString& SvtPathOptions::GetFavoritesPath() const
{
    return pImpl->GetFavoritesPath();
}

const OUString& SvtPathOptions::GetFilterPath() const
{
    return pImpl->GetFilterPath();
}

const OUString& SvtPathOptions::GetGalleryPath() const
{
    return pImpl->GetGalleryPath();
}

const OUString& SvtPathOptions::GetGraphicPath() const
{
    return pImpl->GetGraphicPath();
}

const OUString& SvtPathOptions::GetHelpPath() const
{
    return pImpl->GetHelpPath();
}

const OUString& SvtPathOptions::GetLinguisticPath() const
{
    return pImpl->GetLinguisticPath();
}

const OUString& SvtPathOptions::GetFingerprintPath() const
{
    return pImpl->GetFingerprintPath();
}

const OUString& SvtPathOptions::GetModulePath() const
{
    return pImpl->GetModulePath();
}

const OUString& SvtPathOptions::GetPalettePath() const
{
    return pImpl->GetPalettePath();
}

const OUString& SvtPathOptions::GetPluginPath() const
{
    return pImpl->GetPluginPath();
}

const OUString& SvtPathOptions::GetStoragePath() const
{
    return pImpl->GetStoragePath();
}

const OUString& SvtPathOptions::GetTempPath() const
{
    return pImpl->GetTempPath();
}

const OUString& SvtPathOptions::GetTemplatePath() const
{
    return pImpl->GetTemplatePath();
}

const OUString& SvtPathOptions::GetUserConfigPath() const
{
    return pImpl->GetUserConfigPath();
}

const OUString& SvtPathOptions::GetWorkPath() const
{
    return pImpl->GetWorkPath();
}

const OUString& SvtPathOptions::GetClassificationPath() const
{
    return pImpl->GetClassificationPath();
}

void SvtPathOptions::SetAddinPath( const OUString& rPath )
{
    pImpl->SetAddinPath( rPath );
}

void SvtPathOptions::SetAutoCorrectPath( const OUString& rPath )
{
    pImpl->SetAutoCorrectPath( rPath );
}

void SvtPathOptions::SetAutoTextPath( const OUString& rPath )
{
    pImpl->SetAutoTextPath( rPath );
}

void SvtPathOptions::SetBackupPath( const OUString& rPath )
{
    pImpl->SetBackupPath( rPath );
}

void SvtPathOptions::SetBasicPath( const OUString& rPath )
{
    pImpl->SetBasicPath( rPath );
}

void SvtPathOptions::SetBitmapPath( const OUString& rPath )
{
    pImpl->SetBitmapPath( rPath );
}

void SvtPathOptions::SetConfigPath( const OUString& rPath )
{
    pImpl->SetConfigPath( rPath );
}

void SvtPathOptions::SetDictionaryPath( const OUString& rPath )
{
    pImpl->SetDictionaryPath( rPath );
}

void SvtPathOptions::SetFavoritesPath( const OUString& rPath )
{
    pImpl->SetFavoritesPath( rPath );
}

void SvtPathOptions::SetFilterPath( const OUString& rPath )
{
    pImpl->SetFilterPath( rPath );
}

void SvtPathOptions::SetGalleryPath( const OUString& rPath )
{
    pImpl->SetGalleryPath( rPath );
}

void SvtPathOptions::SetGraphicPath( const OUString& rPath )
{
    pImpl->SetGraphicPath( rPath );
}

void SvtPathOptions::SetHelpPath( const OUString& rPath )
{
    pImpl->SetHelpPath( rPath );
}

void SvtPathOptions::SetLinguisticPath( const OUString& rPath )
{
    pImpl->SetLinguisticPath( rPath );
}

void SvtPathOptions::SetModulePath( const OUString& rPath )
{
    pImpl->SetModulePath( rPath );
}

void SvtPathOptions::SetPalettePath( const OUString& rPath )
{
    pImpl->SetPalettePath( rPath );
}

void SvtPathOptions::SetPluginPath( const OUString& rPath )
{
    pImpl->SetPluginPath( rPath );
}

void SvtPathOptions::SetStoragePath( const OUString& rPath )
{
    pImpl->SetStoragePath( rPath );
}

void SvtPathOptions::SetTempPath( const OUString& rPath )
{
    pImpl->SetTempPath( rPath );
}

void SvtPathOptions::SetTemplatePath( const OUString& rPath )
{
    pImpl->SetTemplatePath( rPath );
}

void SvtPathOptions::SetUserConfigPath( const OUString& rPath )
{
    pImpl->SetUserConfigPath( rPath );
}

void SvtPathOptions::SetWorkPath( const OUString& rPath )
{
    pImpl->SetWorkPath( rPath );
}

OUString SvtPathOptions::SubstituteVariable( const OUString& rVar ) const
{
    return pImpl->SubstVar( rVar );
}

OUString SvtPathOptions::ExpandMacros( const OUString& rPath ) const
{
    return pImpl->ExpandMacros( rPath );
}

OUString SvtPathOptions::UseVariable( const OUString& rPath ) const
{
    return pImpl->UsePathVariables( rPath );
}

bool SvtPathOptions::SearchFile( OUString& rIniFile, Paths ePath )
{
    // check parameter: empty inifile name?
    if ( rIniFile.isEmpty() )
    {
        SAL_WARN( "unotools.config", "SvtPathOptions::SearchFile(): invalid parameter" );
        return false;
    }

    OUString aIniFile = pImpl->SubstVar( rIniFile );
    bool bRet = false;

    switch ( ePath )
    {
        case PATH_USERCONFIG:
        {
            // path is a URL
            bRet = true;
            INetURLObject aObj( GetUserConfigPath() );

            sal_Int32 nIniIndex = 0;
            do
            {
                OUString aToken = aIniFile.getToken( 0, '/', nIniIndex );
                aObj.insertName(aToken);
            }
            while ( nIniIndex >= 0 );

            if ( !::utl::UCBContentHelper::Exists( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) ) )
            {
                aObj.SetSmartURL( GetConfigPath() );
                aObj.insertName( aIniFile );
                bRet = ::utl::UCBContentHelper::Exists( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
            }

            if ( bRet )
                rIniFile = aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

            break;
        }

        default:
        {
            OUString aPath;
            switch ( ePath )
            {
                case PATH_ADDIN:        aPath = GetAddinPath();         break;
                case PATH_AUTOCORRECT:  aPath = GetAutoCorrectPath();   break;
                case PATH_AUTOTEXT:     aPath = GetAutoTextPath();      break;
                case PATH_BACKUP:       aPath = GetBackupPath();        break;
                case PATH_BASIC:        aPath = GetBasicPath();         break;
                case PATH_BITMAP:       aPath = GetBitmapPath();        break;
                case PATH_CONFIG:       aPath = GetConfigPath();        break;
                case PATH_DICTIONARY:   aPath = GetDictionaryPath();    break;
                case PATH_FAVORITES:    aPath = GetFavoritesPath();     break;
                case PATH_FILTER:       aPath = GetFilterPath();        break;
                case PATH_GALLERY:      aPath = GetGalleryPath();       break;
                case PATH_GRAPHIC:      aPath = GetGraphicPath();       break;
                case PATH_HELP:         aPath = GetHelpPath();          break;
                case PATH_LINGUISTIC:   aPath = GetLinguisticPath();    break;
                case PATH_MODULE:       aPath = GetModulePath();        break;
                case PATH_PALETTE:      aPath = GetPalettePath();       break;
                case PATH_PLUGIN:       aPath = GetPluginPath();        break;
                case PATH_STORAGE:      aPath = GetStoragePath();       break;
                case PATH_TEMP:         aPath = GetTempPath();          break;
                case PATH_TEMPLATE:     aPath = GetTemplatePath();      break;
                case PATH_WORK:         aPath = GetWorkPath();          break;
                case PATH_UICONFIG:     aPath = pImpl->GetUIConfigPath(); break;
                case PATH_FINGERPRINT:  aPath = GetFingerprintPath();   break;
                case PATH_CLASSIFICATION: aPath = GetClassificationPath(); break;
                // coverity[dead_error_begin] - following conditions exist to avoid compiler warning
                case PATH_USERCONFIG:
                case PATH_COUNT:
                    break;
            }

            sal_Int32 nPathIndex = 0;
            do
            {
                bool bIsURL = true;
                OUString aPathToken = aPath.getToken( 0, SEARCHPATH_DELIMITER, nPathIndex );
                INetURLObject aObj( aPathToken );
                if ( aObj.HasError() )
                {
                    bIsURL = false;
                    OUString aURL;
                    if ( osl::FileBase::getFileURLFromSystemPath( aPathToken, aURL )
                         == osl::FileBase::E_None )
                        aObj.SetURL( aURL );
                }
                if ( aObj.GetProtocol() == INetProtocol::VndSunStarExpand )
                {
                    Reference< XMacroExpander > xMacroExpander = theMacroExpander::get( ::comphelper::getProcessComponentContext() );
                    const OUString sExpandedPath = xMacroExpander->expandMacros( aObj.GetURLPath( INetURLObject::DecodeMechanism::WithCharset ) );
                    aObj.SetURL( sExpandedPath );
                }

                sal_Int32 nIniIndex = 0;
                do
                {
                    OUString aToken = aIniFile.getToken( 0, '/', nIniIndex );
                    aObj.insertName(aToken);
                }
                while ( nIniIndex >= 0 );

                bRet = ::utl::UCBContentHelper::Exists( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

                if ( bRet )
                {
                    if ( !bIsURL )
                    {
                        OUString sTmp;
                        osl::FileBase::getSystemPathFromFileURL(
                                            aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ), sTmp );
                        rIniFile = sTmp;
                    }
                    else
                        rIniFile = aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
                    break;
                }
            }
            while ( nPathIndex >= 0 );
        }
    }

    return bRet;
}

const LanguageTag& SvtPathOptions::GetLanguageTag() const
{
    return pImpl->GetLanguageTag();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
