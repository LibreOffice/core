/*************************************************************************
 *
 *  $RCSfile: pathoptions.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: mba $ $Date: 2000-11-17 12:34:52 $
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

#include <vos/process.hxx>
#include <unotools/localfilehelper.hxx>

using namespace osl;
using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

// define ----------------------------------------------------------------

#define SEARCHPATH_DELIMITER            ';'

#define ASCII_STR(s)                    OUString( RTL_CONSTASCII_USTRINGPARAM(s) )

#define SIGN_STARTVARIABLE              ASCII_STR("$(")
#define SIGN_ENDVARIABLE                ASCII_STR(")")

#define SUBSTITUTE_INST                 ASCII_STR("$(inst)")
#define SUBSTITUTE_PROG                 ASCII_STR("$(prog)")
#define SUBSTITUTE_USER                 ASCII_STR("$(user)")
#define SUBSTITUTE_INSTPATH             ASCII_STR("$(inst)")
#define SUBSTITUTE_PROGPATH             ASCII_STR("$(prog)")
#define SUBSTITUTE_USERPATH             ASCII_STR("$(user)")
#define SUBSTITUTE_INSTURL              ASCII_STR("$(insturl)")
#define SUBSTITUTE_PROGURL              ASCII_STR("$(progurl)")
#define SUBSTITUTE_USERURL              ASCII_STR("$(userurl)")
#define SUBSTITUTE_PATH                 ASCII_STR("$(path)")
#define SUBSTITUTE_LANG                 ASCII_STR("$(lang)")
#define SUBSTITUTE_LANGID               ASCII_STR("$(langid)")
//#define SUBSTITUTE_SYSLANGID            ASCII_STR("$(syslangid)")
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
//#define REPLACELENGTH_SYSLANGID         12
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
    String          m_aAutoTextPath;
    String          m_aBackupPath;
    String          m_aBasicPath;
    String          m_aBitmapPath;
    String          m_aConfigPath;
    String          m_aDatabasePath;
    String          m_aDictionaryPath;
    String          m_aFavoritesPath;
    String          m_aFilterPath;
    String          m_aGalleryPath;
    String          m_aGraphicPath;
    String          m_aHelpPath;
    String          m_aLinguisticPath;
    String          m_aModulePath;
    String          m_aNewMenuPath;
    String          m_aPalettePath;
    String          m_aPluginPath;
    String          m_aStoragePath;
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

    typedef String SvtPathOptions_Impl:: *StrPtr;

    // not const because of using a mutex
    const String&   GetPath( StrPtr pPtr );
    void            SetPath( StrPtr pPtr, const String& rNewPath );

public:
                    SvtPathOptions_Impl();

    virtual void    Notify( const com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
    virtual void    Commit();

    // get the pathes, not const because of using a mutex
    const String&   GetAddinPath() { return GetPath( &SvtPathOptions_Impl::m_aAddinPath ); }
    const String&   GetAutoCorrectPath() { return GetPath( &SvtPathOptions_Impl::m_aAutoCorrectPath); }
    const String&   GetAutoPilotPath() { return GetPath( &SvtPathOptions_Impl::m_aAutoPilotPath); }
    const String&   GetAutoTextPath() { return GetPath( &SvtPathOptions_Impl::m_aAutoTextPath); }
    const String&   GetBackupPath() { return GetPath( &SvtPathOptions_Impl::m_aBackupPath); }
    const String&   GetBasicPath() { return GetPath( &SvtPathOptions_Impl::m_aBasicPath); }
    const String&   GetBitmapPath() { return GetPath( &SvtPathOptions_Impl::m_aBitmapPath); }
    const String&   GetConfigPath() { return GetPath( &SvtPathOptions_Impl::m_aConfigPath); }
    const String&   GetDatabasePath() { return GetPath( &SvtPathOptions_Impl::m_aDatabasePath); }
    const String&   GetDictionaryPath() { return GetPath( &SvtPathOptions_Impl::m_aDictionaryPath); }
    const String&   GetFavoritesPath() { return GetPath( &SvtPathOptions_Impl::m_aFavoritesPath); }
    const String&   GetFilterPath() { return GetPath( &SvtPathOptions_Impl::m_aFilterPath); }
    const String&   GetGalleryPath() { return GetPath( &SvtPathOptions_Impl::m_aGalleryPath); }
    const String&   GetGraphicPath() { return GetPath( &SvtPathOptions_Impl::m_aGraphicPath); }
    const String&   GetHelpPath() { return GetPath( &SvtPathOptions_Impl::m_aHelpPath); }
    const String&   GetLinguisticPath() { return GetPath( &SvtPathOptions_Impl::m_aLinguisticPath); }
    const String&   GetModulePath() { return GetPath( &SvtPathOptions_Impl::m_aModulePath); }
    const String&   GetNewMenuPath() { return GetPath( &SvtPathOptions_Impl::m_aNewMenuPath); }
    const String&   GetPalettePath() { return GetPath( &SvtPathOptions_Impl::m_aPalettePath); }
    const String&   GetPluginPath() { return GetPath( &SvtPathOptions_Impl::m_aPluginPath); }
    const String&   GetStoragePath() { return GetPath( &SvtPathOptions_Impl::m_aStoragePath); }
    const String&   GetTempPath() { return GetPath( &SvtPathOptions_Impl::m_aTempPath); }
    const String&   GetTemplatePath() { return GetPath( &SvtPathOptions_Impl::m_aTemplatePath); }
    const String&   GetTrashPath() { return GetPath( &SvtPathOptions_Impl::m_aTrashPath); }
    const String&   GetUserConfigPath() { return GetPath( &SvtPathOptions_Impl::m_aUserConfigPath); }
    const String&   GetUserDictionaryPath() { return GetPath( &SvtPathOptions_Impl::m_aUserDictionaryPath ); }
    const String&   GetWorkPath() { return GetPath( &SvtPathOptions_Impl::m_aWorkPath ); }

    // set the pathes
    void            SetAddinPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aAddinPath, rPath ); }
    void            SetAutoCorrectPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aAutoCorrectPath, rPath ); }
    void            SetAutoPilotPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aAutoPilotPath, rPath ); }
    void            SetAutoTextPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aAutoTextPath, rPath ); }
    void            SetBackupPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aBackupPath, rPath ); }
    void            SetBasicPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aBasicPath, rPath ); }
    void            SetBitmapPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aBitmapPath, rPath ); }
    void            SetConfigPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aConfigPath, rPath ); }
    void            SetDatabasePath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aDatabasePath, rPath ); }
    void            SetDictionaryPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aDictionaryPath, rPath ); }
    void            SetFavoritesPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aFavoritesPath, rPath ); }
    void            SetFilterPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aFilterPath, rPath ); }
    void            SetGalleryPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aGalleryPath, rPath ); }
    void            SetGraphicPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aGraphicPath, rPath ); }
    void            SetHelpPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aHelpPath, rPath ); }
    void            SetLinguisticPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aLinguisticPath, rPath ); }
    void            SetModulePath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aModulePath, rPath ); }
    void            SetNewMenuPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aNewMenuPath, rPath ); }
    void            SetPalettePath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aPalettePath, rPath ); }
    void            SetPluginPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aPluginPath, rPath ); }
    void            SetStoragePath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aStoragePath, rPath ); }
    void            SetTempPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aTempPath, rPath ); }
    void            SetTemplatePath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aTemplatePath, rPath ); }
    void            SetTrashPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aTrashPath, rPath ); }
    void            SetUserConfigPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aUserConfigPath, rPath ); }
    void            SetUserDictionaryPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aUserDictionaryPath, rPath ); }
    void            SetWorkPath( const String& rPath ) { SetPath( &SvtPathOptions_Impl::m_aWorkPath, rPath ); }

    rtl::OUString   SubstVar( const rtl::OUString& rVar );
};

// global ----------------------------------------------------------------

static SvtPathOptions_Impl* pOptions = NULL;
static sal_Int32            nRefCount = 0;
static ::osl::Mutex aMutex;

// functions -------------------------------------------------------------

Sequence< OUString > GetPathPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Addin",            // PATH_ADDIN
        "AutoCorrect",      // PATH_AUTOCORRECT
        "AutoPilot",        // PATH_AUTOPILOT
        "AutoText",         // PATH_AUTOTEXT
        "Backup",           // PATH_BACKUP
        "Basic",            // PATH_BASIC
        "Bitmap",           // PATH_BITMAP
        "Config",           // PATH_CONFIG
        "Database",         // PATH_DATABASE
        "Dictionary",       // PATH_DICTIONARY
        "Favorite",         // PATH_FAVORITES
        "Filter",           // PATH_FILTER
        "Gallery",          // PATH_GALLERY
        "Graphic",          // PATH_GRAPHIC
        "Help",             // PATH_HELP
        "Linguistic",       // PATH_LINGUISTIC
        "Module",           // PATH_MODULE
        "New",              // PATH_NEWMENU
        "Palette",          // PATH_PALETTE
        "Plugin",           // PATH_PLUGIN
        "Storage",          // PATH_STORAGE
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

const String& SvtPathOptions_Impl::GetPath( StrPtr pPtr )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return this->*pPtr;
}

// -----------------------------------------------------------------------

void SvtPathOptions_Impl::SetPath( StrPtr pPtr, const String& rNewPath )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    this->*pPtr = rNewPath;
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
        aSubString = aSubString.toLowerCase();
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
        if ( SUBSTITUTE_USER == aSubString || SUBSTITUTE_USERPATH == aSubString )
        {
            nReplaceLength = REPLACELENGTH_USER;
            aReplacement = m_aUserURL;
            bConvertToPath = sal_True;
        }
        else
        // -------------------------------------------------------------------------------------------------------------------
        // $(prog) - directory of the executable file
        if ( SUBSTITUTE_PROG == aSubString || SUBSTITUTE_PROGPATH == aSubString )
        {
            nReplaceLength = REPLACELENGTH_PROG;
            aReplacement = m_aProgURL;
            bConvertToPath = sal_True;
        }
        else
        // -------------------------------------------------------------------------------------------------------------------
        // $(insturl) - directory of the master (server) installation as URL
        if ( SUBSTITUTE_INSTURL == aSubString || SUBSTITUTE_INSTPATH == aSubString )
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
/*
        // -------------------------------------------------------------------------------------------------------------------
        // $(syslangid) - LanguageType of the system as string (for example "1031")
        if ( SUBSTITUTE_SYSLANGID == aSubString )
        {
            nReplaceLength = REPLACELENGTH_SYSLANGID;
            aReplacement = OUString::valueOf( (sal_uInt16)System::GetLanguage() );
        }
        else
*/
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
    ConfigManager* pCfgMgr = ConfigManager::GetConfigManager();
    Any aAny = pCfgMgr->GetDirectConfigProperty( ConfigManager::OFFICEINSTALL );
    OUString aOfficePath;
    if ( aAny >>= aOfficePath )
        m_aInstPath = aOfficePath;
    else
        DBG_ERRORFILE( "wrong any type" );

    aAny = pCfgMgr->GetDirectConfigProperty( ConfigManager::OFFICEINSTALLURL );
    if ( aAny >>= aOfficePath )
    {
        m_aInstURL = aOfficePath;
        if ( !m_aInstURL.Len() )
            ::utl::LocalFileHelper::ConvertPhysicalNameToURL( m_aInstPath, m_aInstURL );
    }
    else if ( !aAny.hasValue() )
    {
        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( m_aInstPath, m_aInstURL );
    }
    else
        DBG_ERRORFILE( "wrong any type" );

    aAny = pCfgMgr->GetDirectConfigProperty( ConfigManager::INSTALLPATH );
    OUString aUserPath;
    if ( aAny >>= aUserPath )
        m_aUserPath = aUserPath;
    else
        DBG_ERRORFILE( "wrong any type" );

    aAny = pCfgMgr->GetDirectConfigProperty( ConfigManager::USERINSTALLURL );
    if ( aAny >>= aUserPath )
    {
        m_aUserURL = aUserPath;
        if ( !m_aUserURL.Len() )
            ::utl::LocalFileHelper::ConvertPhysicalNameToURL( m_aUserPath, m_aUserURL );
    }
    else if ( !aAny.hasValue() )
    {
        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( m_aUserPath, m_aUserURL );
    }
    else
        DBG_ERRORFILE( "wrong any type" );

    OUString aProgName;
    ::vos::OStartupInfo aInfo;
    aInfo.getExecutableFile( aProgName );
    sal_Int32 lastIndex = aProgName.lastIndexOf('/');
    if ( lastIndex )
    {
        m_aProgPath = aProgName.copy(0, lastIndex + 1);
        OUString aTmp;
        FileBase::getSystemPathFromNormalizedPath( m_aProgPath, aTmp );
        m_aProgPath = aTmp;
        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( m_aProgPath, m_aProgURL );
    }

    m_eLanguageType = LANGUAGE_ENGLISH_US;
    Any aLocale = ConfigManager::GetConfigManager()->GetDirectConfigProperty( ConfigManager::LOCALE );
    OUString aLocaleStr;
    if ( aLocale >>= aLocaleStr )
