/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: printerinfomanager.cxx,v $
 * $Revision: 1.49 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_psprint.hxx"

#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/config.hxx>
#include <cupsmgr.hxx>
#include <psprint/fontmanager.hxx>
#include <psprint/strhelper.hxx>
#include <rtl/strbuf.hxx>

#include <osl/thread.hxx>
#include <osl/mutex.hxx>
#include <osl/process.h>

#ifdef MACOSX
#include <sys/stat.h>
#endif

// filename of configuration files
#define PRINT_FILENAME  "psprint.conf"
// the group of the global defaults
#define GLOBAL_DEFAULTS_GROUP "__Global_Printer_Defaults__"

#include <hash_set>

using namespace psp;
using namespace rtl;
using namespace osl;

namespace psp
{
    class SystemQueueInfo : public Thread
    {
        mutable Mutex               m_aMutex;
        bool                        m_bChanged;
        std::list< PrinterInfoManager::SystemPrintQueue >
                                    m_aQueues;
        OUString                    m_aCommand;

        virtual void run();

        public:
        SystemQueueInfo();
        ~SystemQueueInfo();

        bool hasChanged() const;
        OUString getCommand() const;

        // sets changed status to false; therefore not const
        void getSystemQueues( std::list< PrinterInfoManager::SystemPrintQueue >& rQueues );
    };
} // namespace

/*
*  class PrinterInfoManager
*/

// -----------------------------------------------------------------

PrinterInfoManager& PrinterInfoManager::get()
{
    static PrinterInfoManager* pManager = NULL;

    if( ! pManager )
    {
        pManager = CUPSManager::tryLoadCUPS();
        if( ! pManager )
            pManager = new PrinterInfoManager();

        if( pManager )
            pManager->initialize();
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "PrinterInfoManager::get create Manager of type %d\n", pManager->getType() );
        #endif
    }

    return *pManager;
}

// -----------------------------------------------------------------

PrinterInfoManager::PrinterInfoManager( Type eType ) :
    m_pQueueInfo( NULL ),
    m_eType( eType ),
    m_bUseIncludeFeature( false ),
    m_aSystemDefaultPaper( RTL_CONSTASCII_USTRINGPARAM( "A4" ) ),
    m_bDisableCUPS( false )
{
    if( eType == Default )
        m_pQueueInfo = new SystemQueueInfo();
    initSystemDefaultPaper();
}

// -----------------------------------------------------------------

PrinterInfoManager::~PrinterInfoManager()
{
    delete m_pQueueInfo;
}

// -----------------------------------------------------------------

bool PrinterInfoManager::isCUPSDisabled() const
{
    return m_bDisableCUPS;
}

// -----------------------------------------------------------------

void PrinterInfoManager::setCUPSDisabled( bool bDisable )
{
    m_bDisableCUPS = bDisable;
    writePrinterConfig();
    // actually we know the printers changed
    // however this triggers reinitialization the right way
    checkPrintersChanged( true );
}

// -----------------------------------------------------------------

void PrinterInfoManager::initSystemDefaultPaper()
{
    bool bSuccess = false;

    // try libpaper
    #ifdef SOLARIS
    // #i78617# workaround missing paperconf command; on e.g. Linux
    // the 2>/dev/null works on the started shell also
    FILE* pPipe = popen( "sh -c paperconf 2>/dev/null", "r" );
    #else
    FILE* pPipe = popen( "paperconf 2>/dev/null", "r" );
    #endif
    if( pPipe )
    {
        char pBuffer[ 1024 ];
        *pBuffer = 0;
        fgets( pBuffer, sizeof(pBuffer)-1, pPipe );
        pclose( pPipe );

        ByteString aPaper( pBuffer );
        aPaper = WhitespaceToSpace( aPaper );
        if( aPaper.Len() )
        {
            m_aSystemDefaultPaper = OUString( OStringToOUString( aPaper, osl_getThreadTextEncoding() ) );
            bSuccess = true;
            #if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "paper from paperconf = %s\n", aPaper.GetBuffer() );
            #endif
        }
        if( bSuccess )
            return;
    }

    // default value is Letter for US (en_US), Cannada (en_CA, fr_CA); else A4
    // en will be interpreted as en_US

    // note: at this point m_aSystemDefaultPaper is set to "A4" from the constructor

    // check for LC_PAPER
    const char* pPaperLang = getenv( "LC_PAPER" );
    if( pPaperLang && *pPaperLang )
    {
        OString aLang( pPaperLang );
        if( aLang.getLength() > 5 )
            aLang = aLang.copy( 0, 5 );
        if( aLang.getLength() == 5 )
        {
            if(    aLang.equalsIgnoreAsciiCase( "en_us" )
                || aLang.equalsIgnoreAsciiCase( "en_ca" )
            || aLang.equalsIgnoreAsciiCase( "fr_ca" )
            )
                m_aSystemDefaultPaper = OUString( RTL_CONSTASCII_USTRINGPARAM( "Letter" ) );
        }
        else if( aLang.getLength() == 2 && aLang.equalsIgnoreAsciiCase( "en" ) )
            m_aSystemDefaultPaper = OUString( RTL_CONSTASCII_USTRINGPARAM( "Letter" ) );
        return;
    }

    // use process locale to determine paper
    rtl_Locale* pLoc = NULL;
    osl_getProcessLocale( &pLoc );
    if( pLoc )
    {
        if( 0 == rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( pLoc->Language->buffer, pLoc->Language->length, "en") )
        {
            if(    0 == rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( pLoc->Country->buffer, pLoc->Country->length, "us")
                || 0 == rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( pLoc->Country->buffer, pLoc->Country->length, "ca")
                || pLoc->Country->length == 0
                )
                m_aSystemDefaultPaper = OUString( RTL_CONSTASCII_USTRINGPARAM( "Letter" ) );
        }
        else if( 0 == rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( pLoc->Language->buffer, pLoc->Language->length, "fr") )
        {
            if( 0 == rtl_ustr_ascii_compareIgnoreAsciiCase_WithLength( pLoc->Country->buffer, pLoc->Country->length, "ca") )
                m_aSystemDefaultPaper = OUString( RTL_CONSTASCII_USTRINGPARAM( "Letter" ) );
        }
    }
}

// -----------------------------------------------------------------

bool PrinterInfoManager::checkPrintersChanged( bool bWait )
{
    // check if files were created, deleted or modified since initialize()
    ::std::list< WatchFile >::const_iterator it;
    bool bChanged = false;
    for( it = m_aWatchFiles.begin(); it != m_aWatchFiles.end() && ! bChanged; ++it )
    {
        DirectoryItem aItem;
        if( DirectoryItem::get( it->m_aFilePath, aItem ) )
        {
            if( it->m_aModified.Seconds != 0 )
                bChanged = true; // file probably has vanished
        }
        else
        {
            FileStatus aStatus( FileStatusMask_ModifyTime );
            if( aItem.getFileStatus( aStatus ) )
                bChanged = true; // unlikely but not impossible
            else
            {
                TimeValue aModified = aStatus.getModifyTime();
                if( aModified.Seconds != it->m_aModified.Seconds )
                    bChanged = true;
            }
        }
    }

    if( bWait && m_pQueueInfo )
    {
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "syncing printer discovery thread\n" );
        #endif
        m_pQueueInfo->join();
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "done: syncing printer discovery thread\n" );
        #endif
    }

    if( ! bChanged && m_pQueueInfo )
        bChanged = m_pQueueInfo->hasChanged();
    if( bChanged )
    {
        initialize();
    }

    return bChanged;
}

// -----------------------------------------------------------------

