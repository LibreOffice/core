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
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/mutex.hxx>
#include <osl/file.hxx>

#include <unotools/ucbhelper.hxx>
#include <comphelper/getexpandeduri.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/util/thePathSettings.hpp>
#include <com/sun/star/util/PathSubstitution.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>
#include <rtl/instance.hxx>
#include <o3tl/enumarray.hxx>

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

typedef std::unordered_map<OUString, sal_Int32> NameToHandleMap;

typedef std::set<OUString> VarNameSet;

// class SvtPathOptions_Impl ---------------------------------------------
class SvtPathOptions_Impl
{
    private:
        // Local variables to return const references
        o3tl::enumarray< SvtPath, OUString > m_aPathArray;
        Reference< XFastPropertySet >       m_xPathSettings;
        Reference< XStringSubstitution >    m_xSubstVariables;
        Reference< XMacroExpander >         m_xMacroExpander;
        mutable std::unordered_map<SvtPath, sal_Int32>
                                            m_aMapEnumToPropHandle;
        VarNameSet                          m_aSystemPathVarNames;

        OUString                            m_aEmptyString;
        mutable ::osl::Mutex                m_aMutex;

    public:
                        SvtPathOptions_Impl();

        // get the paths, not const because of using a mutex
        const OUString& GetPath( SvtPath );
        const OUString& GetAddinPath() { return GetPath( SvtPath::AddIn ); }
        const OUString& GetAutoCorrectPath() { return GetPath( SvtPath::AutoCorrect ); }
        const OUString& GetAutoTextPath() { return GetPath( SvtPath::AutoText ); }
        const OUString& GetBackupPath() { return GetPath( SvtPath::Backup ); }
        const OUString& GetBasicPath() { return GetPath( SvtPath::Basic ); }
        const OUString& GetBitmapPath() { return GetPath( SvtPath::Bitmap ); }
        const OUString& GetConfigPath() { return GetPath( SvtPath::Config ); }
        const OUString& GetDictionaryPath() { return GetPath( SvtPath::Dictionary ); }
        const OUString& GetFavoritesPath() { return GetPath( SvtPath::Favorites ); }
        const OUString& GetFilterPath() { return GetPath( SvtPath::Filter ); }
        const OUString& GetGalleryPath() { return GetPath( SvtPath::Gallery ); }
        const OUString& GetGraphicPath() { return GetPath( SvtPath::Graphic ); }
        const OUString& GetHelpPath() { return GetPath( SvtPath::Help ); }
        const OUString& GetLinguisticPath() { return GetPath( SvtPath::Linguistic ); }
        const OUString& GetModulePath() { return GetPath( SvtPath::Module ); }
        const OUString& GetPalettePath() { return GetPath( SvtPath::Palette ); }
        const OUString& GetIconsetPath() { return GetPath( SvtPath::IconSet); }
        const OUString& GetPluginPath() { return GetPath( SvtPath::Plugin ); }
        const OUString& GetStoragePath() { return GetPath( SvtPath::Storage ); }
        const OUString& GetTempPath() { return GetPath( SvtPath::Temp ); }
        const OUString& GetTemplatePath() { return GetPath( SvtPath::Template ); }
        const OUString& GetUserConfigPath() { return GetPath( SvtPath::UserConfig ); }
        const OUString& GetWorkPath() { return GetPath( SvtPath::Work ); }
        const OUString& GetUIConfigPath() { return GetPath( SvtPath::UIConfig ); }
        const OUString& GetFingerprintPath() { return GetPath( SvtPath::Fingerprint ); }
        const OUString& GetNumbertextPath() { return GetPath( SvtPath::NumberText ); }
        const OUString& GetClassificationPath() { return GetPath( SvtPath::Classification ); }

        // set the paths
        void            SetPath( SvtPath, const OUString& rNewPath );
        void            SetAddinPath( const OUString& rPath ) { SetPath( SvtPath::AddIn, rPath ); }
        void            SetAutoCorrectPath( const OUString& rPath ) { SetPath( SvtPath::AutoCorrect, rPath ); }
        void            SetAutoTextPath( const OUString& rPath ) { SetPath( SvtPath::AutoText, rPath ); }
        void            SetBackupPath( const OUString& rPath ) { SetPath( SvtPath::Backup, rPath ); }
        void            SetBasicPath( const OUString& rPath ) { SetPath( SvtPath::Basic, rPath ); }
        void            SetBitmapPath( const OUString& rPath ) { SetPath( SvtPath::Bitmap, rPath ); }
        void            SetConfigPath( const OUString& rPath ) { SetPath( SvtPath::Config, rPath ); }
        void            SetDictionaryPath( const OUString& rPath ) { SetPath( SvtPath::Dictionary, rPath ); }
        void            SetFavoritesPath( const OUString& rPath ) { SetPath( SvtPath::Favorites, rPath ); }
        void            SetFilterPath( const OUString& rPath ) { SetPath( SvtPath::Filter, rPath ); }
        void            SetGalleryPath( const OUString& rPath ) { SetPath( SvtPath::Gallery, rPath ); }
        void            SetGraphicPath( const OUString& rPath ) { SetPath( SvtPath::Graphic, rPath ); }
        void            SetHelpPath( const OUString& rPath ) { SetPath( SvtPath::Help, rPath ); }
        void            SetLinguisticPath( const OUString& rPath ) { SetPath( SvtPath::Linguistic, rPath ); }
        void            SetModulePath( const OUString& rPath ) { SetPath( SvtPath::Module, rPath ); }
        void            SetPalettePath( const OUString& rPath ) { SetPath( SvtPath::Palette, rPath ); }
        void            SetPluginPath( const OUString& rPath ) { SetPath( SvtPath::Plugin, rPath ); }
        void            SetStoragePath( const OUString& rPath ) { SetPath( SvtPath::Storage, rPath ); }
        void            SetTempPath( const OUString& rPath ) { SetPath( SvtPath::Temp, rPath ); }
        void            SetTemplatePath( const OUString& rPath ) { SetPath( SvtPath::Template, rPath ); }
        void            SetUserConfigPath( const OUString& rPath ) { SetPath( SvtPath::UserConfig, rPath ); }
        void            SetWorkPath( const OUString& rPath ) { SetPath( SvtPath::Work, rPath ); }

