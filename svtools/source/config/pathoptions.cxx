/*************************************************************************
 *
 *  $RCSfile: pathoptions.cxx,v $
 *
 *  $Revision: 1.62 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 14:37:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#include "pathoptions.hxx"

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif

#include <unotools/ucbhelper.hxx>
#include <vos/process.hxx>
#include <comphelper/processfactory.hxx>

#ifndef _COM_SUN_STAR_BEANS_XFASTPROPERTYSET_HPP_
#include <com/sun/star/beans/XFastPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSTRINGSUBSTITUTION_HPP_
#include <com/sun/star/util/XStringSubstitution.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#include <vector>
#include <hash_map>

using namespace osl;
using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;

// define ----------------------------------------------------------------

#define SEARCHPATH_DELIMITER            ';'

#define ASCII_STR(s)                    OUString( RTL_CONSTASCII_USTRINGPARAM(s) )

#define SIGN_STARTVARIABLE              ASCII_STR("$(")
#define SIGN_ENDVARIABLE                ASCII_STR(")")

// Supported variables by the old SvtPathOptions implementation
#define SUBSTITUTE_INST                 "$(inst)"
#define SUBSTITUTE_PROG                 "$(prog)"
#define SUBSTITUTE_USER                 "$(user)"
#define SUBSTITUTE_INSTPATH             "$(instpath)"
#define SUBSTITUTE_PROGPATH             "$(progpath)"
#define SUBSTITUTE_USERPATH             "$(userpath)"
#define SUBSTITUTE_INSTURL              "$(insturl)"
#define SUBSTITUTE_PROGURL              "$(progurl)"
#define SUBSTITUTE_USERURL              "$(userurl)"
#define SUBSTITUTE_PATH                 "$(path)"
#define SUBSTITUTE_LANG                 "$(lang)"
#define SUBSTITUTE_LANGID               "$(langid)"
#define SUBSTITUTE_VLANG                "$(vlang)"
#define SUBSTITUTE_WORKDIRURL           "$(workdirurl)"

#define STRPOS_NOTFOUND                 -1

struct OUStringHashCode
{
    size_t operator()( const ::rtl::OUString& sString ) const
    {
        return sString.hashCode();
    }
};

enum VarNameProperty
{
    VAR_NEEDS_SYSTEM_PATH,
    VAR_NEEDS_FILEURL
};

class NameToHandleMap : public ::std::hash_map< ::rtl::OUString, sal_Int32, OUStringHashCode, ::std::equal_to< ::rtl::OUString > >
{
    public:
        inline void free() { NameToHandleMap().swap( *this ); }
};

class EnumToHandleMap : public ::std::hash_map< sal_Int32, sal_Int32, std::hash< sal_Int32 >, std::equal_to< sal_Int32 > >
{
    public:
        inline void free() { EnumToHandleMap().swap( *this ); }
};

class VarNameToEnumMap : public ::std::hash_map< OUString, VarNameProperty, OUStringHashCode, ::std::equal_to< OUString > >
{
    public:
        inline void free() { VarNameToEnumMap().swap( *this ); }
};


// class SvtPathOptions_Impl ---------------------------------------------
class SvtPathOptions_Impl
{
    private:
        // Local variables to return const references
        std::vector< String >               m_aPathArray;
        Reference< XFastPropertySet >       m_xPathSettings;
        Reference< XStringSubstitution >    m_xSubstVariables;
        mutable EnumToHandleMap             m_aMapEnumToPropHandle;
        VarNameToEnumMap                    m_aMapVarNamesToEnum;

        LanguageType                        m_eLanguageType;
        String                              m_aEmptyString;
        mutable ::osl::Mutex                m_aMutex;

        // not const because of using a mutex
        const String&   GetPath( SvtPathOptions::Pathes );
        void            SetPath( SvtPathOptions::Pathes, const String& rNewPath );

    public:
                        SvtPathOptions_Impl();

        // get the pathes, not const because of using a mutex
        const String&   GetAddinPath() { return GetPath( SvtPathOptions::PATH_ADDIN ); }
        const String&   GetAutoCorrectPath() { return GetPath( SvtPathOptions::PATH_AUTOCORRECT ); }
        const String&   GetAutoTextPath() { return GetPath( SvtPathOptions::PATH_AUTOTEXT ); }
        const String&   GetBackupPath() { return GetPath( SvtPathOptions::PATH_BACKUP ); }
        const String&   GetBasicPath() { return GetPath( SvtPathOptions::PATH_BASIC ); }
        const String&   GetBitmapPath() { return GetPath( SvtPathOptions::PATH_BITMAP ); }
        const String&   GetConfigPath() { return GetPath( SvtPathOptions::PATH_CONFIG ); }
        const String&   GetDictionaryPath() { return GetPath( SvtPathOptions::PATH_DICTIONARY ); }
        const String&   GetFavoritesPath() { return GetPath( SvtPathOptions::PATH_FAVORITES ); }
        const String&   GetFilterPath() { return GetPath( SvtPathOptions::PATH_FILTER ); }
        const String&   GetGalleryPath() { return GetPath( SvtPathOptions::PATH_GALLERY ); }
        const String&   GetGraphicPath() { return GetPath( SvtPathOptions::PATH_GRAPHIC ); }
        const String&   GetHelpPath() { return GetPath( SvtPathOptions::PATH_HELP ); }
        const String&   GetLinguisticPath() { return GetPath( SvtPathOptions::PATH_LINGUISTIC ); }
        const String&   GetModulePath() { return GetPath( SvtPathOptions::PATH_MODULE ); }
        const String&   GetPalettePath() { return GetPath( SvtPathOptions::PATH_PALETTE ); }
        const String&   GetPluginPath() { return GetPath( SvtPathOptions::PATH_PLUGIN ); }
        const String&   GetStoragePath() { return GetPath( SvtPathOptions::PATH_STORAGE ); }
        const String&   GetTempPath() { return GetPath( SvtPathOptions::PATH_TEMP ); }
        const String&   GetTemplatePath() { return GetPath( SvtPathOptions::PATH_TEMPLATE ); }
        const String&   GetUserConfigPath() { return GetPath( SvtPathOptions::PATH_USERCONFIG ); }
        const String&   GetUserDictionaryPath() { return GetPath( SvtPathOptions::PATH_USERDICTIONARY ); }
        const String&   GetWorkPath() { return GetPath( SvtPathOptions::PATH_WORK ); }
        const String&   GetUIConfigPath() { return GetPath( SvtPathOptions::PATH_UICONFIG ); }

        // set the pathes
        void            SetAddinPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_ADDIN, rPath ); }
        void            SetAutoCorrectPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_AUTOCORRECT, rPath ); }
        void            SetAutoTextPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_AUTOTEXT, rPath ); }
        void            SetBackupPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_BACKUP, rPath ); }
        void            SetBasicPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_BASIC, rPath ); }
        void            SetBitmapPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_BITMAP, rPath ); }
        void            SetConfigPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_CONFIG, rPath ); }
        void            SetDictionaryPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_DICTIONARY, rPath ); }
        void            SetFavoritesPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_FAVORITES, rPath ); }
        void            SetFilterPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_FILTER, rPath ); }
        void            SetGalleryPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_GALLERY, rPath ); }
        void            SetGraphicPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_GRAPHIC, rPath ); }
        void            SetHelpPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_HELP, rPath ); }
        void            SetLinguisticPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_LINGUISTIC, rPath ); }
        void            SetModulePath( const String& rPath ) { SetPath( SvtPathOptions::PATH_MODULE, rPath ); }
        void            SetPalettePath( const String& rPath ) { SetPath( SvtPathOptions::PATH_PALETTE, rPath ); }
        void            SetPluginPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_PLUGIN, rPath ); }
        void            SetStoragePath( const String& rPath ) { SetPath( SvtPathOptions::PATH_STORAGE, rPath ); }
        void            SetTempPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_TEMP, rPath ); }
        void            SetTemplatePath( const String& rPath ) { SetPath( SvtPathOptions::PATH_TEMPLATE, rPath ); }
        void            SetUserConfigPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_USERCONFIG, rPath ); }
        void            SetUserDictionaryPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_USERDICTIONARY, rPath ); }
        void            SetWorkPath( const String& rPath ) { SetPath( SvtPathOptions::PATH_WORK, rPath ); }

        rtl::OUString   SubstVar( const rtl::OUString& rVar );
        rtl::OUString   SubstituteAndConvert( const rtl::OUString& rPath );
        rtl::OUString   UsePathVariables( const rtl::OUString& rPath );

        LanguageType    GetLanguageType() const { return m_eLanguageType; }

        BOOL            IsPathReadonly(SvtPathOptions::Pathes ePath)const;
};

// global ----------------------------------------------------------------

static SvtPathOptions_Impl* pOptions = NULL;
static sal_Int32            nRefCount = 0;
static ::osl::Mutex aMutex;

// functions -------------------------------------------------------------
struct PropertyStruct
{
    const char*             pPropName;  // The ascii name of the Office path
    SvtPathOptions::Pathes  ePath;      // The enum value used by SvtPathOptions
};

struct VarNameAttribute
{
    const char*             pVarName;       // The name of the path variable
    VarNameProperty         eVarProperty;   // Which return value is needed by this path variable
};

static PropertyStruct aPropNames[] =
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
    { "UserDictionary", SvtPathOptions::PATH_USERDICTIONARY },
    { "Work",           SvtPathOptions::PATH_WORK           },
    { "UIConfig",       SvtPathOptions::PATH_UICONFIG       }
};

static VarNameAttribute aVarNameAttribute[] =
{
    { SUBSTITUTE_INSTPATH,  VAR_NEEDS_SYSTEM_PATH },    // $(instpath)
    { SUBSTITUTE_PROGPATH,  VAR_NEEDS_SYSTEM_PATH },    // $(progpath)
    { SUBSTITUTE_USERPATH,  VAR_NEEDS_SYSTEM_PATH },    // $(userpath)
    { SUBSTITUTE_PATH,      VAR_NEEDS_SYSTEM_PATH },    // $(path)
};

static Sequence< OUString > GetPathPropertyNames()
{
    const int nCount = sizeof( aPropNames ) / sizeof( PropertyStruct );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for ( int i = 0; i < nCount; i++ )
        pNames[i] = OUString::createFromAscii( aPropNames[i].pPropName );

    return aNames;
}

// class SvtPathOptions_Impl ---------------------------------------------

const String& SvtPathOptions_Impl::GetPath( SvtPathOptions::Pathes ePath )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( ePath <= SvtPathOptions::PATH_UICONFIG )
    {
        OUString    aPathValue;
        String      aResult;
        sal_Int32   nHandle = m_aMapEnumToPropHandle[ (sal_Int32)ePath ];

        // Substitution is done by the service itself using the substition service
        Any         a = m_xPathSettings->getFastPropertyValue( nHandle );
        a >>= aPathValue;
        switch ( ePath )
        {
            case SvtPathOptions::PATH_ADDIN:
            case SvtPathOptions::PATH_FILTER:
            case SvtPathOptions::PATH_HELP:
            case SvtPathOptions::PATH_MODULE:
            case SvtPathOptions::PATH_PLUGIN:
            case SvtPathOptions::PATH_STORAGE:
            {
                // These office paths have to be converted to system pathes
                utl::LocalFileHelper::ConvertURLToPhysicalName( aPathValue, aResult );
                aPathValue = aResult;
            }
        }

        m_aPathArray[ ePath ] = aPathValue;
        return m_aPathArray[ ePath ];
    }

    return m_aEmptyString;
}
// -----------------------------------------------------------------------
BOOL SvtPathOptions_Impl::IsPathReadonly(SvtPathOptions::Pathes ePath)const
{
    ::osl::MutexGuard aGuard( m_aMutex );
    BOOL bReadonly = FALSE;
    if ( ePath <= SvtPathOptions::PATH_UICONFIG )
    {
        sal_Int32   nHandle = m_aMapEnumToPropHandle[ (sal_Int32)ePath ];
        Reference<XPropertySet> xPrSet(m_xPathSettings, UNO_QUERY);
        if(xPrSet.is())
        {
            Reference<XPropertySetInfo> xInfo = xPrSet->getPropertySetInfo();
            const char* pA = aPropNames[ePath].pPropName;
            ::rtl::OUString sU = OUString::createFromAscii(pA);
            Property aProperty = xInfo->getPropertyByName(sU);
            bReadonly = 0 != (aProperty.Attributes & PropertyAttribute::READONLY);
        }
    }
    return bReadonly;
}
// -----------------------------------------------------------------------

void SvtPathOptions_Impl::SetPath( SvtPathOptions::Pathes ePath, const String& rNewPath )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( ePath <= SvtPathOptions::PATH_UICONFIG )
    {
        String      aResult;
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

// -----------------------------------------------------------------------

OUString SvtPathOptions_Impl::SubstituteAndConvert( const rtl::OUString& rPath )
{
    return SubstVar( rPath );
}

//-------------------------------------------------------------------------

OUString SvtPathOptions_Impl::UsePathVariables( const OUString& rPath )
{
    return m_xSubstVariables->reSubstituteVariables( rPath );
}

// -----------------------------------------------------------------------

OUString SvtPathOptions_Impl::SubstVar( const OUString& rVar )
{
    // Don't work at parameter-string directly. Copy it.
    OUString aWorkText = rVar;

    // Convert the returned path to system path!
    BOOL bConvertLocal = FALSE;

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
        OUString aReplacement;
        OUString aSubString = aWorkText.copy( nPosition, nLength );
        aSubString = aSubString.toAsciiLowerCase();

        // Look for special variable that needs a system path.
        VarNameToEnumMap::const_iterator pIter = m_aMapVarNamesToEnum.find( aSubString );
        if ( pIter != m_aMapVarNamesToEnum.end() )
            bConvertLocal = TRUE;

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

    aWorkText = m_xSubstVariables->substituteVariables( rVar, sal_False );

    if ( bConvertLocal )
    {
        // Convert the URL to a system path for special path variables
        String aReturn;
        utl::LocalFileHelper::ConvertURLToPhysicalName( aWorkText, aReturn );
        return aReturn;
    }

    return aWorkText;
}

// -----------------------------------------------------------------------

SvtPathOptions_Impl::SvtPathOptions_Impl() :
    m_aPathArray( (sal_Int32)SvtPathOptions::PATH_COUNT )
{
    Reference< XMultiServiceFactory > xSMgr = comphelper::getProcessServiceFactory();

    // Create necessary services
    m_xPathSettings = Reference< XFastPropertySet >( xSMgr->createInstance(
                                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                        "com.sun.star.util.PathSettings" ))),
                                                UNO_QUERY );
    m_xSubstVariables = Reference< XStringSubstitution >( xSMgr->createInstance(
                                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                        "com.sun.star.util.PathSubstitution" ))),
                                                UNO_QUERY );

    // Create temporary hash map to have a mapping between property names and property handles
    Reference< XPropertySet > xPropertySet = Reference< XPropertySet >( m_xPathSettings, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo = xPropertySet->getPropertySetInfo();
    Sequence< Property > aPathPropSeq = xPropSetInfo->getProperties();

    NameToHandleMap aTempHashMap;
    for ( sal_Int32 n = 0; n < aPathPropSeq.getLength(); n++ )
    {
        const com::sun::star::beans::Property& aProperty = aPathPropSeq[n];
        aTempHashMap.insert( NameToHandleMap::value_type( aProperty.Name, aProperty.Handle ));
    }

    // Create mapping between internal enum (SvtPathOptions::Pathes) and property handle
    sal_Int32 nCount = sizeof( aPropNames ) / sizeof( PropertyStruct );
    for ( sal_Int32 i = 0; i < nCount; i++ )
    {
        NameToHandleMap::const_iterator pIter =
            aTempHashMap.find( rtl::OUString::createFromAscii( aPropNames[i].pPropName ));

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
    m_eLanguageType = LANGUAGE_ENGLISH_US;
    Any aLocale = ConfigManager::GetConfigManager()->GetDirectConfigProperty( ConfigManager::LOCALE );
    OUString aLocaleStr;
    if ( aLocale >>= aLocaleStr )
        m_eLanguageType = ConvertIsoStringToLanguage( aLocaleStr );
    else
    {
        DBG_ERRORFILE( "wrong any type" );
    }
}

// -----------------------------------------------------------------------

// class SvtPathOptions --------------------------------------------------

SvtPathOptions::SvtPathOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( aMutex );
    if ( !pOptions )
        pOptions = new SvtPathOptions_Impl;
    ++nRefCount;
    pImp = pOptions;
}

// -----------------------------------------------------------------------

SvtPathOptions::~SvtPathOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( aMutex );
    if ( !--nRefCount )
    {
        DELETEZ( pOptions );
    }
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetAddinPath() const
{
    return pImp->GetAddinPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetAutoCorrectPath() const
{
    return pImp->GetAutoCorrectPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetAutoTextPath() const
{
    return pImp->GetAutoTextPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetBackupPath() const
{
    return pImp->GetBackupPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetBasicPath() const
{
    return pImp->GetBasicPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetBitmapPath() const
{
    return pImp->GetBitmapPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetConfigPath() const
{
    return pImp->GetConfigPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetDictionaryPath() const
{
    return pImp->GetDictionaryPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetFavoritesPath() const
{
    return pImp->GetFavoritesPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetFilterPath() const
{
    return pImp->GetFilterPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetGalleryPath() const
{
    return pImp->GetGalleryPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetGraphicPath() const
{
    return pImp->GetGraphicPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetHelpPath() const
{
    return pImp->GetHelpPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetLinguisticPath() const
{
    return pImp->GetLinguisticPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetModulePath() const
{
    return pImp->GetModulePath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetPalettePath() const
{
    return pImp->GetPalettePath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetPluginPath() const
{
    return pImp->GetPluginPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetStoragePath() const
{
    return pImp->GetStoragePath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetTempPath() const
{
    return pImp->GetTempPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetTemplatePath() const
{
    return pImp->GetTemplatePath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetUserConfigPath() const
{
    return pImp->GetUserConfigPath();
}

const String& SvtPathOptions::GetUIConfigPath() const
{
    return pImp->GetUIConfigPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetUserDictionaryPath() const
{
    return pImp->GetUserDictionaryPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetWorkPath() const
{
    return pImp->GetWorkPath();
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetAddinPath( const String& rPath )
{
    pImp->SetAddinPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetAutoCorrectPath( const String& rPath )
{
    pImp->SetAutoCorrectPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetAutoTextPath( const String& rPath )
{
    pImp->SetAutoTextPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetBackupPath( const String& rPath )
{
    pImp->SetBackupPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetBasicPath( const String& rPath )
{
    pImp->SetBasicPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetBitmapPath( const String& rPath )
{
    pImp->SetBitmapPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetConfigPath( const String& rPath )
{
    pImp->SetConfigPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetDictionaryPath( const String& rPath )
{
    pImp->SetDictionaryPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetFavoritesPath( const String& rPath )
{
    pImp->SetFavoritesPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetFilterPath( const String& rPath )
{
    pImp->SetFilterPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetGalleryPath( const String& rPath )
{
    pImp->SetGalleryPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetGraphicPath( const String& rPath )
{
    pImp->SetGraphicPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetHelpPath( const String& rPath )
{
    pImp->SetHelpPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetLinguisticPath( const String& rPath )
{
    pImp->SetLinguisticPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetModulePath( const String& rPath )
{
    pImp->SetModulePath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetPalettePath( const String& rPath )
{
    pImp->SetPalettePath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetPluginPath( const String& rPath )
{
    pImp->SetPluginPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetStoragePath( const String& rPath )
{
    pImp->SetStoragePath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetTempPath( const String& rPath )
{
    pImp->SetTempPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetTemplatePath( const String& rPath )
{
    pImp->SetTemplatePath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetUserConfigPath( const String& rPath )
{
    pImp->SetUserConfigPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetUserDictionaryPath( const String& rPath )
{
    pImp->SetUserDictionaryPath( rPath );
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetWorkPath( const String& rPath )
{
    pImp->SetWorkPath( rPath );
}

// -----------------------------------------------------------------------

String SvtPathOptions::SubstituteVariable( const String& rVar )
{
    String aRet = pImp->SubstituteAndConvert( rVar );
    return aRet;
}

String SvtPathOptions::UseVariable( const String& rPath )
{
    String aRet = pImp->UsePathVariables( rPath );
    return aRet;
}

// -----------------------------------------------------------------------

sal_Bool SvtPathOptions::SearchFile( String& rIniFile, Pathes ePath )
{
    // check parameter: empty inifile name?
    if ( !rIniFile.Len() )
    {
        DBG_ERRORFILE( "SvtPathOptions::SearchFile(): invalid parameter" );
        return sal_False;
    }

    String aIniFile = pImp->SubstVar( rIniFile );
    sal_Bool bRet = sal_False;

    switch ( ePath )
    {
        case PATH_USERCONFIG:
        case PATH_USERDICTIONARY:
        {
            // path is a URL
            sal_Bool bCfg = ( PATH_USERCONFIG == ePath );
            bRet = sal_True;
            INetURLObject aObj( bCfg ? GetUserConfigPath() : GetUserDictionaryPath() );
            xub_StrLen i, nCount = aIniFile.GetTokenCount( '/' );
            for ( i = 0; i < nCount; ++i )
                aObj.insertName( aIniFile.GetToken( i, '/' ) );

            if ( !::utl::UCBContentHelper::Exists( aObj.GetMainURL( INetURLObject::NO_DECODE ) ) )
            {
                aObj.SetSmartURL( bCfg ? GetConfigPath() : GetDictionaryPath() );
                aObj.insertName( aIniFile );
                bRet = ::utl::UCBContentHelper::Exists( aObj.GetMainURL( INetURLObject::NO_DECODE ) );
            }

            if ( bRet )
                rIniFile = aObj.GetMainURL( INetURLObject::NO_DECODE );

            break;
        }

        default:
        {
            String aPath;
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
            }

            sal_uInt16 i, nIdx = 0, nCount = aPath.GetTokenCount( SEARCHPATH_DELIMITER );
            for ( i = 0; i < nCount; ++i )
            {
                BOOL bIsURL = TRUE;
                String aPathToken = aPath.GetToken( 0, SEARCHPATH_DELIMITER, nIdx );
                INetURLObject aObj( aPathToken );
                if ( aObj.HasError() )
                {
                    bIsURL = FALSE;
                    String aURL;
                    if ( LocalFileHelper::ConvertPhysicalNameToURL( aPathToken, aURL ) )
                        aObj.SetURL( aURL );
                }

                xub_StrLen i, nCount = aIniFile.GetTokenCount( '/' );
                for ( i = 0; i < nCount; ++i )
                    aObj.insertName( aIniFile.GetToken( i, '/' ) );
                bRet = ::utl::UCBContentHelper::Exists( aObj.GetMainURL( INetURLObject::NO_DECODE ) );

                if ( bRet )
                {
                    if ( !bIsURL )
                        ::utl::LocalFileHelper::ConvertURLToPhysicalName(
                                            aObj.GetMainURL( INetURLObject::NO_DECODE ), rIniFile );
                    else
                        rIniFile = aObj.GetMainURL( INetURLObject::NO_DECODE );
                    break;
                }
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------

LanguageType SvtPathOptions::GetLanguageType() const
{
    return pImp->GetLanguageType();
}
// -----------------------------------------------------------------------
BOOL SvtPathOptions::IsPathReadonly(Pathes ePath)const
{
    return pImp->IsPathReadonly(ePath);
}

// class PathService -----------------------------------------------------

void SAL_CALL PathService::addPropertyChangeListener( const ::rtl::OUString& sKeyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException) {}
void SAL_CALL PathService::removePropertyChangeListener( const ::rtl::OUString& sKeyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException) {}
void SAL_CALL PathService::flush(  ) throw(::com::sun::star::uno::RuntimeException) {}

::rtl::OUString SAL_CALL PathService::substituteVariables( const ::rtl::OUString& sText ) throw(::com::sun::star::uno::RuntimeException)
{
    return SvtPathOptions().SubstituteVariable( sText );
}

::rtl::OUString SAL_CALL PathService::getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii("com.sun.star.comp.svtools.PathService");
}

sal_Bool SAL_CALL PathService::supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    if ( ServiceName.compareToAscii("com.sun.star.config.SpecialConfigManager") == COMPARE_EQUAL )
        return sal_True;
    else
        return sal_False;
}

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL PathService::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< OUString > aRet(1);
    *aRet.getArray() = OUString::createFromAscii("com.sun.star.config.SpecialConfigManager");
    return aRet;
}