void PrinterInfoManager::initialize()
{
    m_bUseIncludeFeature = false;
    rtl_TextEncoding aEncoding = gsl_getSystemTextEncoding();
    m_aPrinters.clear();
    m_aWatchFiles.clear();
    OUString aDefaultPrinter;

    // first initialize the global defaults
    // have to iterate over all possible files
    // there should be only one global setup section in all
    // available config files
    m_aGlobalDefaults = PrinterInfo();

    // need a parser for the PPDContext. generic printer should do.
    m_aGlobalDefaults.m_pParser = PPDParser::getParser( String( RTL_CONSTASCII_USTRINGPARAM( "SGENPRT" ) ) );
    m_aGlobalDefaults.m_aContext.setParser( m_aGlobalDefaults.m_pParser );
    m_aGlobalDefaults.m_bPerformFontSubstitution = true;
    m_bDisableCUPS = false;

    if( ! m_aGlobalDefaults.m_pParser )
    {
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "Error: no default PPD file SGENPRT available, shutting down psprint...\n" );
        #endif
        return;
    }

    std::list< OUString > aDirList;
    psp::getPrinterPathList( aDirList, NULL );
    std::list< OUString >::const_iterator print_dir_it;
    for( print_dir_it = aDirList.begin(); print_dir_it != aDirList.end(); ++print_dir_it )
    {
        INetURLObject aFile( *print_dir_it, INET_PROT_FILE, INetURLObject::ENCODE_ALL );
        aFile.Append( String( RTL_CONSTASCII_USTRINGPARAM( PRINT_FILENAME ) ) );
        Config aConfig( aFile.PathToFileName() );
        if( aConfig.HasGroup( GLOBAL_DEFAULTS_GROUP ) )
        {
            #if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "found global defaults in %s\n", OUStringToOString( aFile.PathToFileName(), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
            #endif
            aConfig.SetGroup( GLOBAL_DEFAULTS_GROUP );

            ByteString aValue( aConfig.ReadKey( "Copies" ) );
            if( aValue.Len() )
                m_aGlobalDefaults.m_nCopies = aValue.ToInt32();

            aValue = aConfig.ReadKey( "Orientation" );
            if( aValue.Len() )
                m_aGlobalDefaults.m_eOrientation = aValue.EqualsIgnoreCaseAscii( "Landscape" ) ? orientation::Landscape : orientation::Portrait;

            aValue = aConfig.ReadKey( "MarginAdjust" );
            if( aValue.Len() )
            {
                m_aGlobalDefaults.m_nLeftMarginAdjust   = aValue.GetToken( 0, ',' ).ToInt32();
                m_aGlobalDefaults.m_nRightMarginAdjust  = aValue.GetToken( 1, ',' ).ToInt32();
                m_aGlobalDefaults.m_nTopMarginAdjust    = aValue.GetToken( 2, ',' ).ToInt32();
                m_aGlobalDefaults.m_nBottomMarginAdjust = aValue.GetToken( 3, ',' ).ToInt32();
            }

            aValue = aConfig.ReadKey( "ColorDepth", "24" );
            if( aValue.Len() )
                m_aGlobalDefaults.m_nColorDepth = aValue.ToInt32();

            aValue = aConfig.ReadKey( "ColorDevice" );
            if( aValue.Len() )
                m_aGlobalDefaults.m_nColorDevice = aValue.ToInt32();

            aValue = aConfig.ReadKey( "PSLevel" );
            if( aValue.Len() )
                m_aGlobalDefaults.m_nPSLevel = aValue.ToInt32();

            aValue = aConfig.ReadKey( "PerformFontSubstitution" );
            if( aValue.Len() )
            {
                if( ! aValue.Equals( "0" ) && ! aValue.EqualsIgnoreCaseAscii( "false" ) )
                    m_aGlobalDefaults.m_bPerformFontSubstitution = true;
                else
                    m_aGlobalDefaults.m_bPerformFontSubstitution = false;
            }

            aValue = aConfig.ReadKey( "DisableCUPS" );
            if( aValue.Len() )
            {
                if( aValue.Equals( "1" ) || aValue.EqualsIgnoreCaseAscii( "true" ) )
                    m_bDisableCUPS = true;
                else
                    m_bDisableCUPS = false;
            }

            // get the PPDContext of global JobData
            for( int nKey = 0; nKey < aConfig.GetKeyCount(); nKey++ )
            {
                ByteString aKey( aConfig.GetKeyName( nKey ) );
                if( aKey.CompareTo( "PPD_", 4 ) == COMPARE_EQUAL )
                {
                    aValue = aConfig.ReadKey( aKey );
                    const PPDKey* pKey = m_aGlobalDefaults.m_pParser->getKey( String( aKey.Copy( 4 ), RTL_TEXTENCODING_ISO_8859_1 ) );
                    if( pKey )
                    {
                        m_aGlobalDefaults.m_aContext.
                        setValue( pKey,
                        aValue.Equals( "*nil" ) ? NULL : pKey->getValue( String( aValue, RTL_TEXTENCODING_ISO_8859_1 ) ),
                        TRUE );
                    }
                }
                else if( aKey.Len() > 10 && aKey.CompareTo("SubstFont_", 10 ) == COMPARE_EQUAL )
                {
                    aValue = aConfig.ReadKey( aKey );
                    m_aGlobalDefaults.m_aFontSubstitutes[ OStringToOUString( aKey.Copy( 10 ), RTL_TEXTENCODING_ISO_8859_1 ) ] = OStringToOUString( aValue, RTL_TEXTENCODING_ISO_8859_1 );
                }
            }
            #if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "global settings: fontsubst = %s, %d substitutes\n", m_aGlobalDefaults.m_bPerformFontSubstitution ? "true" : "false", m_aGlobalDefaults.m_aFontSubstitutes.size() );
            #endif
        }
    }
    setDefaultPaper( m_aGlobalDefaults.m_aContext );
    fillFontSubstitutions( m_aGlobalDefaults );

    // now collect all available printers
    for( print_dir_it = aDirList.begin(); print_dir_it != aDirList.end(); ++print_dir_it )
    {
        INetURLObject aDir( *print_dir_it, INET_PROT_FILE, INetURLObject::ENCODE_ALL );
        INetURLObject aFile( aDir );
        aFile.Append( String( RTL_CONSTASCII_USTRINGPARAM( PRINT_FILENAME ) ) );

        // check directory validity
        OUString aUniPath;
        FileBase::getFileURLFromSystemPath( aDir.PathToFileName(), aUniPath );
        Directory aDirectory( aUniPath );
        if( aDirectory.open() )
            continue;
        aDirectory.close();


        FileBase::getFileURLFromSystemPath( aFile.PathToFileName(), aUniPath );
        FileStatus aStatus( FileStatusMask_ModifyTime );
        DirectoryItem aItem;

        // setup WatchFile list
        WatchFile aWatchFile;
        aWatchFile.m_aFilePath = aUniPath;
        if( ! DirectoryItem::get( aUniPath, aItem ) &&
            ! aItem.getFileStatus( aStatus ) )
        {
            aWatchFile.m_aModified = aStatus.getModifyTime();
        }
        else
        {
            aWatchFile.m_aModified.Seconds = 0;
            aWatchFile.m_aModified.Nanosec = 0;
        }
        m_aWatchFiles.push_back( aWatchFile );

        Config aConfig( aFile.PathToFileName() );
        for( int nGroup = 0; nGroup < aConfig.GetGroupCount(); nGroup++ )
        {
            aConfig.SetGroup( aConfig.GetGroupName( nGroup ) );
            ByteString aValue = aConfig.ReadKey( "Printer" );
            if( aValue.Len() )
            {
                OUString aPrinterName;

                int nNamePos = aValue.Search( '/' );
                // check for valid value of "Printer"
                if( nNamePos == STRING_NOTFOUND )
                    continue;

                Printer aPrinter;
                // initialize to global defaults
                aPrinter.m_aInfo = m_aGlobalDefaults;
                // global settings do not default the printer substitution
                // list ! the substitution list in there is only used for
                // newly created printers
                aPrinter.m_aInfo.m_aFontSubstitutes.clear();
                aPrinter.m_aInfo.m_aFontSubstitutions.clear();

                aPrinterName = String( aValue.Copy( nNamePos+1 ), RTL_TEXTENCODING_UTF8 );
                aPrinter.m_aInfo.m_aPrinterName     = aPrinterName;
                aPrinter.m_aInfo.m_aDriverName      = String( aValue.Copy( 0, nNamePos ), RTL_TEXTENCODING_UTF8 );

                // set parser, merge settings
                // don't do this for CUPS printers as this is done
                // by the CUPS system itself
                if( aPrinter.m_aInfo.m_aDriverName.compareToAscii( "CUPS:", 5 ) != 0 )
                {
                    aPrinter.m_aInfo.m_pParser          = PPDParser::getParser( aPrinter.m_aInfo.m_aDriverName );
                    aPrinter.m_aInfo.m_aContext.setParser( aPrinter.m_aInfo.m_pParser );
                    // note: setParser also purges the context

                    // ignore this printer if its driver is not found
                    if( ! aPrinter.m_aInfo.m_pParser )
                        continue;

                    // merge the ppd context keys if the printer has the same keys and values
                    // this is a bit tricky, since it involves mixing two PPDs
                    // without constraints which might end up badly
                    // this feature should be use with caution
                    // it is mainly to select default paper sizes for new printers
                    for( int nPPDValueModified = 0; nPPDValueModified < m_aGlobalDefaults.m_aContext.countValuesModified(); nPPDValueModified++ )
                    {
                        const PPDKey* pDefKey = m_aGlobalDefaults.m_aContext.getModifiedKey( nPPDValueModified );
                        const PPDValue* pDefValue = m_aGlobalDefaults.m_aContext.getValue( pDefKey );
                        const PPDKey* pPrinterKey = pDefKey ? aPrinter.m_aInfo.m_pParser->getKey( pDefKey->getKey() ) : NULL;
                        if( pDefKey && pPrinterKey )
                            // at least the options exist in both PPDs
                        {
                            if( pDefValue )
                            {
                                const PPDValue* pPrinterValue = pPrinterKey->getValue( pDefValue->m_aOption );
                                if( pPrinterValue )
                                    // the printer has a corresponding option for the key
                                aPrinter.m_aInfo.m_aContext.setValue( pPrinterKey, pPrinterValue );
                            }
                            else
                                aPrinter.m_aInfo.m_aContext.setValue( pPrinterKey, NULL );
                        }
                    }

                    aValue = aConfig.ReadKey( "Command" );
                    // no printer without a command
                    if( ! aValue.Len() )
                    {
                        /*  TODO:
                        *  porters: please append your platform to the Solaris
                        *  case if your platform has SystemV printing per default.
                        */
                        #if defined SOLARIS || defined(IRIX)
                        aValue = "lp";
                        #else
                        aValue = "lpr";
                        #endif
                    }
                    aPrinter.m_aInfo.m_aCommand = String( aValue, RTL_TEXTENCODING_UTF8 );
                }

                aValue = aConfig.ReadKey( "QuickCommand" );
                aPrinter.m_aInfo.m_aQuickCommand = String( aValue, RTL_TEXTENCODING_UTF8 );

                aValue = aConfig.ReadKey( "Features" );
                aPrinter.m_aInfo.m_aFeatures = String( aValue, RTL_TEXTENCODING_UTF8 );

                // override the settings in m_aGlobalDefaults if keys exist
                aValue = aConfig.ReadKey( "DefaultPrinter" );
                if( ! aValue.Equals( "0" ) && ! aValue.EqualsIgnoreCaseAscii( "false" ) )
                    aDefaultPrinter = aPrinterName;

                aValue = aConfig.ReadKey( "Location" );
                aPrinter.m_aInfo.m_aLocation = String( aValue, RTL_TEXTENCODING_UTF8 );

                aValue = aConfig.ReadKey( "Comment" );
                aPrinter.m_aInfo.m_aComment = String( aValue, RTL_TEXTENCODING_UTF8 );

                aValue = aConfig.ReadKey( "Copies" );
                if( aValue.Len() )
                    aPrinter.m_aInfo.m_nCopies = aValue.ToInt32();

                aValue = aConfig.ReadKey( "Orientation" );
                if( aValue.Len() )
                    aPrinter.m_aInfo.m_eOrientation = aValue.EqualsIgnoreCaseAscii( "Landscape" ) ? orientation::Landscape : orientation::Portrait;

                aValue = aConfig.ReadKey( "MarginAdjust" );
                if( aValue.Len() )
                {
                    aPrinter.m_aInfo.m_nLeftMarginAdjust    = aValue.GetToken( 0, ',' ).ToInt32();
                    aPrinter.m_aInfo.m_nRightMarginAdjust   = aValue.GetToken( 1, ',' ).ToInt32();
                    aPrinter.m_aInfo.m_nTopMarginAdjust     = aValue.GetToken( 2, ',' ).ToInt32();
                    aPrinter.m_aInfo.m_nBottomMarginAdjust  = aValue.GetToken( 3, ',' ).ToInt32();
                }

                aValue = aConfig.ReadKey( "ColorDepth" );
                if( aValue.Len() )
                    aPrinter.m_aInfo.m_nColorDepth = aValue.ToInt32();

                aValue = aConfig.ReadKey( "ColorDevice" );
                if( aValue.Len() )
                    aPrinter.m_aInfo.m_nColorDevice = aValue.ToInt32();

                aValue = aConfig.ReadKey( "PSLevel" );
                if( aValue.Len() )
                    aPrinter.m_aInfo.m_nPSLevel = aValue.ToInt32();

                aValue = aConfig.ReadKey( "PerformFontSubstitution" );
                if( ! aValue.Equals( "0" ) && ! aValue.EqualsIgnoreCaseAscii( "false" ) )
                    aPrinter.m_aInfo.m_bPerformFontSubstitution = true;
                else
                    aPrinter.m_aInfo.m_bPerformFontSubstitution = false;

                // now iterate over all keys to extract multi key information:
                // 1. PPDContext information
                // 2. Font substitution table
                for( int nKey = 0; nKey < aConfig.GetKeyCount(); nKey++ )
                {
                    ByteString aKey( aConfig.GetKeyName( nKey ) );
                    if( aKey.CompareTo( "PPD_", 4 ) == COMPARE_EQUAL && aPrinter.m_aInfo.m_pParser )
                    {
                        aValue = aConfig.ReadKey( aKey );
                        const PPDKey* pKey = aPrinter.m_aInfo.m_pParser->getKey( String( aKey.Copy( 4 ), RTL_TEXTENCODING_ISO_8859_1 ) );
                        if( pKey )
                        {
                            aPrinter.m_aInfo.m_aContext.
                            setValue( pKey,
                            aValue.Equals( "*nil" ) ? NULL : pKey->getValue( String( aValue, RTL_TEXTENCODING_ISO_8859_1 ) ),
                            TRUE );
                        }
                    }
                    else if( aKey.Len() > 10 && aKey.CompareTo("SubstFont_", 10 ) == COMPARE_EQUAL )
                    {
                        aValue = aConfig.ReadKey( aKey );
                        aPrinter.m_aInfo.m_aFontSubstitutes[ OStringToOUString( aKey.Copy( 10 ), RTL_TEXTENCODING_ISO_8859_1 ) ] = OStringToOUString( aValue, RTL_TEXTENCODING_ISO_8859_1 );
                    }
                }

                setDefaultPaper( aPrinter.m_aInfo.m_aContext );
                fillFontSubstitutions( aPrinter.m_aInfo );

                // finally insert printer
                FileBase::getFileURLFromSystemPath( aFile.PathToFileName(), aPrinter.m_aFile );
                aPrinter.m_bModified    = false;
                aPrinter.m_aGroup       = aConfig.GetGroupName( nGroup );
                std::hash_map< OUString, Printer, OUStringHash >::const_iterator find_it =
                m_aPrinters.find( aPrinterName );
                if( find_it != m_aPrinters.end() )
                {
                    aPrinter.m_aAlternateFiles = find_it->second.m_aAlternateFiles;
                    aPrinter.m_aAlternateFiles.push_front( find_it->second.m_aFile );
                }
                m_aPrinters[ aPrinterName ] = aPrinter;
            }
        }
    }

    // set default printer
    if( m_aPrinters.size() )
    {
        if( m_aPrinters.find( aDefaultPrinter ) == m_aPrinters.end() )
            aDefaultPrinter = m_aPrinters.begin()->first;
    }
    else
        aDefaultPrinter = OUString();
    m_aDefaultPrinter = aDefaultPrinter;

    if( m_eType != Default )
        return;

    // add a default printer for every available print queue
    // merge paper and font substitution from default printer,
    // all else from global defaults
    PrinterInfo aMergeInfo( m_aGlobalDefaults );
    aMergeInfo.m_aDriverName    = String( RTL_CONSTASCII_USTRINGPARAM( "SGENPRT" ) );
    aMergeInfo.m_aFeatures      = String( RTL_CONSTASCII_USTRINGPARAM( "autoqueue" ) );

    if( m_aDefaultPrinter.getLength() )
    {
        PrinterInfo aDefaultInfo( getPrinterInfo( m_aDefaultPrinter ) );
        aMergeInfo.m_bPerformFontSubstitution = aDefaultInfo.m_bPerformFontSubstitution;
        fillFontSubstitutions( aMergeInfo );

        const PPDKey* pDefKey           = aDefaultInfo.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) ) );
        const PPDKey* pMergeKey         = aMergeInfo.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) ) );
        const PPDValue* pDefValue       = aDefaultInfo.m_aContext.getValue( pDefKey );
        const PPDValue* pMergeValue     = pMergeKey ? pMergeKey->getValue( pDefValue->m_aOption ) : NULL;
        if( pMergeKey && pMergeValue )
            aMergeInfo.m_aContext.setValue( pMergeKey, pMergeValue );
    }

    getSystemPrintQueues();
    for( ::std::list< SystemPrintQueue >::iterator it = m_aSystemPrintQueues.begin(); it != m_aSystemPrintQueues.end(); ++it )
    {
        String aPrinterName( RTL_CONSTASCII_USTRINGPARAM( "<" ) );
        aPrinterName += String( it->m_aQueue );
        aPrinterName.Append( '>' );

        if( m_aPrinters.find( aPrinterName ) != m_aPrinters.end() )
            // probably user made this one permanent in padmin
        continue;

        String aCmd( m_aSystemPrintCommand );
        aCmd.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "(PRINTER)" ) ), it->m_aQueue );

        Printer aPrinter;

        // initialize to merged defaults
        aPrinter.m_aInfo = aMergeInfo;
        aPrinter.m_aInfo.m_aPrinterName     = aPrinterName;
        aPrinter.m_aInfo.m_aCommand         = aCmd;
        aPrinter.m_aInfo.m_aComment         = it->m_aComment;
        aPrinter.m_aInfo.m_aLocation        = it->m_aLocation;
        aPrinter.m_bModified                = false;
        aPrinter.m_aGroup                   = ByteString( aPrinterName, aEncoding ); //provide group name in case user makes this one permanent in padmin

        m_aPrinters[ aPrinterName ] = aPrinter;
    }
}