        OUString   SubstVar( const OUString& rVar ) const;
        OUString   ExpandMacros( const OUString& rPath ) const;
        OUString   UsePathVariables( const OUString& rPath ) const;
};

// global ----------------------------------------------------------------

static std::weak_ptr<SvtPathOptions_Impl> g_pOptions;

namespace {

// functions -------------------------------------------------------------
struct PropertyStruct
{
    const char*             pPropName;  // The ascii name of the Office path
    SvtPath                 ePath;      // The enum value used by SvtPathOptions
};

struct VarNameAttribute
{
    const char*             pVarName;       // The name of the path variable
};

}

const PropertyStruct aPropNames[] =
{
    { "Addin",          SvtPath::AddIn          },
    { "AutoCorrect",    SvtPath::AutoCorrect    },
    { "AutoText",       SvtPath::AutoText       },
    { "Backup",         SvtPath::Backup         },
    { "Basic",          SvtPath::Basic          },
    { "Bitmap",         SvtPath::Bitmap         },
    { "Config",         SvtPath::Config         },
    { "Dictionary",     SvtPath::Dictionary     },
    { "Favorite",       SvtPath::Favorites      },
    { "Filter",         SvtPath::Filter         },
    { "Gallery",        SvtPath::Gallery        },
    { "Graphic",        SvtPath::Graphic        },
    { "Help",           SvtPath::Help           },
    { "Iconset",        SvtPath::IconSet        },
    { "Linguistic",     SvtPath::Linguistic     },
    { "Module",         SvtPath::Module         },
    { "Palette",        SvtPath::Palette        },
    { "Plugin",         SvtPath::Plugin         },
    { "Storage",        SvtPath::Storage        },
    { "Temp",           SvtPath::Temp           },
    { "Template",       SvtPath::Template       },
    { "UserConfig",     SvtPath::UserConfig     },
    { "Work",           SvtPath::Work           },
    { "UIConfig",       SvtPath::UIConfig       },
    { "Fingerprint",    SvtPath::Fingerprint    },
    { "Numbertext",     SvtPath::NumberText     },
    { "Classification", SvtPath::Classification }
};

const VarNameAttribute aVarNameAttribute[] =
{
    { SUBSTITUTE_INSTPATH },    // $(instpath)
    { SUBSTITUTE_PROGPATH },    // $(progpath)
    { SUBSTITUTE_USERPATH },    // $(userpath)
    { SUBSTITUTE_PATH },    // $(path)
};

// class SvtPathOptions_Impl ---------------------------------------------

