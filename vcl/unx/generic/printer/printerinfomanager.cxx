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

#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#include <unx/cpdmgr.hxx>
#include <unx/cupsmgr.hxx>
#include <vcl/strhelper.hxx>

#include <saldatabasic.hxx>

#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <tools/config.hxx>

#include <i18nutil/paper.hxx>
#include <rtl/strbuf.hxx>
#include <sal/macros.h>
#include <sal/log.hxx>

#include <osl/thread.hxx>
#include <osl/mutex.hxx>
#include <osl/process.h>

// filename of configuration files
#define PRINT_FILENAME  "psprint.conf"
// the group of the global defaults
#define GLOBAL_DEFAULTS_GROUP "__Global_Printer_Defaults__"

#include <unordered_set>

using namespace psp;
using namespace osl;

namespace psp
{
    class SystemQueueInfo : public Thread
    {
        mutable Mutex               m_aMutex;
        bool                        m_bChanged;
        std::vector< PrinterInfoManager::SystemPrintQueue >
                                    m_aQueues;
        OUString                    m_aCommand;

        virtual void SAL_CALL run() override;

        public:
        SystemQueueInfo();
        virtual ~SystemQueueInfo() override;

        bool hasChanged() const;
        OUString getCommand() const;

        // sets changed status to false; therefore not const
        void getSystemQueues( std::vector< PrinterInfoManager::SystemPrintQueue >& rQueues );
    };
} // namespace

/*
*  class PrinterInfoManager
*/

PrinterInfoManager& PrinterInfoManager::get()
{
    SalData* pSalData = GetSalData();

    if( ! pSalData->m_pPIManager )
    {
        pSalData->m_pPIManager = CPDManager::tryLoadCPD();
        if( ! pSalData->m_pPIManager )
            pSalData->m_pPIManager = CUPSManager::tryLoadCUPS();
        if( ! pSalData->m_pPIManager )
            pSalData->m_pPIManager = new PrinterInfoManager();

        pSalData->m_pPIManager->initialize();
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "PrinterInfoManager::get create Manager of type %d\n", pSalData->m_pPIManager->getType() );
#endif
    }

    return *pSalData->m_pPIManager;
}

void PrinterInfoManager::release()
{
    SalData* pSalData = GetSalData();
    delete pSalData->m_pPIManager;
    pSalData->m_pPIManager = nullptr;
}

PrinterInfoManager::PrinterInfoManager( Type eType ) :
    m_eType( eType ),
    m_bUseIncludeFeature( false ),
    m_bUseJobPatch( true ),
    m_aSystemDefaultPaper( "A4" )
{
    if( eType == Type::Default )
        m_pQueueInfo.reset( new SystemQueueInfo );

    m_aSystemDefaultPaper = OStringToOUString(
        PaperInfo::toPSName(PaperInfo::getSystemDefaultPaper().getPaper()),
        RTL_TEXTENCODING_UTF8);
}

PrinterInfoManager::~PrinterInfoManager()
{
    #if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "PrinterInfoManager: destroyed Manager of type %d\n", getType() );
    #endif
}