// -----------------------------------------------------------------

void PrinterInfoManager::listPrinters( ::std::list< OUString >& rList ) const
{
    ::std::hash_map< OUString, Printer, OUStringHash >::const_iterator it;
    rList.clear();
    for( it = m_aPrinters.begin(); it != m_aPrinters.end(); ++it )
        rList.push_back( it->first );
}

// -----------------------------------------------------------------

const PrinterInfo& PrinterInfoManager::getPrinterInfo( const OUString& rPrinter ) const
{
    static PrinterInfo aEmptyInfo;
    ::std::hash_map< OUString, Printer, OUStringHash >::const_iterator it = m_aPrinters.find( rPrinter );

    DBG_ASSERT( it != m_aPrinters.end(), "Do not ask for info about nonexistant printers" );

    return it != m_aPrinters.end() ? it->second.m_aInfo : aEmptyInfo;
}

// -----------------------------------------------------------------

void PrinterInfoManager::changePrinterInfo( const OUString& rPrinter, const PrinterInfo& rNewInfo )
{
    ::std::hash_map< OUString, Printer, OUStringHash >::iterator it = m_aPrinters.find( rPrinter );

    DBG_ASSERT( it != m_aPrinters.end(), "Do not change nonexistant printers" );

    if( it != m_aPrinters.end() )
    {
        it->second.m_aInfo      = rNewInfo;
        // recalculate font substitutions
        fillFontSubstitutions( it->second.m_aInfo );
        it->second.m_bModified  = true;
        writePrinterConfig();
    }
}

