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
#include <comphelper/diagnose_ex.hxx>
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
#include <o3tl/enumarray.hxx>
#include <o3tl/string_view.hxx>

#include "itemholder1.hxx"

#include <set>
#include <unordered_map>

using namespace osl;
using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;

#define SEARCHPATH_DELIMITER  ';'
#define SIGN_STARTVARIABLE    "$("
#define SIGN_ENDVARIABLE      ")"

#define STRPOS_NOTFOUND       -1

typedef std::unordered_map<OUString, sal_Int32> NameToHandleMap;

typedef std::set<OUString> VarNameSet;

// class SvtPathOptions_Impl ---------------------------------------------
class SvtPathOptions_Impl
{
    private:
        // Local variables to return const references
        o3tl::enumarray< SvtPathOptions::Paths, OUString > m_aPathArray;
        Reference< XFastPropertySet >       m_xPathSettings;
        Reference< XStringSubstitution >    m_xSubstVariables;
        Reference< XMacroExpander >         m_xMacroExpander;
        mutable std::unordered_map<SvtPathOptions::Paths, sal_Int32>
                                            m_aMapEnumToPropHandle;
        VarNameSet                          m_aSystemPathVarNames;

        OUString                            m_aEmptyString;
        mutable std::mutex                  m_aMutex;

    public:
                        SvtPathOptions_Impl();

        // get the paths, not const because of using a mutex
        const OUString& GetPath( SvtPathOptions::Paths );
        const OUString& GetAddinPath() { return GetPath( SvtPathOptions::Paths::AddIn ); }
        const OUString& GetAutoCorrectPath() { return GetPath( SvtPathOptions::Paths::AutoCorrect ); }
        const OUString& GetAutoTextPath() { return GetPath( SvtPathOptions::Paths::AutoText ); }
        const OUString& GetBackupPath() { return GetPath( SvtPathOptions::Paths::Backup ); }
        const OUString& GetBasicPath() { return GetPath( SvtPathOptions::Paths::Basic ); }
        const OUString& GetBitmapPath() { return GetPath( SvtPathOptions::Paths::Bitmap ); }
        const OUString& GetConfigPath() { return GetPath( SvtPathOptions::Paths::Config ); }
        const OUString& GetDictionaryPath() { return GetPath( SvtPathOptions::Paths::Dictionary ); }
        const OUString& GetFavoritesPath() { return GetPath( SvtPathOptions::Paths::Favorites ); }
        const OUString& GetFilterPath() { return GetPath( SvtPathOptions::Paths::Filter ); }
        const OUString& GetGalleryPath() { return GetPath( SvtPathOptions::Paths::Gallery ); }
        const OUString& GetGraphicPath() { return GetPath( SvtPathOptions::Paths::Graphic ); }
        const OUString& GetHelpPath() { return GetPath( SvtPathOptions::Paths::Help ); }
        const OUString& GetLinguisticPath() { return GetPath( SvtPathOptions::Paths::Linguistic ); }
        const OUString& GetModulePath() { return GetPath( SvtPathOptions::Paths::Module ); }
        const OUString& GetPalettePath() { return GetPath( SvtPathOptions::Paths::Palette ); }
        const OUString& GetIconsetPath() { return GetPath( SvtPathOptions::Paths::IconSet); }
        const OUString& GetPluginPath() { return GetPath( SvtPathOptions::Paths::Plugin ); }
        const OUString& GetStoragePath() { return GetPath( SvtPathOptions::Paths::Storage ); }
        const OUString& GetTempPath() { return GetPath( SvtPathOptions::Paths::Temp ); }
        const OUString& GetTemplatePath() { return GetPath( SvtPathOptions::Paths::Template ); }
        const OUString& GetUserConfigPath() { return GetPath( SvtPathOptions::Paths::UserConfig ); }
        const OUString& GetWorkPath() { return GetPath( SvtPathOptions::Paths::Work ); }
        const OUString& GetUIConfigPath() { return GetPath( SvtPathOptions::Paths::UIConfig ); }
        const OUString& GetFingerprintPath() { return GetPath( SvtPathOptions::Paths::Fingerprint ); }
        const OUString& GetNumbertextPath() { return GetPath( SvtPathOptions::Paths::NumberText ); }
        const OUString& GetClassificationPath() { return GetPath( SvtPathOptions::Paths::Classification ); }

        // set the paths
        void            SetPath( SvtPathOptions::Paths, const OUString& rNewPath );
        void            SetAutoTextPath( const OUString& rPath ) { SetPath( SvtPathOptions::Paths::AutoText, rPath ); }
        void            SetBasicPath( const OUString& rPath ) { SetPath( SvtPathOptions::Paths::Basic, rPath ); }
        void            SetTempPath( const OUString& rPath ) { SetPath( SvtPathOptions::Paths::Temp, rPath ); }
        void            SetWorkPath( const OUString& rPath ) { SetPath( SvtPathOptions::Paths::Work, rPath ); }

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
    OUString                aPropName;  // The ascii name of the Office path
    SvtPathOptions::Paths   ePath;      // The enum value used by SvtPathOptions
};