bool PrinterInfoManager::checkPrintersChanged( bool bWait )
{
    // check if files were created, deleted or modified since initialize()
    bool bChanged = false;
    for (auto const& watchFile : m_aWatchFiles)
    {
        DirectoryItem aItem;
        if( DirectoryItem::get( watchFile.m_aFilePath, aItem ) )
        {
            if( watchFile.m_aModified.Seconds != 0 )
            {
                bChanged = true; // file probably has vanished
                break;
            }
        }
        else
        {
            FileStatus aStatus( osl_FileStatus_Mask_ModifyTime );
            if( aItem.getFileStatus( aStatus ) )
            {
                bChanged = true; // unlikely but not impossible
                break;
            }
            else
            {
                TimeValue aModified = aStatus.getModifyTime();
                if( aModified.Seconds != watchFile.m_aModified.Seconds )
                {
                    bChanged = true;
                    break;
                }
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

void PrinterInfoManager::initialize()
{
    m_bUseIncludeFeature = false;
    m_aPrinters.clear();
    m_aWatchFiles.clear();
    OUString aDefaultPrinter;

    // first initialize the global defaults
    // have to iterate over all possible files
    // there should be only one global setup section in all
    // available config files
    m_aGlobalDefaults = PrinterInfo();

    // need a parser for the PPDContext. generic printer should do.
    m_aGlobalDefaults.m_pParser = PPDParser::getParser( "SGENPRT" );
    m_aGlobalDefaults.m_aContext.setParser( m_aGlobalDefaults.m_pParser );

    if( ! m_aGlobalDefaults.m_pParser )
    {
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "Error: no default PPD file SGENPRT available, shutting down psprint...\n" );
        #endif
        return;
    }

    std::vector< OUString > aDirList;
    psp::getPrinterPathList( aDirList, nullptr );
    for (auto const& printDir : aDirList)
    {
        INetURLObject aFile( printDir, INetProtocol::File, INetURLObject::EncodeMechanism::All );
        aFile.Append( PRINT_FILENAME );
        Config aConfig( aFile.PathToFileName() );
        if( aConfig.HasGroup( GLOBAL_DEFAULTS_GROUP ) )
        {
            #if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "found global defaults in %s\n", OUStringToOString( aFile.PathToFileName(), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
            #endif
            aConfig.SetGroup( GLOBAL_DEFAULTS_GROUP );

            OString aValue( aConfig.ReadKey( "Copies" ) );
            if (!aValue.isEmpty())
                m_aGlobalDefaults.m_nCopies = aValue.toInt32();

            aValue = aConfig.ReadKey( "Orientation" );
            if (!aValue.isEmpty())
                m_aGlobalDefaults.m_eOrientation = aValue.equalsIgnoreAsciiCase("Landscape") ? orientation::Landscape : orientation::Portrait;

            aValue = aConfig.ReadKey( "MarginAdjust" );
            if (!aValue.isEmpty())
            {
                sal_Int32 nIdx {0};
                m_aGlobalDefaults.m_nLeftMarginAdjust = aValue.getToken(0, ',', nIdx).toInt32();
                m_aGlobalDefaults.m_nRightMarginAdjust  = aValue.getToken(0, ',', nIdx).toInt32();
                m_aGlobalDefaults.m_nTopMarginAdjust = aValue.getToken(0, ',', nIdx).toInt32();
                m_aGlobalDefaults.m_nBottomMarginAdjust = aValue.getToken(0, ',', nIdx).toInt32();
            }

            aValue = aConfig.ReadKey( "ColorDepth", "24" );
            if (!aValue.isEmpty())
                m_aGlobalDefaults.m_nColorDepth = aValue.toInt32();

            aValue = aConfig.ReadKey( "ColorDevice" );
            if (!aValue.isEmpty())
                m_aGlobalDefaults.m_nColorDevice = aValue.toInt32();

            aValue = aConfig.ReadKey( "PSLevel" );
            if (!aValue.isEmpty())
                m_aGlobalDefaults.m_nPSLevel = aValue.toInt32();

            aValue = aConfig.ReadKey( "PDFDevice" );
            if (!aValue.isEmpty())
                m_aGlobalDefaults.m_nPDFDevice = aValue.toInt32();

            // get the PPDContext of global JobData
            for( int nKey = 0; nKey < aConfig.GetKeyCount(); ++nKey )
            {
                OString aKey( aConfig.GetKeyName( nKey ) );
                if (aKey.startsWith("PPD_"))
                {
                    aValue = aConfig.ReadKey( aKey );
                    const PPDKey* pKey = m_aGlobalDefaults.m_pParser->getKey(OStringToOUString(aKey.copy(4), RTL_TEXTENCODING_ISO_8859_1));
                    if( pKey )
                    {
                        m_aGlobalDefaults.m_aContext.
                        setValue( pKey,
                                  aValue == "*nil" ? nullptr : pKey->getValue(OStringToOUString(aValue, RTL_TEXTENCODING_ISO_8859_1)),
                                  true );
                    }
                }
            }
        }
    }
    setDefaultPaper( m_aGlobalDefaults.m_aContext );

    // now collect all available printers
    for (auto const& printDir : aDirList)
    {
        INetURLObject aDir( printDir, INetProtocol::File, INetURLObject::EncodeMechanism::All );
        INetURLObject aFile( aDir );
        aFile.Append( PRINT_FILENAME );

        // check directory validity
        OUString aUniPath;
        FileBase::getFileURLFromSystemPath( aDir.PathToFileName(), aUniPath );
        Directory aDirectory( aUniPath );
        if( aDirectory.open() )
            continue;
        aDirectory.close();

        FileBase::getFileURLFromSystemPath( aFile.PathToFileName(), aUniPath );
        FileStatus aStatus( osl_FileStatus_Mask_ModifyTime );
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
            OString aValue = aConfig.ReadKey( "Printer" );
            if (!aValue.isEmpty())
            {
                OUString aPrinterName;

                sal_Int32 nNamePos = aValue.indexOf('/');
                // check for valid value of "Printer"
                if (nNamePos == -1)
                    continue;

                Printer aPrinter;
                // initialize to global defaults
                aPrinter.m_aInfo = m_aGlobalDefaults;

                aPrinterName = OStringToOUString(aValue.copy(nNamePos+1),
                    RTL_TEXTENCODING_UTF8);
                aPrinter.m_aInfo.m_aPrinterName = aPrinterName;
                aPrinter.m_aInfo.m_aDriverName = OStringToOUString(aValue.copy(0, nNamePos), RTL_TEXTENCODING_UTF8);

                // set parser, merge settings
                // don't do this for CUPS printers as this is done
                // by the CUPS system itself
                if( !aPrinter.m_aInfo.m_aDriverName.startsWith( "CUPS:" ) )
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
                        const PPDKey* pPrinterKey = pDefKey ? aPrinter.m_aInfo.m_pParser->getKey( pDefKey->getKey() ) : nullptr;
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
                                aPrinter.m_aInfo.m_aContext.setValue( pPrinterKey, nullptr );
                        }
                    }

                    aValue = aConfig.ReadKey( "Command" );
                    // no printer without a command
                    if (aValue.isEmpty())
                    {
                        /*  TODO:
                        *  porters: please append your platform to the Solaris
                        *  case if your platform has SystemV printing per default.
                        */
                        #if defined __sun
                        aValue = "lp";
                        #else
                        aValue = "lpr";
                        #endif
                    }
                    aPrinter.m_aInfo.m_aCommand = OStringToOUString(aValue, RTL_TEXTENCODING_UTF8);
                }

                aValue = aConfig.ReadKey( "QuickCommand" );
                aPrinter.m_aInfo.m_aQuickCommand = OStringToOUString(aValue, RTL_TEXTENCODING_UTF8);

                aValue = aConfig.ReadKey( "Features" );
                aPrinter.m_aInfo.m_aFeatures = OStringToOUString(aValue, RTL_TEXTENCODING_UTF8);

                // override the settings in m_aGlobalDefaults if keys exist
                aValue = aConfig.ReadKey( "DefaultPrinter" );
                if (aValue != "0" && !aValue.equalsIgnoreAsciiCase("false"))
                    aDefaultPrinter = aPrinterName;

                aValue = aConfig.ReadKey( "Location" );
                aPrinter.m_aInfo.m_aLocation = OStringToOUString(aValue, RTL_TEXTENCODING_UTF8);

                aValue = aConfig.ReadKey( "Comment" );
                aPrinter.m_aInfo.m_aComment = OStringToOUString(aValue, RTL_TEXTENCODING_UTF8);

                aValue = aConfig.ReadKey( "Copies" );
                if (!aValue.isEmpty())
                    aPrinter.m_aInfo.m_nCopies = aValue.toInt32();

                aValue = aConfig.ReadKey( "Orientation" );
                if (!aValue.isEmpty())
                    aPrinter.m_aInfo.m_eOrientation = aValue.equalsIgnoreAsciiCase("Landscape") ? orientation::Landscape : orientation::Portrait;

                aValue = aConfig.ReadKey( "MarginAdjust" );
                if (!aValue.isEmpty())
                {
                    sal_Int32 nIdx {0};
                    aPrinter.m_aInfo.m_nLeftMarginAdjust = aValue.getToken(0, ',', nIdx).toInt32();
                    aPrinter.m_aInfo.m_nRightMarginAdjust = aValue.getToken(0, ',', nIdx).toInt32();
                    aPrinter.m_aInfo.m_nTopMarginAdjust = aValue.getToken(0, ',', nIdx).toInt32();
                    aPrinter.m_aInfo.m_nBottomMarginAdjust = aValue.getToken(0, ',', nIdx).toInt32();
                }

                aValue = aConfig.ReadKey( "ColorDepth" );
                if (!aValue.isEmpty())
                    aPrinter.m_aInfo.m_nColorDepth = aValue.toInt32();

                aValue = aConfig.ReadKey( "ColorDevice" );
                if (!aValue.isEmpty())
                    aPrinter.m_aInfo.m_nColorDevice = aValue.toInt32();

                aValue = aConfig.ReadKey( "PSLevel" );
                if (!aValue.isEmpty())
                    aPrinter.m_aInfo.m_nPSLevel = aValue.toInt32();

                aValue = aConfig.ReadKey( "PDFDevice" );
                if (!aValue.isEmpty())
                    aPrinter.m_aInfo.m_nPDFDevice = aValue.toInt32();

                // now iterate over all keys to extract multi key information:
                // 1. PPDContext information
                for( int nKey = 0; nKey < aConfig.GetKeyCount(); ++nKey )
                {
                    OString aKey( aConfig.GetKeyName( nKey ) );
                    if( aKey.startsWith("PPD_") && aPrinter.m_aInfo.m_pParser )
                    {
                        aValue = aConfig.ReadKey( aKey );
                        const PPDKey* pKey = aPrinter.m_aInfo.m_pParser->getKey(OStringToOUString(aKey.copy(4), RTL_TEXTENCODING_ISO_8859_1));
                        if( pKey )
                        {
                            aPrinter.m_aInfo.m_aContext.
                            setValue( pKey,
                                      aValue == "*nil" ? nullptr : pKey->getValue(OStringToOUString(aValue, RTL_TEXTENCODING_ISO_8859_1)),
                                      true );
                        }
                    }
                }

                setDefaultPaper( aPrinter.m_aInfo.m_aContext );

                // if it's a "Generic Printer", apply defaults from config...
                aPrinter.m_aInfo.resolveDefaultBackend();

                // finally insert printer
                FileBase::getFileURLFromSystemPath( aFile.PathToFileName(), aPrinter.m_aFile );
                std::unordered_map< OUString, Printer >::const_iterator find_it =
                m_aPrinters.find( aPrinterName );
                if( find_it != m_aPrinters.end() )
                {
                    aPrinter.m_aAlternateFiles = find_it->second.m_aAlternateFiles;
                    aPrinter.m_aAlternateFiles.insert( find_it->second.m_aFile );
                }
                m_aPrinters[ aPrinterName ] = aPrinter;
            }
        }
    }

    // set default printer
    if( !m_aPrinters.empty() )
    {
        if( m_aPrinters.find( aDefaultPrinter ) == m_aPrinters.end() )
            aDefaultPrinter = m_aPrinters.begin()->first;
    }
    else
        aDefaultPrinter.clear();
    m_aDefaultPrinter = aDefaultPrinter;

    if( m_eType != Type::Default )
        return;

    // add a default printer for every available print queue
    // merge paper default printer, all else from global defaults
    PrinterInfo aMergeInfo( m_aGlobalDefaults );
    aMergeInfo.m_aDriverName    = "SGENPRT";
    aMergeInfo.m_aFeatures      = "autoqueue";

    if( !m_aDefaultPrinter.isEmpty() )
    {
        PrinterInfo aDefaultInfo( getPrinterInfo( m_aDefaultPrinter ) );

        const PPDKey* pDefKey           = aDefaultInfo.m_pParser->getKey( OUString( "PageSize" ) );
        const PPDKey* pMergeKey         = aMergeInfo.m_pParser->getKey( OUString( "PageSize" ) );
        const PPDValue* pDefValue       = aDefaultInfo.m_aContext.getValue( pDefKey );
        const PPDValue* pMergeValue     = pMergeKey ? pMergeKey->getValue( pDefValue->m_aOption ) : nullptr;
        if( pMergeKey && pMergeValue )
            aMergeInfo.m_aContext.setValue( pMergeKey, pMergeValue );
    }

    if( m_pQueueInfo && m_pQueueInfo->hasChanged() )
    {
        m_aSystemPrintCommand = m_pQueueInfo->getCommand();
        m_pQueueInfo->getSystemQueues( m_aSystemPrintQueues );
        m_pQueueInfo.reset();
    }
    for (auto const& printQueue : m_aSystemPrintQueues)
    {
        OUString aPrinterName( "<" );
        aPrinterName += printQueue.m_aQueue;
        aPrinterName += ">";

        if( m_aPrinters.find( aPrinterName ) != m_aPrinters.end() )
            // probably user made this one permanent
            continue;

        OUString aCmd( m_aSystemPrintCommand );
        aCmd = aCmd.replaceAll( "(PRINTER)", printQueue.m_aQueue );

        Printer aPrinter;

        // initialize to merged defaults
        aPrinter.m_aInfo = aMergeInfo;
        aPrinter.m_aInfo.m_aPrinterName     = aPrinterName;
        aPrinter.m_aInfo.m_aCommand         = aCmd;
        aPrinter.m_aInfo.m_aComment         = printQueue.m_aComment;
        aPrinter.m_aInfo.m_aLocation        = printQueue.m_aLocation;

        m_aPrinters[ aPrinterName ] = aPrinter;
    }
}