// -----------------------------------------------------------------

// need to check writeability / creatability of config files
static bool checkWriteability( const OUString& rUniPath )
{
    bool bRet = false;
    OUString aSysPath;
    FileBase::getSystemPathFromFileURL( rUniPath, aSysPath );
    SvFileStream aStream( aSysPath, STREAM_READ | STREAM_WRITE );
    if( aStream.IsOpen() && aStream.IsWritable() )
        bRet = true;
    return bRet;
}

bool PrinterInfoManager::writePrinterConfig()
{
    // find at least one writeable config
    ::std::hash_map< OUString, Config*, OUStringHash > files;
    ::std::hash_map< OUString, int, OUStringHash > rofiles;
    ::std::hash_map< OUString, Config*, OUStringHash >::iterator file_it;

    for( ::std::list< WatchFile >::const_iterator wit = m_aWatchFiles.begin(); wit != m_aWatchFiles.end(); ++wit )
    {
        if( checkWriteability( wit->m_aFilePath ) )
        {
            files[ wit->m_aFilePath ] = new Config( wit->m_aFilePath );
            break;
        }
    }

    if( files.empty() )
        return false;

    Config* pGlobal = files.begin()->second;
    pGlobal->SetGroup( GLOBAL_DEFAULTS_GROUP );
    pGlobal->WriteKey( "DisableCUPS", m_bDisableCUPS ? "true" : "false" );

    ::std::hash_map< OUString, Printer, OUStringHash >::iterator it;
    for( it = m_aPrinters.begin(); it != m_aPrinters.end(); ++it )
    {
        if( ! it->second.m_bModified )
            // printer was not changed, do nothing
        continue;

        // don't save autoqueue printers
        sal_Int32 nIndex = 0;
        bool bAutoQueue = false;
        while( nIndex != -1 && ! bAutoQueue )
        {
            OUString aToken( it->second.m_aInfo.m_aFeatures.getToken( 0, ',', nIndex ) );
            if( aToken.getLength() && aToken.compareToAscii( "autoqueue" ) == 0 )
                bAutoQueue = true;
        }
        if( bAutoQueue )
            continue;

        if( it->second.m_aFile.getLength() )
        {
            // check if file is writable
            if( files.find( it->second.m_aFile ) == files.end() )
            {
                bool bInsertToNewFile = false;
                // maybe it is simply not inserted yet
                if( rofiles.find( it->second.m_aFile ) == rofiles.end() )
                {
                    if( checkWriteability( it->second.m_aFile ) )
                        files[ it->second.m_aFile ] = new Config( it->second.m_aFile );
                    else
                        bInsertToNewFile = true;
                }
                else
                    bInsertToNewFile = true;
                // original file is read only, insert printer in a new writeable file
                if( bInsertToNewFile )
                {
                    rofiles[ it->second.m_aFile ] = 1;
                    // update alternate file list
                    // the remove operation ensures uniqueness of each alternate
                    it->second.m_aAlternateFiles.remove( it->second.m_aFile );
                    it->second.m_aAlternateFiles.remove( files.begin()->first );
                    it->second.m_aAlternateFiles.push_front( it->second.m_aFile );
                    // update file
                    it->second.m_aFile = files.begin()->first;
                }
            }
        }
        else // a new printer, write it to the first file available
            it->second.m_aFile = files.begin()->first;

        if( ! it->second.m_aGroup.getLength() ) // probably a new printer
            it->second.m_aGroup = OString( it->first.getStr(), it->first.getLength(), RTL_TEXTENCODING_UTF8 );

        if( files.find( it->second.m_aFile ) != files.end() )
        {
            Config* pConfig = files[ it->second.m_aFile ];
            pConfig->DeleteGroup( it->second.m_aGroup ); // else some old keys may remain
            pConfig->SetGroup( it->second.m_aGroup );

            ByteString aValue( String( it->second.m_aInfo.m_aDriverName ), RTL_TEXTENCODING_UTF8 );
            aValue += '/';
            aValue += ByteString( String( it->first ), RTL_TEXTENCODING_UTF8 );
            pConfig->WriteKey( "Printer", aValue );
            pConfig->WriteKey( "DefaultPrinter", it->first == m_aDefaultPrinter ? "1" : "0" );
            pConfig->WriteKey( "Location", ByteString( String( it->second.m_aInfo.m_aLocation ), RTL_TEXTENCODING_UTF8 ) );
            pConfig->WriteKey( "Comment", ByteString( String( it->second.m_aInfo.m_aComment ), RTL_TEXTENCODING_UTF8 ) );
            pConfig->WriteKey( "Command", ByteString( String( it->second.m_aInfo.m_aCommand ), RTL_TEXTENCODING_UTF8 ) );
            pConfig->WriteKey( "QuickCommand", ByteString( String( it->second.m_aInfo.m_aQuickCommand ), RTL_TEXTENCODING_UTF8 ) );
            pConfig->WriteKey( "Features", ByteString( String( it->second.m_aInfo.m_aFeatures ), RTL_TEXTENCODING_UTF8 ) );
            pConfig->WriteKey( "Copies", ByteString::CreateFromInt32( it->second.m_aInfo.m_nCopies ) );
            pConfig->WriteKey( "Orientation", it->second.m_aInfo.m_eOrientation == orientation::Landscape ? "Landscape" : "Portrait" );
            pConfig->WriteKey( "PSLevel", ByteString::CreateFromInt32( it->second.m_aInfo.m_nPSLevel ) );
            pConfig->WriteKey( "ColorDevice", ByteString::CreateFromInt32( it->second.m_aInfo.m_nColorDevice ) );
            pConfig->WriteKey( "ColorDepth", ByteString::CreateFromInt32( it->second.m_aInfo.m_nColorDepth ) );
            aValue = ByteString::CreateFromInt32( it->second.m_aInfo.m_nLeftMarginAdjust );
            aValue += ',';
            aValue += ByteString::CreateFromInt32( it->second.m_aInfo.m_nRightMarginAdjust );
            aValue += ',';
            aValue += ByteString::CreateFromInt32( it->second.m_aInfo.m_nTopMarginAdjust );
            aValue += ',';
            aValue += ByteString::CreateFromInt32( it->second.m_aInfo.m_nBottomMarginAdjust );
            pConfig->WriteKey( "MarginAdjust", aValue );

            if( it->second.m_aInfo.m_aDriverName.compareToAscii( "CUPS:", 5 ) != 0 )
            {
                // write PPDContext (not for CUPS)
                for( int i = 0; i < it->second.m_aInfo.m_aContext.countValuesModified(); i++ )
                {
                    const PPDKey* pKey = it->second.m_aInfo.m_aContext.getModifiedKey( i );
                    ByteString aKey( "PPD_" );
                    aKey += ByteString( pKey->getKey(), RTL_TEXTENCODING_ISO_8859_1 );

                    const PPDValue* pValue = it->second.m_aInfo.m_aContext.getValue( pKey );
                    aValue = pValue ? ByteString( pValue->m_aOption, RTL_TEXTENCODING_ISO_8859_1 ) : ByteString( "*nil" );
                    pConfig->WriteKey( aKey, aValue );
                }
            }

            // write font substitution table
            pConfig->WriteKey( "PerformFontSubstitution", it->second.m_aInfo.m_bPerformFontSubstitution ? "true" : "false" );
            for( ::std::hash_map< OUString, OUString, OUStringHash >::const_iterator subst = it->second.m_aInfo.m_aFontSubstitutes.begin();
            subst != it->second.m_aInfo.m_aFontSubstitutes.end(); ++subst )
            {
                ByteString aKey( "SubstFont_" );
                aKey.Append( OUStringToOString( subst->first, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
                pConfig->WriteKey( aKey, OUStringToOString( subst->second, RTL_TEXTENCODING_ISO_8859_1 ) );
            }
        }
    }

    // get rid of Config objects. this also writes any changes
    for( file_it = files.begin(); file_it != files.end(); ++file_it )
        delete file_it->second;

    return true;
}

// -----------------------------------------------------------------

bool PrinterInfoManager::addPrinter( const OUString& rPrinterName, const OUString& rDriverName )
{
    bool bSuccess = false;

    const PPDParser* pParser = NULL;
    if( m_aPrinters.find( rPrinterName ) == m_aPrinters.end() && ( pParser = PPDParser::getParser( rDriverName ) ) )
    {
        Printer aPrinter;
        aPrinter.m_bModified                        = true;
        aPrinter.m_aInfo                            = m_aGlobalDefaults;
        aPrinter.m_aInfo.m_aDriverName              = rDriverName;
        aPrinter.m_aInfo.m_pParser                  = pParser;
        aPrinter.m_aInfo.m_aContext.setParser( pParser );
        aPrinter.m_aInfo.m_aPrinterName             = rPrinterName;

        fillFontSubstitutions( aPrinter.m_aInfo );
        // merge PPD values with global defaults
        for( int nPPDValueModified = 0; nPPDValueModified < m_aGlobalDefaults.m_aContext.countValuesModified(); nPPDValueModified++ )
        {
            const PPDKey* pDefKey = m_aGlobalDefaults.m_aContext.getModifiedKey( nPPDValueModified );
            const PPDValue* pDefValue = m_aGlobalDefaults.m_aContext.getValue( pDefKey );
            const PPDKey* pPrinterKey = pDefKey ? aPrinter.m_aInfo.m_pParser->getKey( pDefKey->getKey() ) : NULL;
            if( pDefKey && pPrinterKey )
                // at least the options exist in both PPDs
            {
                if( pDefValue )
                {
                    const PPDValue* pPrinterValue = pPrinterKey->getValue( pDefValue->m_aOption );
                    if( pPrinterValue )
                        // the printer has a corresponding option for the key
                    aPrinter.m_aInfo.m_aContext.setValue( pPrinterKey, pPrinterValue );
                }
                else
                    aPrinter.m_aInfo.m_aContext.setValue( pPrinterKey, NULL );
            }
        }

        m_aPrinters[ rPrinterName ] = aPrinter;
        bSuccess = true;
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "new printer %s, level = %d, colordevice = %d, depth = %d\n",
        OUStringToOString( rPrinterName, osl_getThreadTextEncoding() ).getStr(),
        m_aPrinters[rPrinterName].m_aInfo.m_nPSLevel,
        m_aPrinters[rPrinterName].m_aInfo.m_nColorDevice,
        m_aPrinters[rPrinterName].m_aInfo.m_nColorDepth );
        #endif
        // comment: logically one should writePrinterConfig() here
        // but immediately after addPrinter() a changePrinterInfo()
        // will follow (see padmin code), which writes it again,
        // so we can currently save some performance here
    }
    return bSuccess;
}

// -----------------------------------------------------------------

bool PrinterInfoManager::removePrinter( const OUString& rPrinterName, bool bCheckOnly )
{
    bool bSuccess = true;

    ::std::hash_map< OUString, Printer, OUStringHash >::iterator it = m_aPrinters.find( rPrinterName );
    if( it != m_aPrinters.end() )
    {
        if( it->second.m_aFile.getLength() )
        {
            // this printer already exists in a config file


            // check writeability of config file(s)
            if( ! checkWriteability( it->second.m_aFile ) )
                bSuccess = false;
            else
            {
                for( std::list< OUString >::const_iterator file_it = it->second.m_aAlternateFiles.begin();
                file_it != it->second.m_aAlternateFiles.end() && bSuccess; ++file_it )
                {
                    if( ! checkWriteability( *file_it ) )
                        bSuccess = false;
                }
            }
            if( bSuccess && ! bCheckOnly )
            {

                Config aConfig( it->second.m_aFile );
                aConfig.DeleteGroup( it->second.m_aGroup );
                aConfig.Flush();
                for( std::list< OUString >::const_iterator file_it = it->second.m_aAlternateFiles.begin();
                file_it != it->second.m_aAlternateFiles.end() && bSuccess; ++file_it )
                {
                    Config aAltConfig( *file_it );
                    aAltConfig.DeleteGroup( it->second.m_aGroup );
                    aAltConfig.Flush();
                }
            }
        }
        if( bSuccess && ! bCheckOnly )
        {
            m_aPrinters.erase( it );
            // need this here because someone may call
            // checkPrintersChanged after the removal
            // but then other added printers were not flushed
            // to disk, so they are discarded
            writePrinterConfig();
        }
    }
    return bSuccess;
}

// -----------------------------------------------------------------

bool PrinterInfoManager::setDefaultPrinter( const OUString& rPrinterName )
{
    bool bSuccess = false;

    ::std::hash_map< OUString, Printer, OUStringHash >::iterator it = m_aPrinters.find( rPrinterName );
    if( it != m_aPrinters.end() )
    {
        bSuccess = true;
        it->second.m_bModified = true;
        if( ( it = m_aPrinters.find( m_aDefaultPrinter ) ) != m_aPrinters.end() )
            it->second.m_bModified = true;
        m_aDefaultPrinter = rPrinterName;
        writePrinterConfig();
    }
    return bSuccess;
}

// -----------------------------------------------------------------
bool PrinterInfoManager::addOrRemovePossible() const
{
    return true;
}

// -----------------------------------------------------------------

void PrinterInfoManager::fillFontSubstitutions( PrinterInfo& rInfo ) const
{
    PrintFontManager& rFontManager( PrintFontManager::get() );
    rInfo.m_aFontSubstitutions.clear();

    if( ! rInfo.m_bPerformFontSubstitution ||
        ! rInfo.m_aFontSubstitutes.size() )
    return;

    ::std::list< FastPrintFontInfo > aFonts;
    ::std::hash_map< OUString, ::std::list< FastPrintFontInfo >, OUStringHash > aPrinterFonts;
    rFontManager.getFontListWithFastInfo( aFonts, rInfo.m_pParser );

    // get builtin fonts
    ::std::list< FastPrintFontInfo >::const_iterator it;
    for( it = aFonts.begin(); it != aFonts.end(); ++it )
        if( it->m_eType == fonttype::Builtin )
            aPrinterFonts[ it->m_aFamilyName.toAsciiLowerCase() ].push_back( *it );

    // map lower case, so build a local copy of the font substitutions
    ::std::hash_map< OUString, OUString, OUStringHash > aSubstitutions;
    ::std::hash_map< OUString, OUString, OUStringHash >::const_iterator subst;
    for( subst = rInfo.m_aFontSubstitutes.begin(); subst != rInfo.m_aFontSubstitutes.end(); ++subst )
    {
        OUString aFamily( subst->first.toAsciiLowerCase() );
        // first look if there is a builtin of this family
        // in this case override the substitution table
        if( aPrinterFonts.find( aFamily ) != aPrinterFonts.end() )
            aSubstitutions[ aFamily ] = aFamily;
        else
            aSubstitutions[ aFamily ] = subst->second.toAsciiLowerCase();
    }


    // now find substitutions
    for( it = aFonts.begin(); it != aFonts.end(); ++it )
    {
        if( it->m_eType != fonttype::Builtin )
        {
            OUString aFamily( it->m_aFamilyName.toAsciiLowerCase() );
            subst = aSubstitutions.find( aFamily );
            if( subst != aSubstitutions.end() )
            {
                // search a substitution
                const ::std::list< FastPrintFontInfo >& rBuiltins( aPrinterFonts[ aSubstitutions[ aFamily ] ] );
                ::std::list< FastPrintFontInfo >::const_iterator builtin;
                int nLastMatch = -10000;
                fontID nSubstitute = -1;
                for( builtin = rBuiltins.begin(); builtin != rBuiltins.end(); ++builtin )
                {
                    int nMatch = 0;
                    int nDiff;
                    if( builtin->m_eItalic == it->m_eItalic )
                        nMatch += 8000;

                    nDiff = builtin->m_eWeight - it->m_eWeight;
                    nDiff = nDiff < 0 ? -nDiff : nDiff;
                    nMatch += 4000 - 1000*nDiff;

                    nDiff = builtin->m_eWidth - it->m_eWidth;
                    nDiff = nDiff < 0 ? -nDiff : nDiff;
                    nMatch += 2000 - 500*nDiff;

                    if( nMatch > nLastMatch )
                    {
                        nLastMatch = nMatch;
                        nSubstitute = builtin->m_nID;
                    }
                }
                if( nSubstitute != -1 )
                {
                    rInfo.m_aFontSubstitutions[ it->m_nID ] = nSubstitute;
                    #if OSL_DEBUG_LEVEL > 2
                    FastPrintFontInfo aInfo;
                    rFontManager.getFontFastInfo( nSubstitute, aInfo );
                    fprintf( stderr,
                    "substitute %s %s %d %d\n"
                    " ->        %s %s %d %d\n",
                    OUStringToOString( it->m_aFamilyName, RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
                    it->m_eItalic == italic::Upright ? "r" : it->m_eItalic == italic::Oblique ? "o" : it->m_eItalic == italic::Italic ? "i" : "u",
                    it->m_eWeight,
                    it->m_eWidth,

                    OUStringToOString( aInfo.m_aFamilyName, RTL_TEXTENCODING_ISO_8859_1 ).getStr(),
                    aInfo.m_eItalic == italic::Upright ? "r" : aInfo.m_eItalic == italic::Oblique ? "o" : aInfo.m_eItalic == italic::Italic ? "i" : "u",
                    aInfo.m_eWeight,
                    aInfo.m_eWidth
                    );
                    #endif
                }
            }
        }
    }
}

// -----------------------------------------------------------------

void PrinterInfoManager::getSystemPrintCommands( std::list< OUString >& rCommands )
{
    if( m_pQueueInfo && m_pQueueInfo->hasChanged() )
    {
        m_aSystemPrintCommand = m_pQueueInfo->getCommand();
        m_pQueueInfo->getSystemQueues( m_aSystemPrintQueues );
        delete m_pQueueInfo, m_pQueueInfo = NULL;
    }

    std::list< SystemPrintQueue >::const_iterator it;
    rCommands.clear();
    String aPrinterConst( RTL_CONSTASCII_USTRINGPARAM( "(PRINTER)" ) );
    for( it = m_aSystemPrintQueues.begin(); it != m_aSystemPrintQueues.end(); ++it )
    {
        String aCmd( m_aSystemPrintCommand );
        aCmd.SearchAndReplace( aPrinterConst, it->m_aQueue );
        rCommands.push_back( aCmd );
    }
}

const std::list< PrinterInfoManager::SystemPrintQueue >& PrinterInfoManager::getSystemPrintQueues()
{
    if( m_pQueueInfo && m_pQueueInfo->hasChanged() )
    {
        m_aSystemPrintCommand = m_pQueueInfo->getCommand();
        m_pQueueInfo->getSystemQueues( m_aSystemPrintQueues );
        delete m_pQueueInfo, m_pQueueInfo = NULL;
    }

    return m_aSystemPrintQueues;
}

bool PrinterInfoManager::checkFeatureToken( const rtl::OUString& rPrinterName, const char* pToken ) const
{
    const PrinterInfo& rPrinterInfo( getPrinterInfo( rPrinterName ) );
    sal_Int32 nIndex = 0;
    while( nIndex != -1 )
    {
        OUString aOuterToken = rPrinterInfo.m_aFeatures.getToken( 0, ',', nIndex );
        sal_Int32 nInnerIndex = 0;
        OUString aInnerToken = aOuterToken.getToken( 0, '=', nInnerIndex );
        if( aInnerToken.equalsIgnoreAsciiCaseAscii( pToken ) )
            return true;
    }
    return false;
}

FILE* PrinterInfoManager::startSpool( const OUString& rPrintername, bool bQuickCommand )
{
    const PrinterInfo&   rPrinterInfo   = getPrinterInfo (rPrintername);
    const rtl::OUString& rCommand       = (bQuickCommand && rPrinterInfo.m_aQuickCommand.getLength() ) ?
                                          rPrinterInfo.m_aQuickCommand : rPrinterInfo.m_aCommand;
    rtl::OString aShellCommand  = OUStringToOString (rCommand, RTL_TEXTENCODING_ISO_8859_1);
    aShellCommand += rtl::OString( " 2>/dev/null" );

    return popen (aShellCommand.getStr(), "w");
}

int PrinterInfoManager::endSpool( const OUString& /*rPrintername*/, const OUString& /*rJobTitle*/, FILE* pFile, const JobData& /*rDocumentJobData*/ )
{
    return (0 == pclose( pFile ));
}

void PrinterInfoManager::setupJobContextData( JobData& rData )
{
    std::hash_map< OUString, Printer, OUStringHash >::iterator it =
    m_aPrinters.find( rData.m_aPrinterName );
    if( it != m_aPrinters.end() )
    {
        rData.m_pParser     = it->second.m_aInfo.m_pParser;
        rData.m_aContext    = it->second.m_aInfo.m_aContext;
    }
}

void PrinterInfoManager::setDefaultPaper( PPDContext& rContext ) const
{
    if(  ! rContext.getParser() )
        return;

    const PPDKey* pPageSizeKey = rContext.getParser()->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) ) );
    if( ! pPageSizeKey )
        return;

    int nModified = rContext.countValuesModified();
    while( nModified-- &&
        rContext.getModifiedKey( nModified ) != pPageSizeKey )
    ;

    if( nModified >= 0 ) // paper was set already, do not modify
    {
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "not setting default paper, already set %s\n",
        OUStringToOString( rContext.getValue( pPageSizeKey )->m_aOption, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
        #endif
        return;
    }

    // paper not set, fill in default value
    const PPDValue* pPaperVal = NULL;
    int nValues = pPageSizeKey->countValues();
    for( int i = 0; i < nValues && ! pPaperVal; i++ )
    {
        const PPDValue* pVal = pPageSizeKey->getValue( i );
        if( pVal->m_aOption.EqualsIgnoreCaseAscii( m_aSystemDefaultPaper.getStr() ) )
            pPaperVal = pVal;
    }
    if( pPaperVal )
    {
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "setting default paper %s\n", OUStringToOString( pPaperVal->m_aOption, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
        #endif
        rContext.setValue( pPageSizeKey, pPaperVal );
        #if OSL_DEBUG_LEVEL > 1
        pPaperVal = rContext.getValue( pPageSizeKey );
        fprintf( stderr, "-> got paper %s\n", OUStringToOString( pPaperVal->m_aOption, RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
        #endif
    }
}

// -----------------------------------------------------------------

SystemQueueInfo::SystemQueueInfo() :
    m_bChanged( false )
{
    create();
}

SystemQueueInfo::~SystemQueueInfo()
{
    terminate();
}

bool SystemQueueInfo::hasChanged() const
{
    MutexGuard aGuard( m_aMutex );
    bool bChanged = m_bChanged;
    return bChanged;
}

void SystemQueueInfo::getSystemQueues( std::list< PrinterInfoManager::SystemPrintQueue >& rQueues )
{
    MutexGuard aGuard( m_aMutex );
    rQueues = m_aQueues;
    m_bChanged = false;
}

OUString SystemQueueInfo::getCommand() const
{
    MutexGuard aGuard( m_aMutex );
    OUString aRet = m_aCommand;
    return aRet;
}

struct SystemCommandParameters;
typedef void(* tokenHandler)(const std::list< rtl::OString >&,
                std::list< PrinterInfoManager::SystemPrintQueue >&,
                const SystemCommandParameters*);

struct SystemCommandParameters
{
    const char*     pQueueCommand;
    const char*     pPrintCommand;
    const char*     pForeToken;
    const char*     pAftToken;
    unsigned int    nForeTokenCount;
    tokenHandler    pHandler;
};

#if ! (defined(LINUX) || defined(NETBSD) || defined(FREEBSD) || defined(MACOSX))
static void lpgetSysQueueTokenHandler(
    const std::list< rtl::OString >& i_rLines,
    std::list< PrinterInfoManager::SystemPrintQueue >& o_rQueues,
    const SystemCommandParameters* )
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    std::hash_set< OUString, OUStringHash > aUniqueSet;
    std::hash_set< OUString, OUStringHash > aOnlySet;
    aUniqueSet.insert( OUString( RTL_CONSTASCII_USTRINGPARAM( "_all" ) ) );
    aUniqueSet.insert( OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ) );

    // the eventual "all" attribute of the "_all" queue tells us, which
    // printers are to be used for this user at all

    // find _all: line
    rtl::OString aAllLine( "_all:" );
    rtl::OString aAllAttr( "all=" );
    for( std::list< rtl::OString >::const_iterator it = i_rLines.begin();
         it != i_rLines.end(); ++it )
    {
        if( it->indexOf( aAllLine, 0 ) == 0 )
        {
            // now find the "all" attribute
            ++it;
            while( it != i_rLines.end() )
            {
                rtl::OString aClean( WhitespaceToSpace( *it ) );
                if( aClean.indexOf( aAllAttr, 0 ) == 0 )
                {
                    // insert the comma separated entries into the set of printers to use
                    sal_Int32 nPos = aAllAttr.getLength();
                    while( nPos != -1 )
                    {
                        OString aTok( aClean.getToken( 0, ',', nPos ) );
                        if( aTok.getLength() > 0 )
                            aOnlySet.insert( rtl::OStringToOUString( aTok, aEncoding ) );
                    }
                    break;
                }
            }
            break;
        }
    }

    bool bInsertAttribute = false;
    rtl::OString aDescrStr( "description=" );
    rtl::OString aLocStr( "location=" );
    for( std::list< rtl::OString >::const_iterator it = i_rLines.begin();
         it != i_rLines.end(); ++it )
    {
        sal_Int32 nPos = 0;
        // find the begin of a new printer section
        nPos = it->indexOf( ':', 0 );
        if( nPos != -1 )
        {
            OUString aSysQueue( rtl::OStringToOUString( it->copy( 0, nPos ), aEncoding ) );
            // do not insert duplicates (e.g. lpstat tends to produce such lines)
            // in case there was a "_all" section, insert only those printer explicitly
            // set in the "all" attribute
            if( aUniqueSet.find( aSysQueue ) == aUniqueSet.end() &&
                ( aOnlySet.empty() || aOnlySet.find( aSysQueue ) != aOnlySet.end() )
                )
            {
                o_rQueues.push_back( PrinterInfoManager::SystemPrintQueue() );
                o_rQueues.back().m_aQueue = aSysQueue;
                o_rQueues.back().m_aLocation = aSysQueue;
                aUniqueSet.insert( aSysQueue );
                bInsertAttribute = true;
            }
            else
                bInsertAttribute = false;
            continue;
        }
        if( bInsertAttribute && ! o_rQueues.empty() )
        {
            // look for "description" attribute, insert as comment
            nPos = it->indexOf( aDescrStr, 0 );
            if( nPos != -1 )
            {
                ByteString aComment( WhitespaceToSpace( it->copy(nPos+12) ) );
                if( aComment.Len() > 0 )
                    o_rQueues.back().m_aComment = String( aComment, aEncoding );
                continue;
            }
            // look for "location" attribute, inser as location
            nPos = it->indexOf( aLocStr, 0 );
            if( nPos != -1 )
            {
                ByteString aLoc( WhitespaceToSpace( it->copy(nPos+9) ) );
                if( aLoc.Len() > 0 )
                    o_rQueues.back().m_aLocation = String( aLoc, aEncoding );
                continue;
            }
        }
    }
}
#endif
static void standardSysQueueTokenHandler(
    const std::list< rtl::OString >& i_rLines,
    std::list< PrinterInfoManager::SystemPrintQueue >& o_rQueues,
    const SystemCommandParameters* i_pParms)
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    std::hash_set< OUString, OUStringHash > aUniqueSet;
    rtl::OString aForeToken( i_pParms->pForeToken );
    rtl::OString aAftToken( i_pParms->pAftToken );
    /* Normal Unix print queue discovery, also used for Darwin 5 LPR printing
    */
    for( std::list< rtl::OString >::const_iterator it = i_rLines.begin();
         it != i_rLines.end(); ++it )
    {
        sal_Int32 nPos = 0;

        // search for a line describing a printer:
        // find if there are enough tokens before the name
        for( unsigned int i = 0; i < i_pParms->nForeTokenCount && nPos != -1; i++ )
        {
            nPos = it->indexOf( aForeToken, nPos );
            if( nPos != -1 && it->getLength() >= nPos+aForeToken.getLength() )
                nPos += aForeToken.getLength();
        }
        if( nPos != -1 )
        {
            // find if there is the token after the queue
            sal_Int32 nAftPos = it->indexOf( aAftToken, nPos );
            if( nAftPos != -1 )
            {
                // get the queue name between fore and aft tokens
                OUString aSysQueue( rtl::OStringToOUString( it->copy( nPos, nAftPos - nPos ), aEncoding ) );
                // do not insert duplicates (e.g. lpstat tends to produce such lines)
                if( aUniqueSet.find( aSysQueue ) == aUniqueSet.end() )
                {
                    o_rQueues.push_back( PrinterInfoManager::SystemPrintQueue() );
                    o_rQueues.back().m_aQueue = aSysQueue;
                    o_rQueues.back().m_aLocation = aSysQueue;
                    aUniqueSet.insert( aSysQueue );
                }
            }
        }
    }
}