struct VarNameAttribute
{
    OUString aVarName;       // The name of the path variable
};

}

constexpr PropertyStruct aPropNames[] =
{
    { u"Addin"_ustr,          SvtPathOptions::Paths::AddIn          },
    { u"AutoCorrect"_ustr,    SvtPathOptions::Paths::AutoCorrect    },
    { u"AutoText"_ustr,       SvtPathOptions::Paths::AutoText       },
    { u"Backup"_ustr,         SvtPathOptions::Paths::Backup         },
    { u"Basic"_ustr,          SvtPathOptions::Paths::Basic          },
    { u"Bitmap"_ustr,         SvtPathOptions::Paths::Bitmap         },
    { u"Config"_ustr,         SvtPathOptions::Paths::Config         },
    { u"Dictionary"_ustr,     SvtPathOptions::Paths::Dictionary     },
    { u"Favorite"_ustr,       SvtPathOptions::Paths::Favorites      },
    { u"Filter"_ustr,         SvtPathOptions::Paths::Filter         },
    { u"Gallery"_ustr,        SvtPathOptions::Paths::Gallery        },
    { u"Graphic"_ustr,        SvtPathOptions::Paths::Graphic        },
    { u"Help"_ustr,           SvtPathOptions::Paths::Help           },
    { u"Iconset"_ustr,        SvtPathOptions::Paths::IconSet        },
    { u"Linguistic"_ustr,     SvtPathOptions::Paths::Linguistic     },
    { u"Module"_ustr,         SvtPathOptions::Paths::Module         },
    { u"Palette"_ustr,        SvtPathOptions::Paths::Palette        },
    { u"Plugin"_ustr,         SvtPathOptions::Paths::Plugin         },
    { u"Storage"_ustr,        SvtPathOptions::Paths::Storage        },
    { u"Temp"_ustr,           SvtPathOptions::Paths::Temp           },
    { u"Template"_ustr,       SvtPathOptions::Paths::Template       },
    { u"UserConfig"_ustr,     SvtPathOptions::Paths::UserConfig     },
    { u"Work"_ustr,           SvtPathOptions::Paths::Work           },
    { u"UIConfig"_ustr,       SvtPathOptions::Paths::UIConfig       },
    { u"Fingerprint"_ustr,    SvtPathOptions::Paths::Fingerprint    },
    { u"Numbertext"_ustr,     SvtPathOptions::Paths::NumberText     },
    { u"Classification"_ustr, SvtPathOptions::Paths::Classification }
};

// Supported variables by the old SvtPathOptions implementation
constexpr VarNameAttribute aVarNameAttribute[] =
{
    { u"$(instpath)"_ustr },    // $(instpath)
    { u"$(progpath)"_ustr },    // $(progpath)
    { u"$(userpath)"_ustr },    // $(userpath)
    { u"$(path)"_ustr },    // $(path)
};

// class SvtPathOptions_Impl ---------------------------------------------