#if SUPD < 613
        m_eLanguageType = ConvertIsoStringToLanguage( aLocaleStr, '_' );
#else
        m_eLanguageType = ConvertIsoStringToLanguage( aLocaleStr );
#endif
    else
    {
        DBG_ERRORFILE( "wrong any type" );
    }

    Sequence< OUString > aNames = GetPathPropertyNames();
    Sequence< Any > aValues = GetProperties( aNames );
    EnableNotification( aNames );
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT( aValues.getLength() == aNames.getLength(), "GetProperties failed" );
    if ( aValues.getLength() == aNames.getLength() )
    {
        OUString aTempStr, aFullPath;

        for ( int nProp = 0; nProp < aNames.getLength(); nProp++ )
        {
            if ( pValues[nProp].hasValue() )
            {
                switch ( pValues[nProp].getValueTypeClass() )
                {
                    case ::com::sun::star::uno::TypeClass_STRING :
                    {
                        // multi pathes
                        if ( pValues[nProp] >>= aTempStr )
                            aFullPath = SubstVar( aTempStr );
                        else
                        {
                            DBG_ERRORFILE( "any operator >>= failed" );
                        }
                        break;
                    }

                    case ::com::sun::star::uno::TypeClass_SEQUENCE :
                    {
                        // single pathes
                        aFullPath = OUString();
                        Sequence < OUString > aList;
                        if ( pValues[nProp] >>= aList )
                        {
                            sal_Int32 nCount = aList.getLength();
                            for ( sal_Int32 nPosition = 0; nPosition < nCount; ++nPosition )
                            {
                                aTempStr = SubstVar( aList[ nPosition ] );
                                aFullPath += aTempStr;
                                if ( nPosition < nCount-1 )
                                    aFullPath += OUString( RTL_CONSTASCII_USTRINGPARAM(";") );
                            }
                        }
                        else
                        {
                            DBG_ERRORFILE( "any operator >>= failed" );
                        }
                        break;
                    }

                    default:
                    {
                        DBG_ERRORFILE( "Wrong any type" );
                    }
                }

                switch ( nProp )
                {
                    case SvtPathOptions::PATH_ADDIN:        m_aAddinPath = String( aFullPath );         break;
                    case SvtPathOptions::PATH_AUTOCORRECT:  m_aAutoCorrectPath = String( aFullPath );   break;
                    case SvtPathOptions::PATH_AUTOPILOT:    m_aAutoPilotPath = String( aFullPath );     break;
                    case SvtPathOptions::PATH_AUTOTEXT:     m_aAutoTextPath = String( aFullPath );      break;
                    case SvtPathOptions::PATH_BACKUP:       m_aBackupPath = String( aFullPath );        break;
                    case SvtPathOptions::PATH_BASIC:        m_aBasicPath = String( aFullPath );         break;
                    case SvtPathOptions::PATH_BITMAP:       m_aBitmapPath = String( aFullPath );        break;
                    case SvtPathOptions::PATH_CONFIG:       m_aConfigPath = String( aFullPath );        break;
                    case SvtPathOptions::PATH_DATABASE:     m_aDatabasePath = String( aFullPath );      break;
                    case SvtPathOptions::PATH_DICTIONARY:   m_aDictionaryPath = String( aFullPath );    break;
                    case SvtPathOptions::PATH_FAVORITES:    m_aFavoritesPath = String( aFullPath );     break;
                    case SvtPathOptions::PATH_FILTER:       m_aFilterPath = String( aFullPath );        break;
                    case SvtPathOptions::PATH_GALLERY:      m_aGalleryPath = String( aFullPath );       break;
                    case SvtPathOptions::PATH_GRAPHIC:      m_aGraphicPath = String( aFullPath );       break;
                    case SvtPathOptions::PATH_HELP:         m_aHelpPath = String( aFullPath );          break;
                    case SvtPathOptions::PATH_LINGUISTIC:   m_aLinguisticPath = String( aFullPath );    break;
                    case SvtPathOptions::PATH_MODULE:       m_aModulePath = String( aFullPath );        break;
                    case SvtPathOptions::PATH_NEWMENU:      m_aNewMenuPath = String( aFullPath );       break;
                    case SvtPathOptions::PATH_PALETTE:      m_aPalettePath = String( aFullPath );       break;
                    case SvtPathOptions::PATH_PLUGIN:       m_aPluginPath = String( aFullPath );        break;
                    case SvtPathOptions::PATH_STORAGE:      m_aStoragePath = String( aFullPath );       break;
                    case SvtPathOptions::PATH_TEMP:         m_aTempPath = String( aFullPath );          break;
                    case SvtPathOptions::PATH_TEMPLATE:     m_aTemplatePath = String( aFullPath );      break;
                    case SvtPathOptions::PATH_TRASH:        m_aTrashPath = String( aFullPath );         break;
                    case SvtPathOptions::PATH_USERCONFIG:   m_aUserConfigPath = String( aFullPath );    break;
                    case SvtPathOptions::PATH_USERDICTIONARY: m_aUserDictionaryPath = String( aFullPath );break;
                    case SvtPathOptions::PATH_WORK:         m_aWorkPath = String( aFullPath );          break;

                    default:
                        DBG_ERRORFILE( "invalid index to load a path" );
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
        sal_Bool bList = sal_False;

        switch ( nProp )
        {
            // multi pathes
            case SvtPathOptions::PATH_AUTOCORRECT:  aTempStr = OUString( m_aAutoCorrectPath );  bList = sal_True; break;
            case SvtPathOptions::PATH_AUTOTEXT:     aTempStr = OUString( m_aAutoTextPath );     bList = sal_True; break;
            case SvtPathOptions::PATH_BASIC:        aTempStr = OUString( m_aBasicPath );        bList = sal_True; break;
            case SvtPathOptions::PATH_GALLERY:      aTempStr = OUString( m_aGalleryPath );      bList = sal_True; break;
            case SvtPathOptions::PATH_PLUGIN:       aTempStr = OUString( m_aPluginPath );       bList = sal_True; break;
            case SvtPathOptions::PATH_TEMPLATE:     aTempStr = OUString( m_aTemplatePath );     bList = sal_True; break;

            // single pathes
            case SvtPathOptions::PATH_ADDIN:            aTempStr = OUString( m_aAddinPath );            break;
            case SvtPathOptions::PATH_AUTOPILOT:        aTempStr = OUString( m_aAutoPilotPath );        break;
            case SvtPathOptions::PATH_BACKUP:           aTempStr = OUString( m_aBackupPath );           break;
            case SvtPathOptions::PATH_BITMAP:           aTempStr = OUString( m_aBitmapPath );           break;
            case SvtPathOptions::PATH_CONFIG:           aTempStr = OUString( m_aConfigPath );           break;
            case SvtPathOptions::PATH_DATABASE:         aTempStr = OUString( m_aDatabasePath );         break;
            case SvtPathOptions::PATH_DICTIONARY:       aTempStr = OUString( m_aDictionaryPath );       break;
            case SvtPathOptions::PATH_FAVORITES:        aTempStr = OUString( m_aFavoritesPath );        break;
            case SvtPathOptions::PATH_FILTER:           aTempStr = OUString( m_aFilterPath );           break;
            case SvtPathOptions::PATH_GRAPHIC:          aTempStr = OUString( m_aGraphicPath );          break;
            case SvtPathOptions::PATH_HELP:             aTempStr = OUString( m_aHelpPath );             break;
            case SvtPathOptions::PATH_LINGUISTIC:       aTempStr = OUString( m_aLinguisticPath );       break;
            case SvtPathOptions::PATH_MODULE:           aTempStr = OUString( m_aModulePath );           break;
            case SvtPathOptions::PATH_NEWMENU:          aTempStr = OUString( m_aNewMenuPath );          break;
            case SvtPathOptions::PATH_PALETTE:          aTempStr = OUString( m_aPalettePath );          break;
            case SvtPathOptions::PATH_STORAGE:          aTempStr = OUString( m_aStoragePath );          break;
            case SvtPathOptions::PATH_TEMP:             aTempStr = OUString( m_aTempPath );             break;
            case SvtPathOptions::PATH_TRASH:            aTempStr = OUString( m_aTrashPath );            break;
            case SvtPathOptions::PATH_USERCONFIG:       aTempStr = OUString( m_aUserConfigPath );       break;
            case SvtPathOptions::PATH_USERDICTIONARY:   aTempStr = OUString( m_aUserDictionaryPath );   break;
            case SvtPathOptions::PATH_WORK:             aTempStr = OUString( m_aWorkPath );             break;

            default:
                DBG_ERRORFILE( "invalid index to save a path" );
        }

        if ( bList )
        {
            String aFullPath( aTempStr );
            USHORT nCount = aFullPath.GetTokenCount(), nIdx = 0;
            sal_Int32 nPos = 0;
            Sequence < OUString > aList( nCount );
            while ( STRING_NOTFOUND != nIdx )
                aList[nPos++] = OUString( aFullPath.GetToken( 0, ';', nIdx ) );
            pValues[nProp] <<= aList;
        }
        else
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
        if ( pOptions->IsModified() )
            pOptions->Commit();
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

const String& SvtPathOptions::GetAutoPilotPath() const
{
    return pImp->GetAutoPilotPath();
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

// -----------------------------------------------------------------------

String SvtPathOptions::SubstituteVariable( const String& rVar )
{
    String aRet = pImp->SubstVar( rVar );
    return aRet;
}

// -----------------------------------------------------------------------

BOOL IniFileExists_Impl( const String rURL )
{
    INetURLObject aObj( rURL, INET_PROT_FILE );
    rtl::OUString aTmp( aObj.GetMainURL() );
    rtl::OUString aResult;
    if ( FileBase::getNormalizedPathFromFileURL( aTmp, aResult )  == FileBase::E_None )
    {
        FileBase::RC err = Directory::create( aResult );
        if ( err == FileBase::E_EXIST )
            return sal_True;
        else
            Directory::remove( aResult );
    }

    return sal_False;
}

sal_Bool SvtPathOptions::SearchFile( String& rIniFile, Pathes ePath )
{
    // check parameter: empty inifile name?
    if ( !rIniFile.Len() )
    {
        DBG_ERRORFILE( "SvtPathOptions::SearchFile(): invalid parameter" );
        return sal_False;
    }

    String aIniFile = SubstituteVariable( rIniFile );
    sal_Bool bRet = sal_False;

    switch ( ePath )
    {
        case PATH_USERCONFIG:
        case PATH_USERDICTIONARY:
        {
            sal_Bool bCfg = ( PATH_USERCONFIG == ePath );
            bRet = sal_True;
            INetURLObject aObj( bCfg ? GetUserConfigPath() : GetUserDictionaryPath(), INET_PROT_FILE );
            aObj.insertName( aIniFile );
            if ( !IniFileExists_Impl( aObj.GetMainURL() ) )
            {
                aObj.SetSmartURL( bCfg ? GetConfigPath() : GetDictionaryPath() );
                aObj.insertName( aIniFile );
                bRet = IniFileExists_Impl( aObj.GetMainURL() );
            }

            if ( bRet )
                rIniFile = aObj.PathToFileName();

            break;
        }

        default:
        {
            String aPath;
            switch ( ePath )
            {
                case PATH_ADDIN:        aPath = GetAddinPath();         break;
                case PATH_AUTOCORRECT:  aPath = GetAutoCorrectPath();   break;
                case PATH_AUTOPILOT:    aPath = GetAutoPilotPath();     break;
                case PATH_AUTOTEXT:     aPath = GetAutoTextPath();      break;
                case PATH_BACKUP:       aPath = GetBackupPath();        break;
                case PATH_BASIC:        aPath = GetBasicPath();         break;
                case PATH_BITMAP:       aPath = GetBitmapPath();        break;
                case PATH_CONFIG:       aPath = GetConfigPath();        break;
                case PATH_DATABASE:     aPath = GetDatabasePath();      break;
                case PATH_DICTIONARY:   aPath = GetDictionaryPath();    break;
                case PATH_FAVORITES:    aPath = GetFavoritesPath();     break;
                case PATH_FILTER:       aPath = GetFilterPath();        break;
                case PATH_GALLERY:      aPath = GetGalleryPath();       break;
                case PATH_GRAPHIC:      aPath = GetGraphicPath();       break;
                case PATH_HELP:         aPath = GetHelpPath();          break;
                case PATH_LINGUISTIC:   aPath = GetLinguisticPath();    break;
                case PATH_MODULE:       aPath = GetModulePath();        break;
                case PATH_NEWMENU:      aPath = GetNewMenuPath();       break;
                case PATH_PALETTE:      aPath = GetPalettePath();       break;
                case PATH_PLUGIN:       aPath = GetPluginPath();        break;
                case PATH_STORAGE:      aPath = GetStoragePath();       break;
                case PATH_TEMP:         aPath = GetTempPath();          break;
                case PATH_TEMPLATE:     aPath = GetTemplatePath();      break;
                case PATH_TRASH:        aPath = GetTrashPath();         break;
                case PATH_WORK:         aPath = GetWorkPath();          break;
            }
            sal_uInt16 i, nIdx = 0, nCount = aPath.GetTokenCount( SEARCHPATH_DELIMITER );
            for ( i = 0; i < nCount; ++i )
            {
                INetURLObject aObj;
                aObj.SetSmartProtocol( INET_PROT_FILE );
                aObj.SetSmartURL( aPath.GetToken( i, SEARCHPATH_DELIMITER, nIdx ) );
                aObj.insertName( aIniFile );
                bRet = IniFileExists_Impl( aObj.GetMainURL() );

                if ( bRet )
                {
                    rIniFile = aObj.PathToFileName();
                    break;
                }
            }
        }
    }

    return bRet;
}

void SAL_CALL PathService::addPropertyChangeListener( const ::rtl::OUString& sKeyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException) {}
void SAL_CALL PathService::removePropertyChangeListener( const ::rtl::OUString& sKeyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException) {}
void SAL_CALL PathService::flush(  ) throw(::com::sun::star::uno::RuntimeException) {}

::rtl::OUString SAL_CALL PathService::substituteVariables( const ::rtl::OUString& sText ) throw(::com::sun::star::uno::RuntimeException)
{
    return SvtPathOptions().SubstituteVariable( sText );
}

::rtl::OUString SAL_CALL PathService::getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.svtools.PathService");
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
    Sequence< ::rtl::OUString > aRet(1);
    *aRet.getArray() = ::rtl::OUString::createFromAscii("com.sun.star.config.SpecialConfigManager");
    return aRet;
}