static const struct SystemCommandParameters aParms[] =
{
    #if defined(LINUX) || defined(NETBSD) || defined(FREEBSD) || defined(MACOSX)
    { "/usr/sbin/lpc status", "lpr -P \"(PRINTER)\"", "", ":", 0, standardSysQueueTokenHandler },
    { "lpc status", "lpr -P \"(PRINTER)\"", "", ":", 0, standardSysQueueTokenHandler },
    { "LANG=C;LC_ALL=C;export LANG LC_ALL;lpstat -s", "lp -d \"(PRINTER)\"", "system for ", ": ", 1, standardSysQueueTokenHandler }
    #else
    { "LANG=C;LC_ALL=C;export LANG LC_ALL;lpget list", "lp -d \"(PRINTER)\"", "", ":", 0, lpgetSysQueueTokenHandler },
    { "LANG=C;LC_ALL=C;export LANG LC_ALL;lpstat -s", "lp -d \"(PRINTER)\"", "system for ", ": ", 1, standardSysQueueTokenHandler },
    { "/usr/sbin/lpc status", "lpr -P \"(PRINTER)\"", "", ":", 0, standardSysQueueTokenHandler },
    { "lpc status", "lpr -P \"(PRINTER)\"", "", ":", 0, standardSysQueueTokenHandler }
    #endif
};

