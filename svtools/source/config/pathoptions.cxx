/*************************************************************************
 *
 *  $RCSfile: pathoptions.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pb $ $Date: 2000-09-25 10:00:42 $
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
#include "iniman.hxx"

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
#ifndef _SV_SYSTEM_HXX
#include <vcl/system.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

// define ----------------------------------------------------------------

#define PATH_ADDIN                      0
#define PATH_AUTOCORRECT                1
#define PATH_AUTOPILOT                  2
#define PATH_BACKUP                     3
#define PATH_BASIC                      4
#define PATH_BITMAP                     5
#define PATH_CONFIG                     6
#define PATH_DATABASE                   7
#define PATH_DICTIONARY                 8
#define PATH_FAVORITES                  9
#define PATH_FILTER                     10
#define PATH_GALLERY                    11
#define PATH_GLOSSARY                   12
#define PATH_GRAPHIC                    13
#define PATH_HELP                       14
#define PATH_LINGUISTIC                 15
#define PATH_MODULE                     16
#define PATH_NEWMENU                    17
#define PATH_PALETTE                    18
#define PATH_PLUGIN                     19
#define PATH_STORAGE                    20
#define PATH_SUBINI                     21
#define PATH_TEMP                       22
#define PATH_TEMPLATE                   23
#define PATH_TRASH                      24
#define PATH_USERCONFIG                 25
#define PATH_USERDICTIONARY             26
#define PATH_WORK                       27

#define ASCII_STR(s)                    OUString( RTL_CONSTASCII_USTRINGPARAM(s) )

#define SIGN_STARTVARIABLE              ASCII_STR("$(")
#define SIGN_ENDVARIABLE                ASCII_STR(")")

#define SUBSTITUTE_INST                 ASCII_STR("$(inst)")
#define SUBSTITUTE_PROG                 ASCII_STR("$(prog)")
#define SUBSTITUTE_USER                 ASCII_STR("$(user)")
#define SUBSTITUTE_INSTURL              ASCII_STR("$(insturl)")
#define SUBSTITUTE_PROGURL              ASCII_STR("$(progurl)")
#define SUBSTITUTE_USERURL              ASCII_STR("$(userurl)")
#define SUBSTITUTE_PATH                 ASCII_STR("$(path)")
#define SUBSTITUTE_LANG                 ASCII_STR("$(lang)")
#define SUBSTITUTE_LANGID               ASCII_STR("$(langid)")
#define SUBSTITUTE_SYSLANGID            ASCII_STR("$(syslangid)")
#define SUBSTITUTE_VLANG                ASCII_STR("$(vlang)")

// Length of SUBSTITUTE_... to replace it with real values.
#define REPLACELENGTH_INST              7
#define REPLACELENGTH_PROG              7
#define REPLACELENGTH_USER              7
#define REPLACELENGTH_INSTURL           10
#define REPLACELENGTH_PROGURL           10
#define REPLACELENGTH_USERURL           10
#define REPLACELENGTH_PATH              7
#define REPLACELENGTH_LANG              7
#define REPLACELENGTH_LANGID            9
#define REPLACELENGTH_SYSLANGID         12
#define REPLACELENGTH_VLANG             8

// Strings to replace $(vlang)
#define REPLACEMENT_ARABIC              ASCII_STR("arabic")
#define REPLACEMENT_CZECH               ASCII_STR("czech")
#define REPLACEMENT_DANISH              ASCII_STR("danish")
#define REPLACEMENT_DUTCH               ASCII_STR("dutch")
#define REPLACEMENT_ENGLISH             ASCII_STR("english")
#define REPLACEMENT_FINNISH             ASCII_STR("finnish")
#define REPLACEMENT_FRENCH              ASCII_STR("french")
#define REPLACEMENT_GERMAN              ASCII_STR("german")
#define REPLACEMENT_GREEK               ASCII_STR("greek")
#define REPLACEMENT_HEBREW              ASCII_STR("hebrew")
#define REPLACEMENT_ITALIAN             ASCII_STR("italian")
#define REPLACEMENT_JAPANESE            ASCII_STR("japanese")
#define REPLACEMENT_KOREAN              ASCII_STR("korean")
#define REPLACEMENT_POLISH              ASCII_STR("polish")
#define REPLACEMENT_RUSSIAN             ASCII_STR("russian")
#define REPLACEMENT_SLOVAK              ASCII_STR("slovak")
#define REPLACEMENT_SPANISH             ASCII_STR("spanish")
#define REPLACEMENT_SWEDISH             ASCII_STR("swedish")
#define REPLACEMENT_TURKISH             ASCII_STR("turkish")
#define REPLACEMENT_NORWEGIAN           ASCII_STR("norwegian")
#define REPLACEMENT_HUNGARIAN           ASCII_STR("hungarian")
#define REPLACEMENT_BULGARIAN           ASCII_STR("bulgarian")
#define REPLACEMENT_CHINESE_TRADITIONAL ASCII_STR("chinese_traditional")
#define REPLACEMENT_CHINESE_SIMPLIFIED  ASCII_STR("chinese_simplified")
#define REPLACEMENT_PORTUGUESE          ASCII_STR("portuguese")

#define STRPOS_NOTFOUND                 -1

// class SvtPathOptions_Impl ---------------------------------------------

class SvtPathOptions_Impl : public utl::ConfigItem
{
private:
    String          m_aAddinPath;
    String          m_aAutoCorrectPath;
    String          m_aAutoPilotPath;
    String          m_aBackupPath;
    String          m_aBasicPath;
    String          m_aBitmapPath;
    String          m_aConfigPath;
    String          m_aDatabasePath;
    String          m_aDictionaryPath;
    String          m_aFavoritesPath;
    String          m_aFilterPath;
    String          m_aGalleryPath;
    String          m_aGlossaryPath;
    String          m_aGraphicPath;
    String          m_aHelpPath;
    String          m_aLinguisticPath;
    String          m_aModulePath;
    String          m_aNewMenuPath;
    String          m_aPalettePath;
    String          m_aPluginPath;
    String          m_aStoragePath;
    String          m_aSubIniPath;
    String          m_aTempPath;
    String          m_aTemplatePath;
    String          m_aTrashPath;
    String          m_aUserConfigPath;
    String          m_aUserDictionaryPath;
    String          m_aWorkPath;

    String          m_aEmptyString;
    String          m_aInstPath;
    String          m_aUserPath;
    String          m_aProgPath;
    String          m_aInstURL;
    String          m_aUserURL;
    String          m_aProgURL;

    LanguageType    m_eLanguageType;

    ::osl::Mutex    m_aMutex;

    // not const because of using a mutex
    const String&   GetPath( int nIndex );
    void            SetPath( int nIndex, const String& rNewPath );

    rtl::OUString   SubstVar( const rtl::OUString& rVar );

public:
                    SvtPathOptions_Impl();

    virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void    Commit();

    // get the pathes, not const because of using a mutex
    const String&   GetAddinPath() { return GetPath( PATH_ADDIN ); }
    const String&   GetAutoCorrectPath() { return GetPath( PATH_AUTOCORRECT ); }
    const String&   GetAutoPilotPath() { return GetPath( PATH_AUTOPILOT ); }
    const String&   GetBackupPath() { return GetPath( PATH_BACKUP ); }
    const String&   GetBasicPath() { return GetPath( PATH_BASIC ); }
    const String&   GetBitmapPath() { return GetPath( PATH_BITMAP ); }
    const String&   GetConfigPath() { return GetPath( PATH_CONFIG ); }
    const String&   GetDatabasePath() { return GetPath( PATH_DATABASE ); }
    const String&   GetDictionaryPath() { return GetPath( PATH_DICTIONARY ); }
    const String&   GetFavoritesPath() { return GetPath( PATH_FAVORITES ); }
    const String&   GetFilterPath() { return GetPath( PATH_FILTER ); }
    const String&   GetGalleryPath() { return GetPath( PATH_GALLERY ); }
    const String&   GetGlossaryPath() { return GetPath( PATH_GLOSSARY ); }
    const String&   GetGraphicPath() { return GetPath( PATH_GRAPHIC ); }
    const String&   GetHelpPath() { return GetPath( PATH_HELP ); }
    const String&   GetLinguisticPath() { return GetPath( PATH_LINGUISTIC ); }
    const String&   GetModulePath() { return GetPath( PATH_MODULE ); }
    const String&   GetNewMenuPath() { return GetPath( PATH_NEWMENU ); }
    const String&   GetPalettePath() { return GetPath( PATH_PALETTE ); }
    const String&   GetPluginPath() { return GetPath( PATH_PLUGIN ); }
    const String&   GetStoragePath() { return GetPath( PATH_STORAGE ); }
    const String&   GetSubIniPath() { return GetPath( PATH_SUBINI ); }
    const String&   GetTempPath() { return GetPath( PATH_TEMP ); }
    const String&   GetTemplatePath() { return GetPath( PATH_TEMPLATE ); }
    const String&   GetTrashPath() { return GetPath( PATH_TRASH ); }
    const String&   GetUserConfigPath() { return GetPath( PATH_USERCONFIG ); }
    const String&   GetUserDictionaryPath() { return GetPath( PATH_USERDICTIONARY ); }
    const String&   GetWorkPath() { return GetPath( PATH_WORK ); }

    // set the pathes
    void            SetAddinPath( const String& rPath ) { SetPath( PATH_ADDIN, rPath ); }
    void            SetAutoCorrectPath( const String& rPath ) { SetPath( PATH_AUTOCORRECT, rPath ); }
    void            SetAutoPilotPath( const String& rPath ) { SetPath( PATH_AUTOPILOT, rPath ); }
    void            SetBackupPath( const String& rPath ) { SetPath( PATH_BACKUP, rPath ); }
    void            SetBasicPath( const String& rPath ) { SetPath( PATH_BASIC, rPath ); }
    void            SetBitmapPath( const String& rPath ) { SetPath( PATH_BITMAP, rPath ); }
    void            SetConfigPath( const String& rPath ) { SetPath( PATH_CONFIG, rPath ); }
    void            SetDatabasePath( const String& rPath ) { SetPath( PATH_DATABASE, rPath ); }
    void            SetDictionaryPath( const String& rPath ) { SetPath( PATH_DICTIONARY, rPath ); }
    void            SetFavoritesPath( const String& rPath ) { SetPath( PATH_FAVORITES, rPath ); }
    void            SetFilterPath( const String& rPath ) { SetPath( PATH_FILTER, rPath ); }
    void            SetGalleryPath( const String& rPath ) { SetPath( PATH_GALLERY, rPath ); }
    void            SetGlossaryPath( const String& rPath ) { SetPath( PATH_GLOSSARY, rPath ); }
    void            SetGraphicPath( const String& rPath ) { SetPath( PATH_GRAPHIC, rPath ); }
    void            SetHelpPath( const String& rPath ) { SetPath( PATH_HELP, rPath ); }
    void            SetLinguisticPath( const String& rPath ) { SetPath( PATH_LINGUISTIC, rPath ); }
    void            SetModulePath( const String& rPath ) { SetPath( PATH_MODULE, rPath ); }
    void            SetNewMenuPath( const String& rPath ) { SetPath( PATH_NEWMENU, rPath ); }
    void            SetPalettePath( const String& rPath ) { SetPath( PATH_PALETTE, rPath ); }
    void            SetPluginPath( const String& rPath ) { SetPath( PATH_PLUGIN, rPath ); }
    void            SetStoragePath( const String& rPath ) { SetPath( PATH_STORAGE, rPath ); }
    void            SetSubIniPath( const String& rPath ) { SetPath( PATH_SUBINI, rPath ); }
    void            SetTempPath( const String& rPath ) { SetPath( PATH_TEMP, rPath ); }
    void            SetTemplatePath( const String& rPath ) { SetPath( PATH_TEMPLATE, rPath ); }
    void            SetTrashPath( const String& rPath ) { SetPath( PATH_TRASH, rPath ); }
    void            SetUserConfigPath( const String& rPath ) { SetPath( PATH_USERCONFIG, rPath ); }
    void            SetUserDictionaryPath( const String& rPath ) { SetPath( PATH_USERDICTIONARY, rPath ); }
    void            SetWorkPath( const String& rPath ) { SetPath( PATH_WORK, rPath ); }
};

// global ----------------------------------------------------------------

static SvtPathOptions_Impl* pOptions = NULL;
static sal_Int32            nRefCount = 0;

// functions -------------------------------------------------------------

Sequence< OUString > GetPathPropertyNames()
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

    const int nCount = sizeof( aPropNames ) / sizeof( const char* );
    Sequence< OUString > aNames( nCount );
    OUString* pNames = aNames.getArray();
    for ( int i = 0; i < nCount; i++ )
        pNames[i] = OUString::createFromAscii( aPropNames[i] );

    return aNames;
}

// class SvtPathOptions_Impl ---------------------------------------------

const String& SvtPathOptions_Impl::GetPath( int nIndex )
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

void SvtPathOptions_Impl::SetPath( int nIndex, const String& rNewPath )
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

OUString SvtPathOptions_Impl::SubstVar( const OUString& rVar )
{
    // Don't work at parameter-string directly. Copy it.
    OUString aWorkText = rVar;

    // Search for first occure of "$(...".
    sal_Int32 nPosition = aWorkText.indexOf( SIGN_STARTVARIABLE );  // = first position of "$(" in string
    sal_Int32 nLength = 0; // = count of letters from "$(" to ")" in string
    sal_Bool bConvertToPath = sal_False;

    // Have we found any variable like "$(...)"?
    if ( nPosition != STRPOS_NOTFOUND )
    {
        // Yes; Get length of found variable.
        // If no ")" was found - nLength is set to 0 by default! see before.
        sal_Int32 nEndPosition = aWorkText.indexOf( SIGN_ENDVARIABLE, nPosition );
        if ( nEndPosition != STRPOS_NOTFOUND )
        {
            nLength = nEndPosition - nPosition;
            nLength++; // First index in string is 0!
        }
    }

    // Is there something for replace ?
    while ( ( nPosition != STRPOS_NOTFOUND ) && ( nLength > 0 ) )
    {
        // YES; Get the next variable for replace.
        OUString aReplacement;
        OUString aSubString = aWorkText.copy( nPosition, nLength );
        aSubString.toLowerCase();
        sal_Int32 nReplaceLength = 0;

        // -------------------------------------------------------------------------------------------------------------------
        // $(inst) - directory of the master (server) installation
        if ( SUBSTITUTE_INST == aSubString )
        {
            nReplaceLength = REPLACELENGTH_INST;
            aReplacement = m_aInstURL;
            bConvertToPath = sal_True;
        }
        else
        // -------------------------------------------------------------------------------------------------------------------
        // $(user) - directory of the user installation (== dir of soffice.ini)
        if ( SUBSTITUTE_USER == aSubString )
        {
            nReplaceLength = REPLACELENGTH_USER;
            aReplacement = m_aUserURL;
            bConvertToPath = sal_True;
        }
        else
        // -------------------------------------------------------------------------------------------------------------------
        // $(prog) - directory of the executable file
        if ( SUBSTITUTE_PROG == aSubString )
        {
            nReplaceLength = REPLACELENGTH_PROG;
            aReplacement = m_aProgURL;
            bConvertToPath = sal_True;
        }
        else
        // -------------------------------------------------------------------------------------------------------------------
        // $(insturl) - directory of the master (server) installation as URL
        if ( SUBSTITUTE_INSTURL == aSubString )
        {
            nReplaceLength = REPLACELENGTH_INSTURL;
            aReplacement = m_aInstURL;
        }
        else
        // -------------------------------------------------------------------------------------------------------------------
        // $(userurl) - directory of the user installation (== dir of soffice.ini)
        if ( SUBSTITUTE_USERURL == aSubString )
        {
            nReplaceLength = REPLACELENGTH_USERURL;
            aReplacement = m_aUserURL;
        }
        else
        // -------------------------------------------------------------------------------------------------------------------
        // $(progurl) - directory of the executable file as URL
        if ( SUBSTITUTE_PROGURL == aSubString )
        {
            nReplaceLength = REPLACELENGTH_PROGURL;
            aReplacement = m_aProgURL;
        }
        else
        // -------------------------------------------------------------------------------------------------------------------
        // $(path)
        if ( SUBSTITUTE_PATH == aSubString )
        {
            nReplaceLength = REPLACELENGTH_PATH;
            aReplacement = OUString::createFromAscii( getenv( "path" ) );
        }
        else
        // -------------------------------------------------------------------------------------------------------------------
        // $(lang) - language dependent directory with LanguageId used as directory name
        if ( SUBSTITUTE_LANG == aSubString )
        {
            nReplaceLength = REPLACELENGTH_LANG;
            aReplacement = OUString::createFromAscii( ResMgr::GetLang( m_eLanguageType, 0 ) );
        }
        else
        // -------------------------------------------------------------------------------------------------------------------
        // $(langid) - LanguageType of the application as string (for example "1031")
        if ( SUBSTITUTE_LANGID == aSubString )
        {
               nReplaceLength = REPLACELENGTH_LANGID;
               aReplacement = OUString::valueOf( (sal_uInt16)m_eLanguageType );
        }
        else
        // -------------------------------------------------------------------------------------------------------------------
        // $(syslangid) - LanguageType of the system as string (for example "1031")
        if ( SUBSTITUTE_SYSLANGID == aSubString )
        {
            nReplaceLength = REPLACELENGTH_SYSLANGID;
            aReplacement = OUString::valueOf( (sal_uInt16)System::GetLanguage() );
        }
        else
        // -------------------------------------------------------------------------------------------------------------------
        // $(vlang) - language dependent directory with english language name as directory name
        if ( SUBSTITUTE_VLANG == aSubString )
        {
            nReplaceLength = REPLACELENGTH_VLANG ;
            switch ( m_eLanguageType )
            {
                case LANGUAGE_ARABIC                :
                case LANGUAGE_ARABIC_IRAQ           :
                case LANGUAGE_ARABIC_EGYPT          :
                case LANGUAGE_ARABIC_LIBYA          :
                case LANGUAGE_ARABIC_ALGERIA        :
                case LANGUAGE_ARABIC_MOROCCO        :
                case LANGUAGE_ARABIC_TUNISIA        :
                case LANGUAGE_ARABIC_OMAN           :
                case LANGUAGE_ARABIC_YEMEN          :
                case LANGUAGE_ARABIC_SYRIA          :
                case LANGUAGE_ARABIC_JORDAN         :
                case LANGUAGE_ARABIC_LEBANON        :
                case LANGUAGE_ARABIC_KUWAIT         :
                case LANGUAGE_ARABIC_UAE            :
                case LANGUAGE_ARABIC_BAHRAIN        :
                case LANGUAGE_ARABIC_QATAR          :   aReplacement = REPLACEMENT_ARABIC;
                                                        break ;

                case LANGUAGE_CZECH                 :   aReplacement = REPLACEMENT_CZECH;
                                                        break ;

                case LANGUAGE_DANISH                :   aReplacement = REPLACEMENT_DANISH;
                                                        break ;

                case LANGUAGE_DUTCH                 :
                case LANGUAGE_DUTCH_BELGIAN         :   aReplacement = REPLACEMENT_DUTCH;
                                                        break ;

                case LANGUAGE_ENGLISH               :
                case LANGUAGE_ENGLISH_AUS           :
                case LANGUAGE_ENGLISH_CAN           :
                case LANGUAGE_ENGLISH_NZ            :
                case LANGUAGE_ENGLISH_EIRE          :
                case LANGUAGE_ENGLISH_SAFRICA       :
                case LANGUAGE_ENGLISH_JAMAICA       :
                case LANGUAGE_ENGLISH_CARRIBEAN     :
                case LANGUAGE_ENGLISH_BELIZE        :
                case LANGUAGE_ENGLISH_TRINIDAD      :
                case LANGUAGE_ENGLISH_ZIMBABWE      :
                case LANGUAGE_ENGLISH_PHILIPPINES   :
                case LANGUAGE_ENGLISH_US            :
                case LANGUAGE_ENGLISH_UK            :   aReplacement = REPLACEMENT_ENGLISH;
                                                        break ;

                case LANGUAGE_FINNISH               :   aReplacement = REPLACEMENT_FINNISH;
                                                        break ;

                case LANGUAGE_FRENCH                :
                case LANGUAGE_FRENCH_BELGIAN        :
                case LANGUAGE_FRENCH_CANADIAN       :
                case LANGUAGE_FRENCH_SWISS          :
                case LANGUAGE_FRENCH_LUXEMBOURG     :
                case LANGUAGE_FRENCH_MONACO         :   aReplacement = REPLACEMENT_FRENCH;
                                                        break ;

                case LANGUAGE_GERMAN                :
                case LANGUAGE_GERMAN_SWISS          :
                case LANGUAGE_GERMAN_AUSTRIAN       :
                case LANGUAGE_GERMAN_LUXEMBOURG     :
                case LANGUAGE_GERMAN_LIECHTENSTEIN  :   aReplacement = REPLACEMENT_GERMAN;
                                                        break ;

                case LANGUAGE_GREEK                 :   aReplacement = REPLACEMENT_GREEK;
                                                        break ;

                case LANGUAGE_HEBREW                :   aReplacement = REPLACEMENT_HEBREW;
                                                        break ;

                case LANGUAGE_ITALIAN               :
                case LANGUAGE_ITALIAN_SWISS         :   aReplacement = REPLACEMENT_ITALIAN;
                                                        break ;

                case LANGUAGE_JAPANESE              :   aReplacement = REPLACEMENT_JAPANESE;
                                                        break ;

                case LANGUAGE_KOREAN                :
                case LANGUAGE_KOREAN_JOHAB          :   aReplacement = REPLACEMENT_KOREAN;
                                                        break ;

                case LANGUAGE_POLISH                :   aReplacement = REPLACEMENT_POLISH;
                                                        break ;

                case LANGUAGE_RUSSIAN               :   aReplacement = REPLACEMENT_RUSSIAN;
                                                        break ;

                case LANGUAGE_SLOVAK                :   aReplacement = REPLACEMENT_SLOVAK;
                                                        break ;

                case LANGUAGE_SPANISH               :
                case LANGUAGE_SPANISH_MEXICAN       :
                case LANGUAGE_SPANISH_MODERN        :
                case LANGUAGE_SPANISH_GUATEMALA     :
                case LANGUAGE_SPANISH_COSTARICA     :
                case LANGUAGE_SPANISH_PANAMA        :
                case LANGUAGE_SPANISH_DOMINICAN_REPUBLIC:
                case LANGUAGE_SPANISH_VENEZUELA     :
                case LANGUAGE_SPANISH_COLOMBIA      :
                case LANGUAGE_SPANISH_PERU          :
                case LANGUAGE_SPANISH_ARGENTINA     :
                case LANGUAGE_SPANISH_ECUADOR       :
                case LANGUAGE_SPANISH_CHILE         :
                case LANGUAGE_SPANISH_URUGUAY       :
                case LANGUAGE_SPANISH_PARAGUAY      :
                case LANGUAGE_SPANISH_BOLIVIA       :
                case LANGUAGE_SPANISH_EL_SALVADOR   :
                case LANGUAGE_SPANISH_HONDURAS      :
                case LANGUAGE_SPANISH_NICARAGUA     :
                case LANGUAGE_SPANISH_PUERTO_RICO   :   aReplacement = REPLACEMENT_SPANISH;
                                                        break ;

                case LANGUAGE_SWEDISH               :
                case LANGUAGE_SWEDISH_FINLAND       :   aReplacement = REPLACEMENT_SWEDISH;
                                                        break ;

                case LANGUAGE_TURKISH               :   aReplacement = REPLACEMENT_TURKISH;
                                                        break ;

                case LANGUAGE_NORWEGIAN             :
                case LANGUAGE_NORWEGIAN_BOKMAL      :
                case LANGUAGE_NORWEGIAN_NYNORSK     :   aReplacement = REPLACEMENT_NORWEGIAN;
                                                        break ;

                case LANGUAGE_HUNGARIAN             :   aReplacement = REPLACEMENT_HUNGARIAN;
                                                        break ;

                case LANGUAGE_BULGARIAN             :   aReplacement = REPLACEMENT_BULGARIAN;
                                                        break ;

                case LANGUAGE_CHINESE_TRADITIONAL   :   aReplacement = REPLACEMENT_CHINESE_TRADITIONAL;
                                                        break ;

                case LANGUAGE_CHINESE_SIMPLIFIED    :   aReplacement = REPLACEMENT_CHINESE_SIMPLIFIED;
                                                        break ;

                case LANGUAGE_PORTUGUESE            :
                case LANGUAGE_PORTUGUESE_BRAZILIAN  :   aReplacement = REPLACEMENT_PORTUGUESE;
                                                        break ;

                default                             :   // no fallback any longer
                                                        DBG_ERRORFILE( "Language unknown." );
                                                        break ;
            }
        }

        // Have we found something to replace?
        if ( nReplaceLength > 0 )
        {
            // Yes ... then do it.
            aWorkText = aWorkText.replaceAt( nPosition, nReplaceLength, aReplacement );
        }
        else
        {
            // Safe impossible cases
            // Unknown variable was found! We have detected "$(...)" but don't know his meaning.
            DBG_ERRORFILE( "Unknown variable was found" );
            // Skip unknown variable.
            nPosition += nLength;
        }

        // Step after replaced text! If no text was replaced (unknown variable!),
        // length of aReplacement is 0 ... and we don't step then.
        nPosition += aReplacement.getLength();

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
            // Else; Position is valid. Search for next variable to replace.
            nPosition = aWorkText.indexOf( SIGN_STARTVARIABLE, nPosition );
            // Have we found any variable like "$(...)"?
            if ( nPosition != STRPOS_NOTFOUND )
            {
                // Yes; Get length of found variable. If no ")" was found - nLength must set to 0!
                nLength = 0;
                sal_Int32 nEndPosition = aWorkText.indexOf( SIGN_ENDVARIABLE, nPosition );
                if ( nEndPosition != STRPOS_NOTFOUND )
                {
                    nLength = nEndPosition - nPosition;
                    nLength++;
                }
            }
        }
    }

    if ( bConvertToPath )
    {
        String aNew;
        for ( sal_Int32 i = 0; i < aWorkText.getTokenCount(); ++i )
        {
            INetURLObject aTemp( aWorkText.getToken(i) );
            if ( aNew.Len() > 0 )
                aNew += ';';
            aNew += aTemp.getFSysPath( INetURLObject::FSYS_DETECT );
        }
        aWorkText = aNew;
    }
    // Return text with replaced substrings.
    return aWorkText ;
}

// -----------------------------------------------------------------------

SvtPathOptions_Impl::SvtPathOptions_Impl() :

    ConfigItem( ASCII_STR("Office.Common/Path") )

{
    // only temporary
    INetURLObject aOfficePath( Application::GetAppFileName(), INET_PROT_FILE );
    aOfficePath.removeSegment();
    aOfficePath.removeFinalSlash();

    m_aProgPath = aOfficePath.getFSysPath( INetURLObject::FSYS_DETECT );
    m_aProgURL = aOfficePath.GetMainURL();

    aOfficePath.removeSegment();
    aOfficePath.removeFinalSlash();
    m_aInstPath = aOfficePath.getFSysPath( INetURLObject::FSYS_DETECT );
    m_aInstURL = aOfficePath.GetMainURL();

    aOfficePath.insertName( ASCII_STR("user") );
    aOfficePath.removeFinalSlash();
    m_aUserPath = aOfficePath.getFSysPath( INetURLObject::FSYS_DETECT );
    m_aUserURL = aOfficePath.GetMainURL();

    m_eLanguageType = LANGUAGE_ENGLISH_US;
    SfxIniManager* pIniMgr = SfxIniManager::Get();
    if ( pIniMgr )
    {
        String aLocale = pIniMgr->Get( SFX_KEY_LANGUAGE );
        m_eLanguageType = ConvertIsoStringToLanguage( aLocale, '_' );
    }

    Sequence< OUString > aNames = GetPathPropertyNames();
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
                if ( pValues[nProp] >>= aTempStr )
                {
                    aTempStr = SubstVar( aTempStr );
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
                else
                {
                    DBG_ERRORFILE( "Wrong any type" );
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

void SvtPathOptions_Impl::Commit()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Sequence< OUString > aNames = GetPathPropertyNames();
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

void SvtPathOptions_Impl::Notify( const Sequence<rtl::OUString>& aPropertyNames )
{
    DBG_ERRORFILE( "properties have been changed" );
}

// class SvtPathOptions --------------------------------------------------

SvtPathOptions::SvtPathOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() );
    if ( !pOptions )
        pOptions = new SvtPathOptions_Impl;
    ++nRefCount;
    pImp = pOptions;
}

// -----------------------------------------------------------------------

SvtPathOptions::~SvtPathOptions()
{
    // Global access, must be guarded (multithreading)
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if ( !--nRefCount )
        DELETEZ( pOptions );
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

const String& SvtPathOptions::GetAutoPilotPath() const
{
    return pImp->GetAutoPilotPath();
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

const String& SvtPathOptions::GetDatabasePath() const
{
    return pImp->GetDatabasePath();
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

const String& SvtPathOptions::GetGlossaryPath() const
{
    return pImp->GetGlossaryPath();
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

const String& SvtPathOptions::GetNewMenuPath() const
{
    return pImp->GetNewMenuPath();
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

const String& SvtPathOptions::GetSubIniPath() const
{
    return pImp->GetSubIniPath();
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

const String& SvtPathOptions::GetTrashPath() const
{
    return pImp->GetTrashPath();
}

// -----------------------------------------------------------------------

const String& SvtPathOptions::GetUserConfigPath() const
{
    return pImp->GetUserConfigPath();
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

void SvtPathOptions::SetAutoPilotPath( const String& rPath )
{
    pImp->SetAutoPilotPath( rPath );
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

void SvtPathOptions::SetDatabasePath( const String& rPath )
{
    pImp->SetDatabasePath( rPath );
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

void SvtPathOptions::SetGlossaryPath( const String& rPath )
{
    pImp->SetGlossaryPath( rPath );
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

void SvtPathOptions::SetNewMenuPath( const String& rPath )
{
    pImp->SetNewMenuPath( rPath );
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

void SvtPathOptions::SetSubIniPath( const String& rPath )
{
    pImp->SetSubIniPath( rPath );
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

void SvtPathOptions::SetTrashPath( const String& rPath )
{
    pImp->SetTrashPath( rPath );
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

