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

#include <unotools/pathoptions.hxx>
#include <unotools/configitem.hxx>
#include <unotools/configmgr.hxx>
#include <tools/urlobj.hxx>
#include <tools/solar.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/mutex.hxx>
#include <osl/file.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/bootstrap.hxx>

#include <unotools/ucbhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/PathSettings.hpp>
#include <com/sun/star/util/PathSubstitution.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/util/theMacroExpander.hpp>
#include <rtl/instance.hxx>

#include <itemholder1.hxx>

#include <vector>
#include <boost/unordered_map.hpp>

using namespace osl;
using namespace utl;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;


#define SEARCHPATH_DELIMITER  ';'
#define SIGN_STARTVARIABLE    OUString( "$("  )
#define SIGN_ENDVARIABLE      OUString( ")" )

// Supported variables by the old SvtPathOptions implementation
#define SUBSTITUTE_INSTPATH   "$(instpath)"
#define SUBSTITUTE_PROGPATH   "$(progpath)"
#define SUBSTITUTE_USERPATH   "$(userpath)"
#define SUBSTITUTE_PATH       "$(path)"

#define STRPOS_NOTFOUND       -1

struct OUStringHashCode
{
    size_t operator()( const OUString& sString ) const
    {
        return sString.hashCode();
    }
};

enum VarNameProperty
{
    VAR_NEEDS_SYSTEM_PATH,
    VAR_NEEDS_FILEURL
};

class NameToHandleMap : public ::boost::unordered_map<  OUString, sal_Int32, OUStringHashCode, ::std::equal_to< OUString > >
{
    public:
        inline void free() { NameToHandleMap().swap( *this ); }
};

class EnumToHandleMap : public ::boost::unordered_map< sal_Int32, sal_Int32, boost::hash< sal_Int32 >, std::equal_to< sal_Int32 > >
{
    public:
        inline void free() { EnumToHandleMap().swap( *this ); }
};

class VarNameToEnumMap : public ::boost::unordered_map< OUString, VarNameProperty, OUStringHashCode, ::std::equal_to< OUString > >
{
    public:
        inline void free() { VarNameToEnumMap().swap( *this ); }
};


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
        VarNameToEnumMap                    m_aMapVarNamesToEnum;

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

static SvtPathOptions_Impl* pOptions = NULL;
static sal_Int32 nRefCount = 0;

// functions -------------------------------------------------------------
struct PropertyStruct
{
    const char*             pPropName;  // The ascii name of the Office path
    SvtPathOptions::Paths   ePath;      // The enum value used by SvtPathOptions
};

struct VarNameAttribute
{
    const char*             pVarName;       // The name of the path variable
    VarNameProperty         eVarProperty;   // Which return value is needed by this path variable
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
    { "Fingerprint",    SvtPathOptions::PATH_FINGERPRINT    }
};

static const VarNameAttribute aVarNameAttribute[] =
{
    { SUBSTITUTE_INSTPATH,  VAR_NEEDS_SYSTEM_PATH },    // $(instpath)
    { SUBSTITUTE_PROGPATH,  VAR_NEEDS_SYSTEM_PATH },    // $(progpath)
    { SUBSTITUTE_USERPATH,  VAR_NEEDS_SYSTEM_PATH },    // $(userpath)
    { SUBSTITUTE_PATH,      VAR_NEEDS_SYSTEM_PATH },    // $(path)
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
            utl::LocalFileHelper::ConvertURLToPhysicalName( aPathValue, aResult );
            aPathValue = aResult;
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
                utl::LocalFileHelper::ConvertPhysicalNameToURL( rNewPath, aResult );
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
        catch (const Exception&)
        {
        }
    }
}

//-------------------------------------------------------------------------

OUString SvtPathOptions_Impl::ExpandMacros( const OUString& rPath ) const
{
    OUString sExpanded( rPath );

    const INetURLObject aParser( rPath );
    if ( aParser.GetProtocol() == INET_PROT_VND_SUN_STAR_EXPAND )
        sExpanded = m_xMacroExpander->expandMacros( aParser.GetURLPath( INetURLObject::DECODE_WITH_CHARSET ) );

    return sExpanded;
}

//-------------------------------------------------------------------------

OUString SvtPathOptions_Impl::UsePathVariables( const OUString& rPath ) const
{
    return m_xSubstVariables->reSubstituteVariables( rPath );
}

// -----------------------------------------------------------------------