void PrinterInfoManager::listPrinters( ::std::vector< OUString >& rVector ) const
{
    rVector.clear();
    for (auto const& printer : m_aPrinters)
        rVector.push_back(printer.first);
}

const PrinterInfo& PrinterInfoManager::getPrinterInfo( const OUString& rPrinter ) const
{
    static PrinterInfo aEmptyInfo;
    std::unordered_map< OUString, Printer >::const_iterator it = m_aPrinters.find( rPrinter );

    SAL_WARN_IF( it == m_aPrinters.end(), "vcl", "Do not ask for info about nonexistent printers" );

    return it != m_aPrinters.end() ? it->second.m_aInfo : aEmptyInfo;
}

bool PrinterInfoManager::checkFeatureToken( const OUString& rPrinterName, const char* pToken ) const
{
    const PrinterInfo& rPrinterInfo( getPrinterInfo( rPrinterName ) );
    sal_Int32 nIndex = 0;
    while( nIndex != -1 )
    {
        OUString aOuterToken = rPrinterInfo.m_aFeatures.getToken( 0, ',', nIndex );
        if( aOuterToken.getToken( 0, '=' ).equalsIgnoreAsciiCaseAscii( pToken ) )
            return true;
    }
    return false;
}

FILE* PrinterInfoManager::startSpool( const OUString& rPrintername, bool bQuickCommand )
{
    const PrinterInfo&   rPrinterInfo   = getPrinterInfo (rPrintername);
    const OUString& rCommand       = (bQuickCommand && !rPrinterInfo.m_aQuickCommand.isEmpty() ) ?
                                          rPrinterInfo.m_aQuickCommand : rPrinterInfo.m_aCommand;
    OString aShellCommand  = OUStringToOString (rCommand, RTL_TEXTENCODING_ISO_8859_1);
    aShellCommand += " 2>/dev/null";

    return popen (aShellCommand.getStr(), "w");
}