const OUString& SvtPathOptions_Impl::GetPath( SvtPath ePath )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    try
    {
        OUString    aPathValue;
        sal_Int32   nHandle = m_aMapEnumToPropHandle[ePath];

        // Substitution is done by the service itself using the substitution service
        Any         a = m_xPathSettings->getFastPropertyValue( nHandle );
        a >>= aPathValue;
        if( ePath == SvtPath::AddIn     ||
            ePath == SvtPath::Filter    ||
            ePath == SvtPath::Help      ||
            ePath == SvtPath::Module    ||
            ePath == SvtPath::Plugin    ||
            ePath == SvtPath::Storage
          )
        {
            // These office paths have to be converted to system pates
            OUString    aResult;
            osl::FileBase::getSystemPathFromFileURL( aPathValue, aResult );
            aPathValue = aResult;
        }
        else if (ePath == SvtPath::Palette ||
                 ePath == SvtPath::IconSet)
        {
            auto ctx = comphelper::getProcessComponentContext();
            OUStringBuffer buf(aPathValue.getLength()*2);
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

void SvtPathOptions_Impl::SetPath( SvtPath ePath, const OUString& rNewPath )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString    aResult;
    OUString    aNewValue;
    Any         a;

    switch ( ePath )
    {
        case SvtPath::AddIn:
        case SvtPath::Filter:
        case SvtPath::Help:
        case SvtPath::Module:
        case SvtPath::Plugin:
        case SvtPath::Storage:
        {
            // These office paths have to be convert back to UCB-URL's
            osl::FileBase::getFileURLFromSystemPath( rNewPath, aResult );
            aNewValue = aResult;
        }
        break;

        default:
            aNewValue = rNewPath;
    }

    // Resubstitution is done by the service itself using the substitution service
    a <<= aNewValue;
    try
    {
        m_xPathSettings->setFastPropertyValue( m_aMapEnumToPropHandle[ePath], a );
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION("unotools.config", "SetPath");
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

SvtPathOptions_Impl::SvtPathOptions_Impl()
{
    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();

    // Create necessary services
    Reference< XPathSettings > xPathSettings = thePathSettings::get(xContext);
    m_xPathSettings.set( xPathSettings, UNO_QUERY_THROW );
    m_xSubstVariables.set( PathSubstitution::create(xContext) );
    m_xMacroExpander = theMacroExpander::get(xContext);

    // Create temporary hash map to have a mapping between property names and property handles
    Reference< XPropertySetInfo > xPropSetInfo = xPathSettings->getPropertySetInfo();
    const Sequence< Property > aPathPropSeq = xPropSetInfo->getProperties();

    NameToHandleMap aTempHashMap;
    for ( const css::beans::Property& aProperty : aPathPropSeq )
    {
        aTempHashMap.emplace(aProperty.Name, aProperty.Handle);
    }

    // Create mapping between internal enum (SvtPathOptions::Paths) and property handle
    for ( auto const & p : aPropNames )
    {
        NameToHandleMap::const_iterator pIter =
            aTempHashMap.find( OUString::createFromAscii( p.pPropName ));

        if ( pIter != aTempHashMap.end() )
        {
            sal_Int32 nHandle   = pIter->second;
            SvtPath nEnum       = p.ePath;
            m_aMapEnumToPropHandle.emplace( nEnum, nHandle );
        }
    }

    // Create hash map for path variables that need a system path as a return value!
    for ( auto const & i : aVarNameAttribute )
    {
        m_aSystemPathVarNames.insert( OUString::createFromAscii( i.pVarName ) );
    }
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

const OUString& SvtPathOptions::GetNumbertextPath() const
{
    return pImpl->GetNumbertextPath();
}

const OUString& SvtPathOptions::GetModulePath() const
{
    return pImpl->GetModulePath();
}

const OUString& SvtPathOptions::GetPalettePath() const
{
    return pImpl->GetPalettePath();
}

const OUString& SvtPathOptions::GetIconsetPath() const
{
    return pImpl->GetIconsetPath();
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

bool SvtPathOptions::SearchFile( OUString& rIniFile, SvtPath ePath )
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
        case SvtPath::UserConfig:
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
                case SvtPath::AddIn:        aPath = GetAddinPath();         break;
                case SvtPath::AutoCorrect:  aPath = GetAutoCorrectPath();   break;
                case SvtPath::AutoText:     aPath = GetAutoTextPath();      break;
                case SvtPath::Backup:       aPath = GetBackupPath();        break;
                case SvtPath::Basic:        aPath = GetBasicPath();         break;
                case SvtPath::Bitmap:       aPath = GetBitmapPath();        break;
                case SvtPath::Config:       aPath = GetConfigPath();        break;
                case SvtPath::Dictionary:   aPath = GetDictionaryPath();    break;
                case SvtPath::Favorites:    aPath = GetFavoritesPath();     break;
                case SvtPath::Filter:       aPath = GetFilterPath();        break;
                case SvtPath::Gallery:      aPath = GetGalleryPath();       break;
                case SvtPath::Graphic:      aPath = GetGraphicPath();       break;
                case SvtPath::Help:         aPath = GetHelpPath();          break;
                case SvtPath::Linguistic:   aPath = GetLinguisticPath();    break;
                case SvtPath::Module:       aPath = GetModulePath();        break;
                case SvtPath::Palette:      aPath = GetPalettePath();       break;
                case SvtPath::IconSet:      aPath = GetIconsetPath();       break;
                case SvtPath::Plugin:       aPath = GetPluginPath();        break;
                case SvtPath::Storage:      aPath = GetStoragePath();       break;
                case SvtPath::Temp:         aPath = GetTempPath();          break;
                case SvtPath::Template:     aPath = GetTemplatePath();      break;
                case SvtPath::Work:         aPath = GetWorkPath();          break;
                case SvtPath::UIConfig:     aPath = pImpl->GetUIConfigPath(); break;
                case SvtPath::Fingerprint:  aPath = GetFingerprintPath();   break;
                case SvtPath::NumberText:   aPath = GetNumbertextPath();    break;
                case SvtPath::Classification: aPath = GetClassificationPath(); break;
                // coverity[dead_error_begin] - following conditions exist to avoid compiler warning
                case SvtPath::UserConfig:
                case SvtPath::LAST:
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