OUString SvtPathOptions_Impl::SubstVar( const OUString& rVar ) const
{
    // Don't work at parameter-string directly. Copy it.
    OUString aWorkText = rVar;

    // Convert the returned path to system path!
    bool bConvertLocal = false;

    // Search for first occure of "$(...".
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
        VarNameToEnumMap::const_iterator pIter = m_aMapVarNamesToEnum.find( aSubString );
        if ( pIter != m_aMapVarNamesToEnum.end() )
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
        utl::LocalFileHelper::ConvertURLToPhysicalName( aWorkText, aReturn );
        return aReturn;
    }

    return aWorkText;
}

// -----------------------------------------------------------------------

SvtPathOptions_Impl::SvtPathOptions_Impl() :
    m_aPathArray( (sal_Int32)SvtPathOptions::PATH_COUNT ),
    m_aLanguageTag( LANGUAGE_DONTKNOW )
{
    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();

    // Create necessary services
    Reference< XPathSettings > xPathSettings = PathSettings::create(xContext);
    m_xPathSettings.set( xPathSettings, UNO_QUERY_THROW );
    m_xSubstVariables.set( PathSubstitution::create(xContext) );
    m_xMacroExpander = theMacroExpander::get(xContext);

    // Create temporary hash map to have a mapping between property names and property handles
    Reference< XPropertySetInfo > xPropSetInfo = xPathSettings->getPropertySetInfo();
    Sequence< Property > aPathPropSeq = xPropSetInfo->getProperties();

    NameToHandleMap aTempHashMap;
    for ( sal_Int32 n = 0; n < aPathPropSeq.getLength(); n++ )
    {
        const com::sun::star::beans::Property& aProperty = aPathPropSeq[n];
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
        m_aMapVarNamesToEnum.insert( VarNameToEnumMap::value_type(
                OUString::createFromAscii( aVarNameAttribute[i].pVarName ),
                aVarNameAttribute[i].eVarProperty ));
    }

    // Set language type!
    m_aLanguageTag.reset( ConfigManager::getLocale() );
}

// -----------------------------------------------------------------------

// class SvtPathOptions --------------------------------------------------

namespace { struct lclMutex : public rtl::Static< ::osl::Mutex, lclMutex > {}; }

SvtPathOptions::SvtPathOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( lclMutex::get() );
    if ( !pOptions )
    {
        pOptions = new SvtPathOptions_Impl;
        ItemHolder1::holdConfigItem(E_PATHOPTIONS);
    }
    ++nRefCount;
    pImp = pOptions;
}

// -----------------------------------------------------------------------

