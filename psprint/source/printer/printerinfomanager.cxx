/*************************************************************************
 *
 *  $RCSfile: printerinfomanager.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-11 17:18:48 $
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

#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/config.hxx>
#include <psprint/printerinfomanager.hxx>
#include <psprint/fontmanager.hxx>

// filename of configuration files
#define PRINT_FILENAME  "psprint.conf"
// the group of the global defaults
#define GLOBAL_DEFAULTS_GROUP "__Global_Printer_Defaults__"

using namespace psp;
using namespace rtl;
using namespace osl;

/*
 *  class PrinterInfoManager
 */

// -----------------------------------------------------------------

PrinterInfoManager& PrinterInfoManager::get()
{
    static PrinterInfoManager aManager;

    return aManager;
}

// -----------------------------------------------------------------

PrinterInfoManager::PrinterInfoManager()
{
    initialize();
}

// -----------------------------------------------------------------

PrinterInfoManager::~PrinterInfoManager()
{
}

// -----------------------------------------------------------------

bool PrinterInfoManager::checkPrintersChanged()
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
    if( bChanged )
        initialize();

    return bChanged;
}

// -----------------------------------------------------------------

void PrinterInfoManager::initialize()
{
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

    if( ! m_aGlobalDefaults.m_pParser )
    {
#ifdef DEBUG
        fprintf( stderr, "Error: no SGENPRT available, shutting down psprint...\n" );
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
#ifdef DEBUG
            fprintf( stderr, "found global defaults in %s\n", OUStringToOString( aFile.PathToFileName(), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif
            aConfig.SetGroup( GLOBAL_DEFAULTS_GROUP );

            ByteString aValue( aConfig.ReadKey( "Copies" ) );
            if( aValue.Len() )
                m_aGlobalDefaults.m_nCopies = aValue.ToInt32();

            aValue = aConfig.ReadKey( "Orientation" );
            m_aGlobalDefaults.m_eOrientation = aValue.EqualsIgnoreCaseAscii( "Landscape" ) ? orientation::Landscape : orientation::Portrait;

            aValue = aConfig.ReadKey( "Scale" );
            m_aGlobalDefaults.m_nScale = aValue.ToInt32();

            aValue = aConfig.ReadKey( "MarginAdjust" );
            m_aGlobalDefaults.m_nLeftMarginAdjust   = aValue.GetToken( 0, ',' ).ToInt32();
            m_aGlobalDefaults.m_nRightMarginAdjust  = aValue.GetToken( 1, ',' ).ToInt32();
            m_aGlobalDefaults.m_nTopMarginAdjust    = aValue.GetToken( 2, ',' ).ToInt32();
            m_aGlobalDefaults.m_nBottomMarginAdjust = aValue.GetToken( 3, ',' ).ToInt32();

            aValue = aConfig.ReadKey( "ColorDepth", "24" );
            m_aGlobalDefaults.m_nColorDepth = aValue.ToInt32();

            aValue = aConfig.ReadKey( "ColorDevice" );
            m_aGlobalDefaults.m_nColorDevice = aValue.ToInt32();

            aValue = aConfig.ReadKey( "PSLevel" );
            m_aGlobalDefaults.m_nPSLevel = aValue.ToInt32();

            aValue = aConfig.ReadKey( "PerformFontSubstitution" );
            if( ! aValue.Equals( "0" ) && ! aValue.EqualsIgnoreCaseAscii( "false" ) )
                m_aGlobalDefaults.m_bPerformFontSubstitution = true;
            else
                m_aGlobalDefaults.m_bPerformFontSubstitution = false;

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
#ifdef DEBUG
            fprintf( stderr, "global settings: fontsubst = %s, %d substitutes\n", m_aGlobalDefaults.m_bPerformFontSubstitution ? "true" : "false", m_aGlobalDefaults.m_aFontSubstitutes.size() );
#endif
            break;
        }
    }
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
#if defined SOLARIS
                    aValue = "lp";
#else
                    aValue = "lpr";
#endif
                }
                aPrinter.m_aInfo.m_aCommand = String( aValue, RTL_TEXTENCODING_UTF8 );

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

                if( aValue.Len() )
                {
                    aValue = aConfig.ReadKey( "Scale" );
                    aPrinter.m_aInfo.m_nScale = aValue.ToInt32();
                }

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
                    if( aKey.CompareTo( "PPD_", 4 ) == COMPARE_EQUAL )
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
    for( ::std::list< OUString >::iterator it = m_aSystemPrintQueues.begin(); it != m_aSystemPrintQueues.end(); ++it )
    {
        String aPrinterName( RTL_CONSTASCII_USTRINGPARAM( "<" ) );
        aPrinterName += String( *it );
        aPrinterName.Append( '>' );

        if( m_aPrinters.find( aPrinterName ) != m_aPrinters.end() )
            // probably user made this one permanent in padmin
            continue;

        String aCmd( m_aSystemPrintCommand );
        aCmd.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "(PRINTER)" ) ), *it );

        Printer aPrinter;

        // initialize to merged defaults
        aPrinter.m_aInfo = aMergeInfo;
        aPrinter.m_aInfo.m_aPrinterName     = aPrinterName;
        aPrinter.m_aInfo.m_aCommand         = aCmd;
        aPrinter.m_aInfo.m_aLocation        = *it;
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

    if( files.begin() == files.end() )
        return false;

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
                // maybe it is simply not inserted yet
                if( rofiles.find( it->second.m_aFile ) == rofiles.end() )
                {
                    if( checkWriteability( it->second.m_aFile ) )
                        files[ it->second.m_aFile ] = new Config( it->second.m_aFile );
                    else
                    {
                        rofiles[ it->second.m_aFile ] = 1;
                        // update alternate file list
                        // the remove operation ensures singularity of each alternate
                        it->second.m_aAlternateFiles.remove( it->second.m_aFile );
                        it->second.m_aAlternateFiles.remove( files.begin()->first );
                        it->second.m_aAlternateFiles.push_front( it->second.m_aFile );
                        // update file
                        it->second.m_aFile = files.begin()->first;
                    }
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
            pConfig->WriteKey( "Features", ByteString( String( it->second.m_aInfo.m_aFeatures ), RTL_TEXTENCODING_UTF8 ) );
            pConfig->WriteKey( "Copies", ByteString::CreateFromInt32( it->second.m_aInfo.m_nCopies ) );
            pConfig->WriteKey( "Scale", ByteString::CreateFromInt32( it->second.m_aInfo.m_nScale ) );
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

            // write PPDContext
            for( int i = 0; i < it->second.m_aInfo.m_aContext.countValuesModified(); i++ )
            {
                const PPDKey* pKey = it->second.m_aInfo.m_aContext.getModifiedKey( i );
                ByteString aKey( "PPD_" );
                aKey += ByteString( pKey->getKey(), RTL_TEXTENCODING_ISO_8859_1 );

                const PPDValue* pValue = it->second.m_aInfo.m_aContext.getValue( pKey );
                aValue = pValue ? ByteString( pValue->m_aOption, RTL_TEXTENCODING_ISO_8859_1 ) : ByteString( "*nil" );
                pConfig->WriteKey( aKey, aValue );
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
                    Config aConfig( *file_it );
                    aConfig.DeleteGroup( it->second.m_aGroup );
                    aConfig.Flush();
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
    }
    return bSuccess;
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
#ifdef DEBUG
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

struct SystemCommandParameters
{
    const char*     pQueueCommand;
    const char*     pPrintCommand;
    const char*     pForeToken;
    const char*     pAftToken;
    int                 nForeTokenCount;
};

static const struct SystemCommandParameters aParms[] =
{
#if defined(LINUX) || defined(NETBSD) || defined(FREEBSD)
    { "/usr/sbin/lpc status", "lpr -P \"(PRINTER)\"", "", ":", 0 },
    { "lpc status", "lpr -P \"(PRINTER)\"", "", ":", 0 },
    { "LANG=C;LC_ALL=C;export LANG LC_ALL;lpstat -s", "lp -d \"(PRINTER)\"", "system for ", ": ", 1 }
#else
    { "LANG=C;LC_ALL=C;export LANG LC_ALL;lpstat -s", "lp -d \"(PRINTER)\"", "system for ", ": ", 1 },
    { "/usr/sbin/lpc status", "lpr -P \"(PRINTER)\"", "", ":", 0 },
    { "lpc status", "lpr -P \"(PRINTER)\"", "", ":", 0 }
#endif
};


const ::std::list< OUString >& PrinterInfoManager::getSystemPrintQueues()
{
    if( m_aSystemPrintQueues.begin() == m_aSystemPrintQueues.end() )
    {
        char pBuffer[1024];
        ByteString aPrtQueueCmd, aForeToken, aAftToken, aString;
        int nForeTokenCount, i;
        FILE *pPipe;
        bool bSuccess = false;
        ::std::list< ByteString > aLines;
        rtl_TextEncoding aEncoding = gsl_getSystemTextEncoding();

        for( i = 0; i < sizeof(aParms)/sizeof(aParms[0]) && ! bSuccess; i++ )
        {
            aLines.clear();
            aPrtQueueCmd            = aParms[i].pQueueCommand;
            m_aSystemPrintCommand   = OUString::createFromAscii( aParms[i].pPrintCommand );
            aForeToken              = aParms[i].pForeToken;
            aAftToken               = aParms[i].pAftToken;
            nForeTokenCount         = aParms[i].nForeTokenCount;
#if defined DEBUG
            fprintf( stderr, "trying print queue command \"%s\" ... ", aParms[i].pQueueCommand );
#endif
            if( pPipe = popen( aPrtQueueCmd.GetBuffer(), "r" ) )
            {
                while( fgets( pBuffer, 1024, pPipe ) )
                    aLines.push_back( ByteString( pBuffer ) );
                if( ! pclose( pPipe ) )
                    bSuccess = TRUE;
            }
#if defined DEBUG
            fprintf( stderr, "%s\n", bSuccess ? "success" : "failed" );
#endif
        }

        while( aLines.begin() != aLines.end() )
        {
            int nPos = 0, nAftPos;

            ByteString aOutLine( aLines.front() );
            aLines.pop_front();

            for( int i = 0; i < nForeTokenCount && nPos != STRING_NOTFOUND; i++ )
            {
                nPos = aOutLine.Search( aForeToken, nPos );
                if( nPos != STRING_NOTFOUND && aOutLine.Len() >= nPos+aForeToken.Len() )
                    nPos += aForeToken.Len();
            }
            if( nPos != STRING_NOTFOUND )
            {
                nAftPos = aOutLine.Search( aAftToken, nPos );
                if( nAftPos != STRING_NOTFOUND )
                {
                    OUString aSysQueue( String( aOutLine.Copy( nPos, nAftPos - nPos ), aEncoding ) );
                    // do not insert doubles (e.g. lpstat tends to produce such lines)
                    ::std::list< OUString >::const_iterator it;
                    for( it = m_aSystemPrintQueues.begin(); it != m_aSystemPrintQueues.end() && *it != aSysQueue; ++it )
                        ;
                    if( it == m_aSystemPrintQueues.end() )
                        m_aSystemPrintQueues.push_back( aSysQueue );
                }
            }
        }
    }
    return m_aSystemPrintQueues;
}

void PrinterInfoManager::getSystemPrintCommands( ::std::list< OUString >& rCommands )
{
    const ::std::list< OUString >& rQueues = getSystemPrintQueues();
    ::std::list< OUString >::const_iterator it;
    rCommands.clear();
    static String aPrinterConst( RTL_CONSTASCII_USTRINGPARAM( "(PRINTER)" ) );
    for( it = rQueues.begin(); it != rQueues.end(); ++it )
    {
        String aCmd( m_aSystemPrintCommand );
        aCmd.SearchAndReplace( aPrinterConst, *it );
        rCommands.push_back( aCmd );
    }
}