void SystemQueueInfo::run()
{
    char pBuffer[1024];
    FILE *pPipe;
    std::list< rtl::OString > aLines;

    /* Discover which command we can use to get a list of all printer queues */
    for( unsigned int i = 0; i < sizeof(aParms)/sizeof(aParms[0]); i++ )
    {
        aLines.clear();
        rtl::OStringBuffer aCmdLine( 128 );
        aCmdLine.append( aParms[i].pQueueCommand );
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "trying print queue command \"%s\" ... ", aParms[i].pQueueCommand );
        #endif
        aCmdLine.append( " 2>/dev/null" );
        if( (pPipe = popen( aCmdLine.getStr(), "r" )) )
        {
            while( fgets( pBuffer, 1024, pPipe ) )
                aLines.push_back( rtl::OString( pBuffer ) );
            if( ! pclose( pPipe ) )
            {
                std::list< PrinterInfoManager::SystemPrintQueue > aSysPrintQueues;
                aParms[i].pHandler( aLines, aSysPrintQueues, &(aParms[i]) );
                MutexGuard aGuard( m_aMutex );
                m_bChanged  = true;
                m_aQueues   = aSysPrintQueues;
                m_aCommand  = rtl::OUString::createFromAscii( aParms[i].pPrintCommand );
                #if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "success\n" );
                #endif
                break;
            }
        }
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "failed\n" );
        #endif
    }
}