SvtPathOptions::~SvtPathOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( lclMutex::get() );
    if ( !--nRefCount )
    {
        DELETEZ( pOptions );
    }
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetAddinPath() const
{
    return pImp->GetAddinPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetAutoCorrectPath() const
{
    return pImp->GetAutoCorrectPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetAutoTextPath() const
{
    return pImp->GetAutoTextPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetBackupPath() const
{
    return pImp->GetBackupPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetBasicPath() const
{
    return pImp->GetBasicPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetBitmapPath() const
{
    return pImp->GetBitmapPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetConfigPath() const
{
    return pImp->GetConfigPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetDictionaryPath() const
{
    return pImp->GetDictionaryPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetFavoritesPath() const
{
    return pImp->GetFavoritesPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetFilterPath() const
{
    return pImp->GetFilterPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetGalleryPath() const
{
    return pImp->GetGalleryPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetGraphicPath() const
{
    return pImp->GetGraphicPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetHelpPath() const
{
    return pImp->GetHelpPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetLinguisticPath() const
{
    return pImp->GetLinguisticPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetFingerprintPath() const
{
    return pImp->GetFingerprintPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetModulePath() const
{
    return pImp->GetModulePath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetPalettePath() const
{
    return pImp->GetPalettePath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetPluginPath() const
{
    return pImp->GetPluginPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetStoragePath() const
{
    return pImp->GetStoragePath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetTempPath() const
{
    return pImp->GetTempPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetTemplatePath() const
{
    return pImp->GetTemplatePath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetUserConfigPath() const
{
    return pImp->GetUserConfigPath();
}

const OUString& SvtPathOptions::GetUIConfigPath() const
{
    return pImp->GetUIConfigPath();
}

// -----------------------------------------------------------------------

const OUString& SvtPathOptions::GetWorkPath() const
{
    return pImp->GetWorkPath();
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetAddinPath( const OUString& rPath )
{
    pImp->SetAddinPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetAutoCorrectPath( const OUString& rPath )
{
    pImp->SetAutoCorrectPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetAutoTextPath( const OUString& rPath )
{
    pImp->SetAutoTextPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetBackupPath( const OUString& rPath )
{
    pImp->SetBackupPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetBasicPath( const OUString& rPath )
{
    pImp->SetBasicPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetBitmapPath( const OUString& rPath )
{
    pImp->SetBitmapPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetConfigPath( const OUString& rPath )
{
    pImp->SetConfigPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetDictionaryPath( const OUString& rPath )
{
    pImp->SetDictionaryPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetFavoritesPath( const OUString& rPath )
{
    pImp->SetFavoritesPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetFilterPath( const OUString& rPath )
{
    pImp->SetFilterPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetGalleryPath( const OUString& rPath )
{
    pImp->SetGalleryPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetGraphicPath( const OUString& rPath )
{
    pImp->SetGraphicPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetHelpPath( const OUString& rPath )
{
    pImp->SetHelpPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetLinguisticPath( const OUString& rPath )
{
    pImp->SetLinguisticPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetModulePath( const OUString& rPath )
{
    pImp->SetModulePath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetPalettePath( const OUString& rPath )
{
    pImp->SetPalettePath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetPluginPath( const OUString& rPath )
{
    pImp->SetPluginPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetStoragePath( const OUString& rPath )
{
    pImp->SetStoragePath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetTempPath( const OUString& rPath )
{
    pImp->SetTempPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetTemplatePath( const OUString& rPath )
{
    pImp->SetTemplatePath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetUserConfigPath( const OUString& rPath )
{
    pImp->SetUserConfigPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetWorkPath( const OUString& rPath )
{
    pImp->SetWorkPath( rPath );
}

// -----------------------------------------------------------------------

OUString SvtPathOptions::SubstituteVariable( const OUString& rVar ) const
{
    return pImp->SubstVar( rVar );
}

// -----------------------------------------------------------------------

OUString SvtPathOptions::ExpandMacros( const OUString& rPath ) const
{
    return pImp->ExpandMacros( rPath );
}

// -----------------------------------------------------------------------

OUString SvtPathOptions::UseVariable( const OUString& rPath ) const
{
    return pImp->UsePathVariables( rPath );
}

// -----------------------------------------------------------------------

bool SvtPathOptions::SearchFile( OUString& rIniFile, Paths ePath )
{
    // check parameter: empty inifile name?
    if ( rIniFile.isEmpty() )
    {
        SAL_WARN( "unotools.config", "SvtPathOptions::SearchFile(): invalid parameter" );
        return false;
    }

    OUString aIniFile = pImp->SubstVar( rIniFile );
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

            if ( !::utl::UCBContentHelper::Exists( aObj.GetMainURL( INetURLObject::NO_DECODE ) ) )
            {
                aObj.SetSmartURL( GetConfigPath() );
                aObj.insertName( aIniFile );
                bRet = ::utl::UCBContentHelper::Exists( aObj.GetMainURL( INetURLObject::NO_DECODE ) );
            }

            if ( bRet )
                rIniFile = aObj.GetMainURL( INetURLObject::NO_DECODE );

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
                case PATH_UICONFIG:     aPath = GetUIConfigPath();      break;
                case PATH_FINGERPRINT:  aPath = GetFingerprintPath();   break;
                case PATH_USERCONFIG:/*-Wall???*/           break;
                case PATH_COUNT: /*-Wall???*/ break;
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
                    if ( LocalFileHelper::ConvertPhysicalNameToURL( aPathToken, aURL ) )
                        aObj.SetURL( aURL );
                }
                if ( aObj.GetProtocol() == INET_PROT_VND_SUN_STAR_EXPAND )
                {
                    Reference< XMacroExpander > xMacroExpander = theMacroExpander::get( ::comphelper::getProcessComponentContext() );
                    const OUString sExpandedPath = xMacroExpander->expandMacros( aObj.GetURLPath( INetURLObject::DECODE_WITH_CHARSET ) );
                    aObj.SetURL( sExpandedPath );
                }

                sal_Int32 nIniIndex = 0;
                do
                {
                    OUString aToken = aIniFile.getToken( 0, '/', nIniIndex );
                    aObj.insertName(aToken);
                }
                while ( nIniIndex >= 0 );

                bRet = ::utl::UCBContentHelper::Exists( aObj.GetMainURL( INetURLObject::NO_DECODE ) );

                if ( bRet )
                {
                    if ( !bIsURL )
                    {
                        OUString sTmp(rIniFile);
                        ::utl::LocalFileHelper::ConvertURLToPhysicalName(
                                            aObj.GetMainURL( INetURLObject::NO_DECODE ), sTmp );
                        rIniFile = sTmp;
                    }
                    else
                        rIniFile = aObj.GetMainURL( INetURLObject::NO_DECODE );
                    break;
                }
            }
            while ( nPathIndex >= 0 );
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------

const LanguageTag& SvtPathOptions::GetLanguageTag() const
{
    return pImp->GetLanguageTag();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
