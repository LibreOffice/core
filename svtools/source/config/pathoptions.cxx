/*************************************************************************
 *
 *  $RCSfile: pathoptions.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:56 $
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

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

// class SvtPathOptions --------------------------------------------------

// initialize the static member
SvtPathOptions* SvtPathOptions::m_pOptions = NULL;

// -----------------------------------------------------------------------

Sequence< OUString > SvtPathOptions::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Addin",            // PATH_ADDIN
        "AutoCorrect",      // PATH_AUTOCORRECT
        "AutoPilot",        // PATH_AUTOPILOT
        "Backup",           // PATH_BACKUP
        "Basic",            // PATH_BASIC
        "Bitmap",           // PATH_BITMAP
        "Config",           // PATH_CONFIG
        "Database",         // PATH_DATABASE
        "Dictionary",       // PATH_DICTIONARY
        "Favorite",         // PATH_FAVORITES
        "Filter",           // PATH_FILTER
        "Gallery",          // PATH_GALLERY
        "Glossary",         // PATH_GLOSSARY
        "Graphic",          // PATH_GRAPHIC
        "Help",             // PATH_HELP
        "Linguistic",       // PATH_LINGUISTIC
        "Module",           // PATH_MODULE
        "New",              // PATH_NEWMENU
        "Palette",          // PATH_PALETTE
        "Plugin",           // PATH_PLUGIN
        "Storage",          // PATH_STORAGE
        "SubIni",           // PATH_SUBINI
        "Temp",             // PATH_TEMP
        "Template",         // PATH_TEMPLATE
        "Trash",            // PATH_TRASH
        "UserConfig",       // PATH_USERCONFIG
        "UserDictionary",   // PATH_USERDICTIONARY
        "Work"              // PATH_WORK
    };

    const int nCount = 28;
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for ( int i = 0; i < nCount; i++ )
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return aNames;
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetPath( int nIndex )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( PATH_ADDIN == nIndex )
        return m_aAddinPath;
    else if ( PATH_AUTOCORRECT == nIndex )
        return m_aAutoCorrectPath;
    else if ( PATH_AUTOPILOT == nIndex )
        return m_aAutoPilotPath;
    else if ( PATH_BACKUP == nIndex )
        return m_aBackupPath;
    else if ( PATH_BASIC == nIndex )
        return m_aBasicPath;
    else if ( PATH_BITMAP == nIndex )
        return m_aBitmapPath;
    else if ( PATH_CONFIG == nIndex )
        return m_aConfigPath;
    else if ( PATH_DATABASE == nIndex )
        return m_aDatabasePath;
    else if ( PATH_DICTIONARY == nIndex )
        return m_aDictionaryPath;
    else if ( PATH_FAVORITES == nIndex )
        return m_aFavoritesPath;
    else if ( PATH_FILTER == nIndex )
        return m_aFilterPath;
    else if ( PATH_GALLERY == nIndex )
        return m_aGalleryPath;
    else if ( PATH_GLOSSARY == nIndex )
        return m_aGlossaryPath;
    else if ( PATH_GRAPHIC == nIndex )
        return m_aGraphicPath;
    else if ( PATH_HELP == nIndex )
        return m_aHelpPath;
    else if ( PATH_LINGUISTIC == nIndex )
        return m_aLinguisticPath;
    else if ( PATH_MODULE == nIndex )
        return m_aModulePath;
    else if ( PATH_NEWMENU == nIndex )
        return m_aNewMenuPath;
    else if ( PATH_PALETTE == nIndex )
        return m_aPalettePath;
    else if ( PATH_PLUGIN == nIndex )
        return m_aPluginPath;
    else if ( PATH_STORAGE == nIndex )
        return m_aStoragePath;
    else if ( PATH_SUBINI == nIndex )
        return m_aSubIniPath;
    else if ( PATH_TEMP == nIndex )
        return m_aTempPath;
    else if ( PATH_TEMPLATE == nIndex )
        return m_aTemplatePath;
    else if ( PATH_TRASH == nIndex )
        return m_aTrashPath;
    else if ( PATH_USERCONFIG == nIndex )
        return m_aUserConfigPath;
    else if ( PATH_USERDICTIONARY == nIndex )
        return m_aUserDictionaryPath;
    else if ( PATH_WORK == nIndex )
        return m_aWorkPath;
    else
    {
        DBG_ERRORFILE( "invalid index to get a path" );
        return m_aEmptyString;
    }
}

// -----------------------------------------------------------------------

void SvtPathOptions::SetPath( int nIndex, const String& rNewPath )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( PATH_ADDIN == nIndex )
        m_aAddinPath = rNewPath;
    else if ( PATH_AUTOCORRECT == nIndex )
        m_aAutoCorrectPath = rNewPath;
    else if ( PATH_AUTOPILOT == nIndex )
        m_aAutoPilotPath = rNewPath;
    else if ( PATH_BACKUP == nIndex )
        m_aBackupPath = rNewPath;
    else if ( PATH_BASIC == nIndex )
        m_aBasicPath = rNewPath;
    else if ( PATH_BITMAP == nIndex )
        m_aBitmapPath = rNewPath;
    else if ( PATH_CONFIG == nIndex )
        m_aConfigPath = rNewPath;
    else if ( PATH_DATABASE == nIndex )
        m_aDatabasePath = rNewPath;
    else if ( PATH_DICTIONARY == nIndex )
        m_aDictionaryPath = rNewPath;
    else if ( PATH_FAVORITES == nIndex )
        m_aFavoritesPath = rNewPath;
    else if ( PATH_FILTER == nIndex )
        m_aFilterPath = rNewPath;
    else if ( PATH_GALLERY == nIndex )
        m_aGalleryPath = rNewPath;
    else if ( PATH_GLOSSARY == nIndex )
        m_aGlossaryPath = rNewPath;
    else if ( PATH_GRAPHIC == nIndex )
        m_aGraphicPath = rNewPath;
    else if ( PATH_HELP == nIndex )
        m_aHelpPath = rNewPath;
    else if ( PATH_LINGUISTIC == nIndex )
        m_aLinguisticPath = rNewPath;
    else if ( PATH_MODULE == nIndex )
        m_aModulePath = rNewPath;
    else if ( PATH_NEWMENU == nIndex )
        m_aNewMenuPath = rNewPath;
    else if ( PATH_PALETTE == nIndex )
        m_aPalettePath = rNewPath;
    else if ( PATH_PLUGIN == nIndex )
        m_aPluginPath = rNewPath;
    else if ( PATH_STORAGE == nIndex )
        m_aStoragePath = rNewPath;
    else if ( PATH_SUBINI == nIndex )
        m_aSubIniPath = rNewPath;
    else if ( PATH_TEMP == nIndex )
        m_aTempPath = rNewPath;
    else if ( PATH_TEMPLATE == nIndex )
        m_aTemplatePath = rNewPath;
    else if ( PATH_TRASH == nIndex )
        m_aTrashPath = rNewPath;
    else if ( PATH_USERCONFIG == nIndex )
        m_aUserConfigPath = rNewPath;
    else if ( PATH_USERDICTIONARY == nIndex )
        m_aUserDictionaryPath = rNewPath;
    else if ( PATH_WORK == nIndex )
        m_aWorkPath = rNewPath;
    else
        DBG_ERRORFILE( "invalid index to set a path" );
    SetModified();
}

// -----------------------------------------------------------------------

SvtPathOptions::SvtPathOptions() : ConfigItem( OUString::createFromAscii("Office/Common/Path") )
{
    Sequence< OUString > aNames = GetPropertyNames();
    Sequence< Any > aValues = GetProperties( aNames );
    EnableNotification( aNames );
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT( aValues.getLength() == aNames.getLength(), "GetProperties failed" );
    if ( aValues.getLength() == aNames.getLength() )
    {
        OUString aTempStr;

        for ( int nProp = 0; nProp < aNames.getLength(); nProp++ )
        {
            DBG_ASSERT( pValues[nProp].hasValue(), "property value missing" );
            if ( pValues[nProp].hasValue() )
            {
                pValues[nProp] >>= aTempStr;
                switch ( nProp )
                {
                    case PATH_ADDIN:            m_aAddinPath = String( aTempStr );          break;
                    case PATH_AUTOCORRECT:      m_aAutoCorrectPath = String( aTempStr );    break;
                    case PATH_AUTOPILOT:        m_aAutoPilotPath = String( aTempStr );      break;
                    case PATH_BACKUP:           m_aBackupPath = String( aTempStr );         break;
                    case PATH_BASIC:            m_aBasicPath = String( aTempStr );          break;
                    case PATH_BITMAP:           m_aBitmapPath = String( aTempStr );         break;
                    case PATH_CONFIG:           m_aConfigPath = String( aTempStr );         break;
                    case PATH_DATABASE:         m_aDatabasePath = String( aTempStr );       break;
                    case PATH_DICTIONARY:       m_aDictionaryPath = String( aTempStr );     break;
                    case PATH_FAVORITES:        m_aFavoritesPath = String( aTempStr );      break;
                    case PATH_FILTER:           m_aFilterPath = String( aTempStr );         break;
                    case PATH_GALLERY:          m_aGalleryPath = String( aTempStr );        break;
                    case PATH_GLOSSARY:         m_aGlossaryPath = String( aTempStr );       break;
                    case PATH_GRAPHIC:          m_aGraphicPath = String( aTempStr );        break;
                    case PATH_HELP:             m_aHelpPath = String( aTempStr );           break;
                    case PATH_LINGUISTIC:       m_aLinguisticPath = String( aTempStr );     break;
                    case PATH_MODULE:           m_aModulePath = String( aTempStr );         break;
                    case PATH_NEWMENU:          m_aNewMenuPath = String( aTempStr );        break;
                    case PATH_PALETTE:          m_aPalettePath = String( aTempStr );        break;
                    case PATH_PLUGIN:           m_aPluginPath = String( aTempStr );         break;
                    case PATH_STORAGE:          m_aStoragePath = String( aTempStr );        break;
                    case PATH_SUBINI:           m_aSubIniPath = String( aTempStr );         break;
                    case PATH_TEMP:             m_aTempPath = String( aTempStr );           break;
                    case PATH_TEMPLATE:         m_aTemplatePath = String( aTempStr );       break;
                    case PATH_TRASH:            m_aTrashPath = String( aTempStr );          break;
                    case PATH_USERCONFIG:       m_aUserConfigPath = String( aTempStr );     break;
                    case PATH_USERDICTIONARY:   m_aUserDictionaryPath = String( aTempStr ); break;
                    case PATH_WORK:             m_aWorkPath = String( aTempStr );           break;

                    default:
                        DBG_ERRORFILE( "invalid index to load a path" );
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

SvtPathOptions::~SvtPathOptions()
{
}

// -----------------------------------------------------------------------

SvtPathOptions* SvtPathOptions::GetPathOptions()
{
    if ( !m_pOptions )
        m_pOptions = new SvtPathOptions;
    return m_pOptions;
}

// -----------------------------------------------------------------------

void SvtPathOptions::DestroyPathOptions()
{
    if ( m_pOptions )
        DELETEZ( m_pOptions );
}

// -----------------------------------------------------------------------

void SvtPathOptions::Commit()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Sequence< OUString > aNames = GetPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence< Any > aValues( aNames.getLength() );
    Any* pValues = aValues.getArray();
    const Type& rType = ::getBooleanCppuType();
    OUString aTempStr;
    for ( int nProp = 0; nProp < aNames.getLength(); nProp++ )
    {
        switch ( nProp )
        {
            case PATH_ADDIN:            aTempStr = OUString( m_aAddinPath );            break;
            case PATH_AUTOCORRECT:      aTempStr = OUString( m_aAutoCorrectPath );      break;
            case PATH_AUTOPILOT:        aTempStr = OUString( m_aAutoPilotPath );        break;
            case PATH_BACKUP:           aTempStr = OUString( m_aBackupPath );           break;
            case PATH_BASIC:            aTempStr = OUString( m_aBasicPath );            break;
            case PATH_BITMAP:           aTempStr = OUString( m_aBitmapPath );           break;
            case PATH_CONFIG:           aTempStr = OUString( m_aConfigPath );           break;
            case PATH_DATABASE:         aTempStr = OUString( m_aDatabasePath );         break;
            case PATH_DICTIONARY:       aTempStr = OUString( m_aDictionaryPath );       break;
            case PATH_FAVORITES:        aTempStr = OUString( m_aFavoritesPath );        break;
            case PATH_FILTER:           aTempStr = OUString( m_aFilterPath );           break;
            case PATH_GALLERY:          aTempStr = OUString( m_aGalleryPath );          break;
            case PATH_GLOSSARY:         aTempStr = OUString( m_aGlossaryPath );         break;
            case PATH_GRAPHIC:          aTempStr = OUString( m_aGraphicPath );          break;
            case PATH_HELP:             aTempStr = OUString( m_aHelpPath );             break;
            case PATH_LINGUISTIC:       aTempStr = OUString( m_aLinguisticPath );       break;
            case PATH_MODULE:           aTempStr = OUString( m_aModulePath );           break;
            case PATH_NEWMENU:          aTempStr = OUString( m_aNewMenuPath );          break;
            case PATH_PALETTE:          aTempStr = OUString( m_aPalettePath );          break;
            case PATH_PLUGIN:           aTempStr = OUString( m_aPluginPath );           break;
            case PATH_STORAGE:          aTempStr = OUString( m_aStoragePath );          break;
            case PATH_SUBINI:           aTempStr = OUString( m_aSubIniPath );           break;
            case PATH_TEMP:             aTempStr = OUString( m_aTempPath );             break;
            case PATH_TEMPLATE:         aTempStr = OUString( m_aTemplatePath );         break;
            case PATH_TRASH:            aTempStr = OUString( m_aTrashPath );            break;
            case PATH_USERCONFIG:       aTempStr = OUString( m_aUserConfigPath );       break;
            case PATH_USERDICTIONARY:   aTempStr = OUString( m_aUserDictionaryPath );   break;
            case PATH_WORK:             aTempStr = OUString( m_aWorkPath );             break;

            default:
                DBG_ERRORFILE( "invalid index to save a path" );
        }
        pValues[nProp] <<= aTempStr;
    }
    PutProperties( aNames, aValues );
}

// -----------------------------------------------------------------------

void SvtPathOptions::Notify( const Sequence<rtl::OUString>& aPropertyNames )
{
    DBG_ERRORFILE( "properties have been changed" );
}