const OUString& SvtPathOptions_Impl::GetPath( SvtPathOptions::Paths ePath )
{
    std::unique_lock aGuard( m_aMutex );

    try
    {
        OUString    aPathValue;
        sal_Int32   nHandle = m_aMapEnumToPropHandle[ePath];

        // Substitution is done by the service itself using the substitution service
        Any         a = m_xPathSettings->getFastPropertyValue( nHandle );
        a >>= aPathValue;
        if( ePath == SvtPathOptions::Paths::AddIn     ||
            ePath == SvtPathOptions::Paths::Filter    ||
            ePath == SvtPathOptions::Paths::Help      ||
            ePath == SvtPathOptions::Paths::Module    ||
            ePath == SvtPathOptions::Paths::Plugin    ||
            ePath == SvtPathOptions::Paths::Storage
          )
        {
            // These office paths have to be converted to system pates
            OUString    aResult;
            osl::FileBase::getSystemPathFromFileURL( aPathValue, aResult );
            aPathValue = aResult;
        }
        else if (ePath == SvtPathOptions::Paths::Palette ||
                 ePath == SvtPathOptions::Paths::IconSet)
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

void SvtPathOptions_Impl::SetPath( SvtPathOptions::Paths ePath, const OUString& rNewPath )
{
    std::unique_lock aGuard( m_aMutex );

    OUString    aResult;
    OUString    aNewValue;
    Any         a;

    switch ( ePath )
    {
        case SvtPathOptions::Paths::AddIn:
        case SvtPathOptions::Paths::Filter:
        case SvtPathOptions::Paths::Help:
        case SvtPathOptions::Paths::Module:
        case SvtPathOptions::Paths::Plugin:
        case SvtPathOptions::Paths::Storage:
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
        NameToHandleMap::const_iterator pIter = aTempHashMap.find( p.aPropName );

        if ( pIter != aTempHashMap.end() )
        {
            sal_Int32 nHandle   = pIter->second;
            SvtPathOptions::Paths nEnum       = p.ePath;
            m_aMapEnumToPropHandle.emplace( nEnum, nHandle );
        }
    }

    // Create hash map for path variables that need a system path as a return value!
    for ( auto const & i : aVarNameAttribute )
    {
        m_aSystemPathVarNames.insert( i.aVarName );
    }
}

// class SvtPathOptions --------------------------------------------------

namespace
{
    std::mutex& lclMutex()
    {
        static std::mutex SINGLETON;
        return SINGLETON;
    }
}

SvtPathOptions::SvtPathOptions()
{
    // Global access, must be guarded (multithreading)
    std::unique_lock aGuard( lclMutex() );
    pImpl = g_pOptions.lock();
    if ( !pImpl )
    {
        pImpl = std::make_shared<SvtPathOptions_Impl>();
        g_pOptions = pImpl;
        aGuard.unlock(); // because holdConfigItem will call this constructor
        ItemHolder1::holdConfigItem(EItem::PathOptions);
    }
}

SvtPathOptions::~SvtPathOptions()
{
    // Global access, must be guarded (multithreading)
    std::unique_lock aGuard( lclMutex() );

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

void SvtPathOptions::SetAutoTextPath( const OUString& rPath )
{
    pImpl->SetAutoTextPath( rPath );
}

void SvtPathOptions::SetBasicPath( const OUString& rPath )
{
    pImpl->SetBasicPath( rPath );
}

void SvtPathOptions::SetTempPath( const OUString& rPath )
{
    pImpl->SetTempPath( rPath );
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

bool SvtPathOptions::SearchFile( OUString& rIniFile, SvtPathOptions::Paths ePath )
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
        case SvtPathOptions::Paths::UserConfig:
        {
            // path is a URL
            bRet = true;
            INetURLObject aObj( GetUserConfigPath() );

            sal_Int32 nIniIndex = 0;
            do
            {
                std::u16string_view aToken = o3tl::getToken(aIniFile, 0, '/', nIniIndex );
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
                case SvtPathOptions::Paths::AddIn:        aPath = GetAddinPath();         break;
                case SvtPathOptions::Paths::AutoCorrect:  aPath = GetAutoCorrectPath();   break;
                case SvtPathOptions::Paths::AutoText:     aPath = GetAutoTextPath();      break;
                case SvtPathOptions::Paths::Backup:       aPath = GetBackupPath();        break;
                case SvtPathOptions::Paths::Basic:        aPath = GetBasicPath();         break;
                case SvtPathOptions::Paths::Bitmap:       aPath = GetBitmapPath();        break;
                case SvtPathOptions::Paths::Config:       aPath = GetConfigPath();        break;
                case SvtPathOptions::Paths::Dictionary:   aPath = GetDictionaryPath();    break;
                case SvtPathOptions::Paths::Favorites:    aPath = GetFavoritesPath();     break;
                case SvtPathOptions::Paths::Filter:       aPath = GetFilterPath();        break;
                case SvtPathOptions::Paths::Gallery:      aPath = GetGalleryPath();       break;
                case SvtPathOptions::Paths::Graphic:      aPath = GetGraphicPath();       break;
                case SvtPathOptions::Paths::Help:         aPath = GetHelpPath();          break;
                case SvtPathOptions::Paths::Linguistic:   aPath = GetLinguisticPath();    break;
                case SvtPathOptions::Paths::Module:       aPath = GetModulePath();        break;
                case SvtPathOptions::Paths::Palette:      aPath = GetPalettePath();       break;
                case SvtPathOptions::Paths::IconSet:      aPath = GetIconsetPath();       break;
                case SvtPathOptions::Paths::Plugin:       aPath = GetPluginPath();        break;
                case SvtPathOptions::Paths::Storage:      aPath = GetStoragePath();       break;
                case SvtPathOptions::Paths::Temp:         aPath = GetTempPath();          break;
                case SvtPathOptions::Paths::Template:     aPath = GetTemplatePath();      break;
                case SvtPathOptions::Paths::Work:         aPath = GetWorkPath();          break;
                case SvtPathOptions::Paths::UIConfig:     aPath = pImpl->GetUIConfigPath(); break;
                case SvtPathOptions::Paths::Fingerprint:  aPath = GetFingerprintPath();   break;
                case SvtPathOptions::Paths::NumberText:   aPath = GetNumbertextPath();    break;
                case SvtPathOptions::Paths::Classification: aPath = GetClassificationPath(); break;
                // coverity[dead_error_begin] - following conditions exist to avoid compiler warning
                case SvtPathOptions::Paths::UserConfig:
                case SvtPathOptions::Paths::LAST:
                    break;
            }

            sal_Int32 nPathIndex = 0;
            do
            {
                bool bIsURL = true;
                OUString aPathToken( aPath.getToken( 0, SEARCHPATH_DELIMITER, nPathIndex ) );
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
                    std::u16string_view aToken = o3tl::getToken(aIniFile, 0, '/', nIniIndex );
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