bool PrinterInfoManager::endSpool( const OUString& /*rPrintername*/, const OUString& /*rJobTitle*/, FILE* pFile, const JobData& /*rDocumentJobData*/, bool /*bBanner*/, const OUString& /*rFaxNumber*/ )
{
    return (0 == pclose( pFile ));
}

void PrinterInfoManager::setupJobContextData( JobData& rData )
{
    std::unordered_map< OUString, Printer >::iterator it =
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

    const PPDKey* pPageSizeKey = rContext.getParser()->getKey( OUString( "PageSize" ) );
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
    const PPDValue* pPaperVal = nullptr;
    int nValues = pPageSizeKey->countValues();
    for( int i = 0; i < nValues && ! pPaperVal; i++ )
    {
        const PPDValue* pVal = pPageSizeKey->getValue( i );
        if( pVal->m_aOption.equalsIgnoreAsciiCase( m_aSystemDefaultPaper ) )
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

SystemQueueInfo::SystemQueueInfo() :
    m_bChanged( false )
{
    create();
}

SystemQueueInfo::~SystemQueueInfo()
{
    static const char* pNoSyncDetection = getenv( "SAL_DISABLE_SYNCHRONOUS_PRINTER_DETECTION" );
    if( ! pNoSyncDetection || !*pNoSyncDetection )
        join();
    else
        terminate();
}

bool SystemQueueInfo::hasChanged() const
{
    MutexGuard aGuard( m_aMutex );
    bool bChanged = m_bChanged;
    return bChanged;
}

void SystemQueueInfo::getSystemQueues( std::vector< PrinterInfoManager::SystemPrintQueue >& rQueues )
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
typedef void(* tokenHandler)(const std::vector< OString >&,
                std::vector< PrinterInfoManager::SystemPrintQueue >&,
                const SystemCommandParameters*);

struct SystemCommandParameters
{
    const char*     pQueueCommand;
    const char*     pPrintCommand;
    const char*     pForeToken;
    const char*     pAftToken;
    unsigned int const    nForeTokenCount;
    tokenHandler const    pHandler;
};

#if ! (defined(LINUX) || defined(NETBSD) || defined(FREEBSD) || defined(OPENBSD))
static void lpgetSysQueueTokenHandler(
    const std::vector< OString >& i_rLines,
    std::vector< PrinterInfoManager::SystemPrintQueue >& o_rQueues,
    const SystemCommandParameters* )
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    std::unordered_set< OUString > aUniqueSet;
    std::unordered_set< OUString > aOnlySet;
    aUniqueSet.insert( OUString( "_all" ) );
    aUniqueSet.insert( OUString( "_default" ) );

    // the eventual "all" attribute of the "_all" queue tells us, which
    // printers are to be used for this user at all

    // find _all: line
    OString aAllLine( "_all:" );
    OString aAllAttr( "all=" );
    auto it = std::find_if(i_rLines.begin(), i_rLines.end(),
        [&aAllLine](const OString& rLine) { return rLine.indexOf( aAllLine, 0 ) == 0; });
    if( it != i_rLines.end() )
    {
        // now find the "all" attribute
        ++it;
        it = std::find_if(it, i_rLines.end(),
            [&aAllAttr](const OString& rLine) { return WhitespaceToSpace( rLine ).startsWith( aAllAttr ); });
        if( it != i_rLines.end() )
        {
            // insert the comma separated entries into the set of printers to use
            OString aClean( WhitespaceToSpace( *it ) );
            sal_Int32 nPos = aAllAttr.getLength();
            while( nPos != -1 )
            {
                OString aTok( aClean.getToken( 0, ',', nPos ) );
                if( !aTok.isEmpty() )
                    aOnlySet.insert( OStringToOUString( aTok, aEncoding ) );
            }
        }
    }

    bool bInsertAttribute = false;
    OString aDescrStr( "description=" );
    OString aLocStr( "location=" );
    for (auto const& line : i_rLines)
    {
        sal_Int32 nPos = 0;
        // find the begin of a new printer section
        nPos = line.indexOf( ':', 0 );
        if( nPos != -1 )
        {
            OUString aSysQueue( OStringToOUString( line.copy( 0, nPos ), aEncoding ) );
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
            nPos = line.indexOf( aDescrStr, 0 );
            if( nPos != -1 )
            {
                OString aComment( WhitespaceToSpace( line.copy(nPos+12) ) );
                if( !aComment.isEmpty() )
                    o_rQueues.back().m_aComment = OStringToOUString(aComment, aEncoding);
                continue;
            }
            // look for "location" attribute, inser as location
            nPos = line.indexOf( aLocStr, 0 );
            if( nPos != -1 )
            {
                OString aLoc( WhitespaceToSpace( line.copy(nPos+9) ) );
                if( !aLoc.isEmpty() )
                    o_rQueues.back().m_aLocation = OStringToOUString(aLoc, aEncoding);
                continue;
            }
        }
    }
}
#endif
static void standardSysQueueTokenHandler(
    const std::vector< OString >& i_rLines,
    std::vector< PrinterInfoManager::SystemPrintQueue >& o_rQueues,
    const SystemCommandParameters* i_pParms)
{
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    std::unordered_set< OUString > aUniqueSet;
    OString aForeToken( i_pParms->pForeToken );
    OString aAftToken( i_pParms->pAftToken );
    /* Normal Unix print queue discovery, also used for Darwin 5 LPR printing
    */
    for (auto const& line : i_rLines)
    {
        sal_Int32 nPos = 0;

        // search for a line describing a printer:
        // find if there are enough tokens before the name
        for( unsigned int i = 0; i < i_pParms->nForeTokenCount && nPos != -1; i++ )
        {
            nPos = line.indexOf( aForeToken, nPos );
            if( nPos != -1 && line.getLength() >= nPos+aForeToken.getLength() )
                nPos += aForeToken.getLength();
        }
        if( nPos != -1 )
        {
            // find if there is the token after the queue
            sal_Int32 nAftPos = line.indexOf( aAftToken, nPos );
            if( nAftPos != -1 )
            {
                // get the queue name between fore and aft tokens
                OUString aSysQueue( OStringToOUString( line.copy( nPos, nAftPos - nPos ), aEncoding ) );
                // do not insert duplicates (e.g. lpstat tends to produce such lines)
                if( aUniqueSet.find( aSysQueue ) == aUniqueSet.end() )
                {
                    o_rQueues.emplace_back( );
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
    #if defined(LINUX) || defined(NETBSD) || defined(FREEBSD) || defined(OPENBSD)
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
    osl_setThreadName("LPR psp::SystemQueueInfo");

    char pBuffer[1024];
    std::vector< OString > aLines;

    /* Discover which command we can use to get a list of all printer queues */
    for(const auto & rParm : aParms)
    {
        aLines.clear();
        OStringBuffer aCmdLine( 128 );
        aCmdLine.append( rParm.pQueueCommand );
        #if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "trying print queue command \"%s\" ... ", rParm.pQueueCommand );
        #endif
        aCmdLine.append( " 2>/dev/null" );
        FILE *pPipe;
        if( (pPipe = popen( aCmdLine.getStr(), "r" )) )
        {
            while( fgets( pBuffer, 1024, pPipe ) )
                aLines.emplace_back( pBuffer );
            if( ! pclose( pPipe ) )
            {
                std::vector< PrinterInfoManager::SystemPrintQueue > aSysPrintQueues;
                rParm.pHandler( aLines, aSysPrintQueues, &rParm );
                MutexGuard aGuard( m_aMutex );
                m_bChanged  = true;
                m_aQueues   = aSysPrintQueues;
                m_aCommand  = OUString::createFromAscii( rParm.pPrintCommand );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
