/*************************************************************************
 *
 *  $RCSfile: ppdparser.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 10:08:39 $
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
#include <stdlib.h>
#include <stdio.h>

#ifndef __SGI_STL_HASH_MAP
#include <hash_map>
#endif
// define a hash for PPDKey
namespace psp { class PPDKey; }

namespace  _STL {
struct hash< const psp::PPDKey* >
{
    size_t operator()( const psp::PPDKey * pKey) const
    { return (size_t)pKey; }
};
}

#include <psprint/ppdparser.hxx>
#include <tools/debug.hxx>
#include <psprint/strhelper.hxx>
#include <psprint/helper.hxx>
#include <cupsmgr.hxx>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <osl/mutex.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.h>

#define PRINTER_PPDDIR "driver"

using namespace psp;
using namespace rtl;

#undef DBG_ASSERT
#if defined DBG_UTIL || (OSL_DEBUG_LEVEL > 1)
#define BSTRING(x) ByteString( x, gsl_getSystemTextEncoding() )
#define DBG_ASSERT( x, y ) { if( ! (x) ) fprintf( stderr, (y) ); }
#else
#define DBG_ASSERT( x, y )
#endif

std::list< PPDParser* > PPDParser::aAllParsers;
std::hash_map< OUString, OUString, OUStringHash >* PPDParser::pAllPPDFiles = NULL;
static String aEmptyString;

void PPDParser::scanPPDDir( const String& rDir )
{
    static const sal_Char* pSuffixes[] = { "PS", "PPD" };
    const int nSuffixes = sizeof(pSuffixes)/sizeof(pSuffixes[0]);

    osl::Directory aDir( rDir );
    aDir.open();
    osl::DirectoryItem aItem;

    INetURLObject aPPDDir(rDir);
    while( aDir.getNextItem( aItem ) == osl::FileBase::E_None )
    {
        osl::FileStatus aStatus( FileStatusMask_FileName            |
                                 FileStatusMask_Type
                                 );
        if( aItem.getFileStatus( aStatus ) == osl::FileBase::E_None &&
            ( aStatus.getFileType() == osl::FileStatus::Regular ||
              aStatus.getFileType() == osl::FileStatus::Link ) )
        {
            INetURLObject aPPDFile = aPPDDir;
            aPPDFile.Append( aStatus.getFileName() );
            String aExt = aPPDFile.getExtension();
            // match extension
            for( int nSuffix = 0; nSuffix < nSuffixes; nSuffix++ )
            {
                if( aExt.EqualsIgnoreCaseAscii( pSuffixes[nSuffix] ) )
                {
                    (*pAllPPDFiles)[ aPPDFile.getBase() ] = aPPDFile.PathToFileName();
                    break;
                }
            }
        }
    }
    aDir.close();
}

void PPDParser::initPPDFiles()
{
    if( pAllPPDFiles )
        return;

    pAllPPDFiles = new std::hash_map< OUString, OUString, OUStringHash >();

    // check installation directories
    std::list< OUString > aPathList;
    psp::getPrinterPathList( aPathList, PRINTER_PPDDIR );
    for( std::list< OUString >::const_iterator ppd_it = aPathList.begin(); ppd_it != aPathList.end(); ++ppd_it )
    {
        INetURLObject aPPDDir( *ppd_it, INET_PROT_FILE, INetURLObject::ENCODE_ALL );
        scanPPDDir( aPPDDir.GetMainURL( INetURLObject::NO_DECODE ) );
    }
    if( pAllPPDFiles->find( OUString( RTL_CONSTASCII_USTRINGPARAM( "SGENPRT" ) ) ) == pAllPPDFiles->end() )
    {
        // last try: search in directory of executable (mainly for setup)
        OUString aExe;
        if( osl_getExecutableFile( &aExe.pData ) == osl_Process_E_None )
        {
            INetURLObject aDir( aExe );
            aDir.removeSegment();
#ifdef DEBUG
            fprintf( stderr, "scanning last chance dir: %s\n", OUStringToOString( aDir.GetMainURL( INetURLObject::NO_DECODE ), osl_getThreadTextEncoding() ).getStr() );
#endif
            scanPPDDir( aDir.GetMainURL( INetURLObject::NO_DECODE ) );
#ifdef DEBUG
            fprintf( stderr, "SGENPRT %s\n", pAllPPDFiles->find( OUString( RTL_CONSTASCII_USTRINGPARAM( "SGENPRT" ) ) ) == pAllPPDFiles->end() ? "not found" : "found" );
#endif
        }
    }
}

String PPDParser::getPPDFile( const String& rFile )
{
    INetURLObject aPPD( rFile, INET_PROT_FILE, INetURLObject::ENCODE_ALL );
    // someone might enter a full qualified name here
    SvFileStream aStream( aPPD.PathToFileName(), STREAM_READ );
    if( ! aStream.IsOpen() )
    {
        initPPDFiles();
        // some PPD files contain dots beside the extension, so try name first,
        // base after that
        std::hash_map< OUString, OUString, OUStringHash >::const_iterator it =
            pAllPPDFiles->find( aPPD.getName() );
        if( it == pAllPPDFiles->end() )
            it = pAllPPDFiles->find( aPPD.getBase() );
        if( it == pAllPPDFiles->end() )
        {
            // a new file ? rehash
            delete pAllPPDFiles; pAllPPDFiles = NULL;
            initPPDFiles();
            // aPPD is already the file name minus the extension
            it = pAllPPDFiles->find( aPPD.getName() );
            if( it == pAllPPDFiles->end() )
                it = pAllPPDFiles->find( aPPD.getBase() );
            // note this is optimized for office start where
            // no new files occur and initPPDFiles is called only once
        }
        if( it != pAllPPDFiles->end() )
            aStream.Open( it->second, STREAM_READ );
    }

    String aRet;
    if( aStream.IsOpen() )
    {
        ByteString aLine;
        aStream.ReadLine( aLine );
        if( aLine.Search( "*PPD-Adobe" ) == 0 )
            aRet = aStream.GetFileName();
        else
        {
            // our *Include hack does usually not begin
            // with *PPD-Adobe, so try some lines for *Include
            int nLines = 10;
            while( aLine.Search( "*Include" ) != 0 && --nLines )
                aStream.ReadLine( aLine );
            if( nLines )
                aRet = aStream.GetFileName();
        }
    }

    return aRet;
}

String PPDParser::getPPDPrinterName( const String& rFile )
{
    String aPath = getPPDFile( rFile );
    String aName;

    // read in the file
    SvFileStream aStream( aPath, STREAM_READ );
    if( aStream.IsOpen() )
    {
        String aCurLine;
        while( ! aStream.IsEof() && aStream.IsOpen() )
        {
            ByteString aByteLine;
            aStream.ReadLine( aByteLine );
            aCurLine = String( aByteLine, RTL_TEXTENCODING_MS_1252 );
            if( aCurLine.CompareIgnoreCaseToAscii( "*include:", 9 ) == COMPARE_EQUAL )
            {
                aCurLine.Erase( 0, 9 );
                aCurLine.EraseLeadingChars( ' ' );
                aCurLine.EraseTrailingChars( ' ' );
                aCurLine.EraseLeadingChars( '\t' );
                aCurLine.EraseTrailingChars( '\t' );
                aCurLine.EraseTrailingChars( '\r' );
                aCurLine.EraseTrailingChars( '\n' );
                aCurLine.EraseLeadingChars( '"' );
                aCurLine.EraseTrailingChars( '"' );
                aStream.Close();
                aStream.Open( getPPDFile( aCurLine ), STREAM_READ );
                continue;
            }
            if( aCurLine.CompareToAscii( "*ModelName:", 11 ) == COMPARE_EQUAL )
            {
                aName = aCurLine.GetToken( 1, '"' );
                break;
            }
            else if( aCurLine.CompareToAscii( "*NickName:", 10 ) == COMPARE_EQUAL )
                aName = aCurLine.GetToken( 1, '"' );
        }
    }
    return aName;
}

const PPDParser* PPDParser::getParser( const String& rFile )
{
    static ::osl::Mutex aMutex;
    ::osl::Guard< ::osl::Mutex > aGuard( aMutex );

    String aFile = rFile;
    if( rFile.CompareToAscii( "CUPS:", 5 ) != COMPARE_EQUAL )
        aFile = getPPDFile( rFile );
    if( ! aFile.Len() )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "Could not get printer PPD file \"%s\" !\n", OUStringToOString( rFile, osl_getThreadTextEncoding() ).getStr() );
#endif
        return NULL;
    }

    for( ::std::list< PPDParser* >::const_iterator it = aAllParsers.begin(); it != aAllParsers.end(); ++it )
        if( (*it)->m_aFile == aFile )
            return *it;

    PPDParser* pNewParser = NULL;
    if( aFile.CompareToAscii( "CUPS:", 5 ) != COMPARE_EQUAL )
        pNewParser = new PPDParser( aFile );
    else
    {
        PrinterInfoManager& rMgr = PrinterInfoManager::get();
        if( rMgr.getType() == PrinterInfoManager::CUPS )
        {
            pNewParser = const_cast<PPDParser*>(static_cast<CUPSManager&>(rMgr).createCUPSParser( aFile ));
        }
    }
    if( pNewParser )
    {
        // this may actually be the SGENPRT parser,
        // so ensure uniquness here
        aAllParsers.remove( pNewParser );
        // insert new parser to list
        aAllParsers.push_front( pNewParser );
    }
    return pNewParser;
}

void PPDParser::freeAll()
{
    while( aAllParsers.begin() != aAllParsers.end() )
    {
        delete aAllParsers.front();
        aAllParsers.pop_front();
    }
    delete pAllPPDFiles;
    pAllPPDFiles = NULL;
}

PPDParser::PPDParser( const String& rFile ) :
        m_aFile( rFile ),
        m_bType42Capable( false ),
        m_pDefaultImageableArea( NULL ),
        m_pImageableAreas( NULL ),
        m_pDefaultPaperDimension( NULL ),
        m_pPaperDimensions( NULL ),
        m_pDefaultInputSlot( NULL ),
        m_pInputSlots( NULL ),
        m_pDefaultResolution( NULL ),
        m_pResolutions( NULL ),
        m_pDefaultDuplexType( NULL ),
        m_pDuplexTypes( NULL ),
        m_pFontList( NULL )
{
    // read in the file
    ::std::list< String > aLines;
    SvFileStream aStream( m_aFile, STREAM_READ );
    if( aStream.IsOpen() )
    {
        String aCurLine;
        while( ! aStream.IsEof() )
        {
            ByteString aByteLine;
            aStream.ReadLine( aByteLine );
            aCurLine = String( aByteLine, RTL_TEXTENCODING_MS_1252 );
            if( aCurLine.CompareIgnoreCaseToAscii( "*include:", 9 ) == COMPARE_EQUAL )
            {
                aCurLine.Erase( 0, 9 );
                aCurLine.EraseLeadingChars( ' ' );
                aCurLine.EraseTrailingChars( ' ' );
                aCurLine.EraseLeadingChars( '\t' );
                aCurLine.EraseTrailingChars( '\t' );
                aCurLine.EraseTrailingChars( '\r' );
                aCurLine.EraseTrailingChars( '\n' );
                aCurLine.EraseLeadingChars( '"' );
                aCurLine.EraseTrailingChars( '"' );
                aStream.Close();
                aStream.Open( getPPDFile( aCurLine ), STREAM_READ );
                continue;
            }
            aLines.push_back( aCurLine );
        }
    }
    aStream.Close();

    // now get the Values
    parse( aLines );
#if OSL_DEBUG_LEVEL > 2
    fprintf( stderr, "acquired %d Keys from PPD %s:\n", m_aKeys.size(), BSTRING( m_aFile ).GetBuffer() );
    for( PPDParser::hash_type::const_iterator it = m_aKeys.begin(); it != m_aKeys.end(); ++it )
    {
        const PPDKey* pKey = it->second;
        char* pSetupType = "<unknown>";
        switch( pKey->m_eSetupType )
        {
            case PPDKey::ExitServer:        pSetupType = "ExitServer";break;
            case PPDKey::Prolog:            pSetupType = "Prolog";break;
            case PPDKey::DocumentSetup: pSetupType = "DocumentSetup";break;
            case PPDKey::PageSetup:     pSetupType = "PageSetup";break;
            case PPDKey::JCLSetup:          pSetupType = "JCLSetup";break;
            case PPDKey::AnySetup:          pSetupType = "AnySetup";break;
            default: break;
        };
        fprintf( stderr, "\t\"%s\" (\"%s\") (%d values) OrderDependency: %d %s\n",
                 BSTRING( pKey->getKey() ).GetBuffer(),
                 BSTRING( pKey->m_aUITranslation ).GetBuffer(),
                 pKey->countValues(),
                 pKey->m_nOrderDependency,
                 pSetupType );
        for( int j = 0; j < pKey->countValues(); j++ )
        {
            fprintf( stderr, "\t\t" );
            const PPDValue* pValue = pKey->getValue( j );
            if( pValue == pKey->m_pDefaultValue )
                fprintf( stderr, "(Default:) " );
            char* pVType = "<unknown>";
            switch( pValue->m_eType )
            {
                case eInvocation:       pVType = "invocation";break;
                case eQuoted:           pVType = "quoted";break;
                case eString:           pVType = "string";break;
                case eSymbol:           pVType = "symbol";break;
                case eNo:               pVType = "no";break;
                default: break;
            };
            fprintf( stderr, "option: \"%s\" (\"%s\"), value: type %s \"%s\" (\"%s\")\n",
                     BSTRING( pValue->m_aOption ).GetBuffer(),
                     BSTRING( pValue->m_aOptionTranslation ).GetBuffer(),
                     pVType,
                     BSTRING( pValue->m_aValue ).GetBuffer(),
                     BSTRING( pValue->m_aValueTranslation ).GetBuffer() );
        }
    }
    fprintf( stderr, "constraints: (%d found)\n", m_aConstraints.size() );
    for( std::list< PPDConstraint >::const_iterator cit = m_aConstraints.begin(); cit != m_aConstraints.end(); ++cit )
    {
        fprintf( stderr, "*\"%s\" \"%s\" *\"%s\" \"%s\"\n",
                 BSTRING( cit->m_pKey1->getKey() ).GetBuffer(),
                 cit->m_pOption1 ? BSTRING( cit->m_pOption1->m_aOption ).GetBuffer() : "<nil>",
                 BSTRING( cit->m_pKey2->getKey() ).GetBuffer(),
                 cit->m_pOption2 ? BSTRING( cit->m_pOption2->m_aOption ).GetBuffer() : "<nil>"
                 );
    }
#endif

    // fill in shortcuts
    const PPDKey* pKey;

    m_pImageableAreas = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "ImageableArea" ) ) );
    if( m_pImageableAreas )
        m_pDefaultImageableArea = m_pImageableAreas->getDefaultValue();
    DBG_ASSERT( m_pImageableAreas, "Warning: no ImageableArea in PPD\n" );
    DBG_ASSERT( m_pDefaultImageableArea, "Warning: no DefaultImageableArea in PPD\n" );

    m_pPaperDimensions = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "PaperDimension" ) ) );
    if( m_pPaperDimensions )
        m_pDefaultPaperDimension = m_pPaperDimensions->getDefaultValue();
    DBG_ASSERT( m_pPaperDimensions, "Warning: no PaperDimension in PPD\n" );
    DBG_ASSERT( m_pDefaultPaperDimension, "Warning: no DefaultPaperDimension in PPD\n" );

    m_pResolutions = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Resolution" ) ) );
    if( m_pResolutions )
        m_pDefaultResolution = m_pResolutions->getDefaultValue();
    DBG_ASSERT( m_pResolutions, "Warning: no Resolution in PPD\n" );
    DBG_ASSERT( m_pDefaultResolution, "Warning: no DefaultResolution in PPD\n" );

    m_pInputSlots = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "InputSlot" ) ) );
    if( m_pInputSlots )
        m_pDefaultInputSlot = m_pInputSlots->getDefaultValue();
    DBG_ASSERT( m_pPaperDimensions, "Warning: no InputSlot in PPD\n" );
    DBG_ASSERT( m_pDefaultPaperDimension, "Warning: no DefaultInputSlot in PPD\n" );

    m_pDuplexTypes = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Duplex" ) ) );
    if( m_pDuplexTypes )
        m_pDefaultDuplexType = m_pDuplexTypes->getDefaultValue();

    m_pFontList = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Font" ) ) );
    DBG_ASSERT( m_pFontList, "Warning: no Font in PPD\n" );

    // fill in direct values
    if( (pKey = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "ModelName" ) ) )) )
        m_aPrinterName = pKey->getValue( 0 )->m_aValue;
    if( (pKey = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "NickName" ) ) )) )
        m_aNickName = pKey->getValue( 0 )->m_aValue;
    if( (pKey = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "ColorDevice" ) ) )) )
        m_bColorDevice = pKey->getValue( 0 )->m_aValue.CompareIgnoreCaseToAscii( "true", 4 ) == COMPARE_EQUAL ? true : false;

    if( (pKey = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "LanguageLevel" ) ) )) )
        m_nLanguageLevel = pKey->getValue( 0 )->m_aValue.ToInt32();
    if( (pKey = getKey( String( RTL_CONSTASCII_USTRINGPARAM( "TTRasterizer" ) ) )) )
        m_bType42Capable = pKey->getValue( 0 )->m_aValue.EqualsIgnoreCaseAscii( "Type42" ) ? true : false;

#ifdef MACOSX
    // Many Mac OS X PPDs for inkjet printers simply use CUPS to convert the output to a PDF,
    // and to take advantage of that we wish to embed fonts for these printers even if its
    // minimal CUPS PPD doesn't have TTRasterizer.
    // To determine if this printer is supposed to take PDF input, look for the "cupsFilter" key,
    // and if it exists, for the string "application/pdf" which specifies the CUPS filter to use for it
    // If present, we assume that we can embed TrueType fonts and that the CUPS filter will take care
    // of rasterizing them appropriately.
    if ( m_bType42Capable == false )
    {
        if ( pKey = getKey( String(RTL_CONSTASCII_USTRINGPARAM("cupsFilter")) ) )
        {
            ByteString    aCupsFilterString( pKey->getValue( 0 )->m_aValue, RTL_TEXTENCODING_ISO_8859_1 );
            if ( strstr(aCupsFilterString.GetBuffer(), "application/pdf") > 0 )
                m_bType42Capable = true;
        }
    }
#endif
}

PPDParser::~PPDParser()
{
    for( PPDParser::hash_type::iterator it = m_aKeys.begin(); it != m_aKeys.end(); ++it )
        delete it->second;
}

void PPDParser::insertKey( const String& rKey, PPDKey* pKey )
{
    m_aKeys[ rKey ] = pKey;
    m_aOrderedKeys.push_back( pKey );
}

const PPDKey* PPDParser::getKey( int n ) const
{
    return ((unsigned int)n < m_aOrderedKeys.size() && n >= 0) ? m_aOrderedKeys[n] : NULL;
}

const PPDKey* PPDParser::getKey( const String& rKey ) const
{
    PPDParser::hash_type::const_iterator it = m_aKeys.find( rKey );
    return it != m_aKeys.end() ? it->second : NULL;
}

bool PPDParser::hasKey( const PPDKey* pKey ) const
{
    return
        pKey ?
        ( m_aKeys.find( pKey->getKey() ) != m_aKeys.end() ? true : false ) :
        false;
}

void PPDParser::parse( ::std::list< String >& rLines )
{
    PPDValue*   pValue  = NULL;
    PPDKey*     pKey    = NULL;

    ::std::list< String >::iterator line = rLines.begin();
    PPDParser::hash_type::const_iterator keyit;
    while( line != rLines.end() )
    {
        String aCurrentLine( *line );
        ++line;
        if( aCurrentLine.GetChar(0) != '*' )
            continue;
        if( aCurrentLine.GetChar(1) == '%' )
            continue;

        String aKey = GetCommandLineToken( 0, aCurrentLine.GetToken( 0, ':' ) );
        int nPos = aKey.Search( '/' );
        if( nPos != STRING_NOTFOUND )
            aKey.Erase( nPos );
        aKey.Erase( 0, 1 ); // remove the '*'

        if( aKey.EqualsAscii( "CloseUI" ) || aKey.EqualsAscii( "OpenGroup" ) || aKey.EqualsAscii( "CloseGroup" ) || aKey.EqualsAscii( "End" ) || aKey.EqualsAscii( "OpenSubGroup" ) || aKey.EqualsAscii( "CloseSubGroup" ) )
            continue;

        if( aKey.EqualsAscii( "OpenUI" ) )
        {
            parseOpenUI( aCurrentLine );
            continue;
        }
        else if( aKey.EqualsAscii( "OrderDependency" ) )
        {
            parseOrderDependency( aCurrentLine );
            continue;
        }
        else if( aKey.EqualsAscii( "UIConstraints" ) || aKey.EqualsAscii( "NonUIConstraints" ) )
            continue; // parsed in pass 2
        else if( aKey.EqualsAscii( "CustomPageSize" ) ) // currently not handled
            continue;

        // default values are parsed in pass 2
        if( aKey.CompareToAscii( "Default", 7 ) == COMPARE_EQUAL )
            continue;

        bool bQuery     = false;
        if( aKey.GetChar( 0 ) == '?' )
        {
            aKey.Erase( 0, 1 );
            bQuery = true;
        }

        keyit = m_aKeys.find( aKey );
        if( keyit == m_aKeys.end() )
        {
            pKey = new PPDKey( aKey );
            insertKey( aKey, pKey );
        }
        else
            pKey = keyit->second;

        String aOption;
        nPos = aCurrentLine.Search( ':' );
        if( nPos != STRING_NOTFOUND )
        {
            aOption = aCurrentLine.Copy( 1, nPos-1 );
            aOption = GetCommandLineToken( 1, aOption );
            int nTransPos = aOption.Search( '/' );
            if( nTransPos != STRING_NOTFOUND )
                aOption.Erase( nTransPos );
        }
        pValue = pKey->insertValue( aOption );
        if( ! pValue )
            continue;

        if( bQuery && pKey->m_bQueryValue == FALSE )
        {
            pKey->m_aQueryValue = *pValue;
            pKey->m_bQueryValue = true;
            pKey->eraseValue( pValue->m_aOption );
        }

        if( nPos == STRING_NOTFOUND )
        {
            // have a single main keyword
            pValue->m_eType = eNo;
            continue;
        }

        // found a colon, there may be an option
        String aLine = aCurrentLine.Copy( 1, nPos-1 );
        aLine = WhitespaceToSpace( aLine );
    #ifdef MACOSX
        // Some Epson PPDs use <20> to encodes spaces in UI strings
        aLine.SearchAndReplaceAllAscii( "<20>", String(RTL_CONSTASCII_USTRINGPARAM(" ")) );
    #endif
        int nTransPos = aLine.Search( '/' );
        if( nTransPos != STRING_NOTFOUND )
            pValue->m_aOptionTranslation = aLine.Copy( nTransPos+1 );

        // read in more lines if necessary for multiline values
        aLine = aCurrentLine.Copy( nPos+1 );
        while( ! ( aLine.GetTokenCount( '"' ) & 1 ) &&
               line != rLines.end() )
            // while there is an even number of tokens; that m_eans
            // an odd number of doubleqoutes
        {
            // copy the newlines also
            aLine += '\n';
            aLine += *line;
            ++line;
        }
        aLine = WhitespaceToSpace( aLine );

        // check for invocation or quoted value
        if( aLine.GetChar(0) == '"' )
        {
            aLine.Erase( 0, 1 );
            nTransPos = aLine.Search( '"' );
            pValue->m_aValue = aLine.Copy( 0, nTransPos );
            // after the second doublequote can follow a / and a translation
            pValue->m_aValueTranslation = aLine.Copy( nTransPos+2 );
            // check for quoted value
            if( pValue->m_aOption.Len() &&
                aKey.CompareToAscii( "JCL", 3 ) != COMPARE_EQUAL )
                pValue->m_eType = eInvocation;
            else
                pValue->m_eType = eQuoted;
            continue;
        }
        // check for symbol value
        if( aLine.GetChar(0) == '^' )
        {
            aLine.Erase( 0, 1 );
            pValue->m_aValue = aLine;
            pValue->m_eType = eSymbol;
            continue;
        }
        else
        {
            // must be a string value then
            // strictly this is false because string values
            // can contain any whitespace which is reduced
            // to one space by now
            // who cares ...
            nTransPos = aLine.Search( '/' );
            if( nTransPos == STRING_NOTFOUND )
                nTransPos = aLine.Len();
            pValue->m_aValue = aLine.Copy( 0, nTransPos );
            pValue->m_aValueTranslation = aLine.Copy( nTransPos+1 );
            pValue->m_eType = eString;
        }
    }

    // second pass: fill in defaults
    for( line = rLines.begin(); line != rLines.end(); ++line )
    {
        String aLine( *line );
        if( aLine.CompareToAscii( "*Default", 8 ) == COMPARE_EQUAL )
        {
            String aKey( aLine.Copy( 8 ) );
            USHORT nPos = aKey.Search( ':' );
            if( nPos != STRING_NOTFOUND )
            {
                aKey.Erase( nPos );
                String aOption( WhitespaceToSpace( aLine.Copy( nPos+9 ) ) );
                keyit = m_aKeys.find( aKey );
                if( keyit != m_aKeys.end() )
                {
                    PPDKey* pKey = keyit->second;
                    const PPDValue* pValue = pKey->getValue( aOption );
                    if( pKey->m_pDefaultValue == NULL )
                        pKey->m_pDefaultValue = pValue;
                }
                else
                {
                    // some PPDs contain defaults for keys that
                    // do not exist otherwise
                    // (example: DefaultResolution)
                    // so invent that key here and have a default value
                    pKey = new PPDKey( aKey );
                    PPDValue* pValue = pKey->insertValue( aOption );
                    pValue->m_eType = eInvocation; // or what ?
                    insertKey( aKey, pKey );
                }
            }
        }
        else if( aLine.CompareToAscii( "*UIConstraints", 14 ) == COMPARE_EQUAL  ||
                 aLine.CompareToAscii( "*NonUIConstraints", 17 ) == COMPARE_EQUAL )
            parseConstraint( aLine );

    }
}

void PPDParser::parseOpenUI( const String& rLine )
{
    String aTranslation;
    String aKey = rLine;

    int nPos = aKey.Search( ':' );
    if( nPos != STRING_NOTFOUND )
        aKey.Erase( nPos );
    nPos = aKey.Search( '/' );
    if( nPos != STRING_NOTFOUND )
    {
        aTranslation = aKey.Copy( nPos + 1 );
        aKey.Erase( nPos );
    }
    aKey = GetCommandLineToken( 1, aKey );
    aKey.Erase( 0, 1 );

    PPDParser::hash_type::const_iterator keyit = m_aKeys.find( aKey );
    PPDKey* pKey;
    if( keyit == m_aKeys.end() )
    {
        pKey = new PPDKey( aKey );
        insertKey( aKey, pKey );
    }
    else
        pKey = keyit->second;

    pKey->m_bUIOption = true;
    pKey->m_aUITranslation = aTranslation;

    String aValue = WhitespaceToSpace( rLine.GetToken( 1, ':' ) );
    if( aValue.CompareIgnoreCaseToAscii( "boolean" ) == COMPARE_EQUAL )
        pKey->m_eUIType = PPDKey::Boolean;
    else if( aValue.CompareIgnoreCaseToAscii( "pickmany" ) == COMPARE_EQUAL )
        pKey->m_eUIType = PPDKey::PickMany;
    else
        pKey->m_eUIType = PPDKey::PickOne;
}

void PPDParser::parseOrderDependency( const String& rLine )
{
    String aLine( rLine );
    int nPos = aLine.Search( ':' );
    if( nPos != STRING_NOTFOUND )
        aLine.Erase( 0, nPos+1 );

    int nOrder = GetCommandLineToken( 0, aLine ).ToInt32();
    String aSetup = GetCommandLineToken( 1, aLine );
    String aKey = GetCommandLineToken( 2, aLine );
    if( aKey.GetChar( 0 ) != '*' )
        return; // invalid order depency
    aKey.Erase( 0, 1 );

    PPDKey* pKey;
    PPDParser::hash_type::const_iterator keyit = m_aKeys.find( aKey );
    if( keyit == m_aKeys.end() )
    {
        pKey = new PPDKey( aKey );
        insertKey( aKey, pKey );
    }
    else
        pKey = keyit->second;

    pKey->m_nOrderDependency = nOrder;
    if( aSetup.EqualsAscii( "ExitServer" ) )
        pKey->m_eSetupType = PPDKey::ExitServer;
    else if( aSetup.EqualsAscii( "Prolog" ) )
        pKey->m_eSetupType = PPDKey::Prolog;
    else if( aSetup.EqualsAscii( "DocumentSetup" ) )
        pKey->m_eSetupType = PPDKey::DocumentSetup;
    else if( aSetup.EqualsAscii( "PageSetup" ) )
        pKey->m_eSetupType = PPDKey::PageSetup;
    else if( aSetup.EqualsAscii( "JCLSetup" ) )
        pKey->m_eSetupType = PPDKey::JCLSetup;
    else
        pKey->m_eSetupType = PPDKey::AnySetup;
}

void PPDParser::parseConstraint( const String& rLine )
{
    bool bFailed = false;

    String aLine( rLine );
    aLine.Erase( 0, rLine.Search( ':' )+1 );
    PPDConstraint aConstraint;
    int nTokens = GetCommandLineTokenCount( aLine );
    for( int i = 0; i < nTokens; i++ )
    {
        String aToken = GetCommandLineToken( i, aLine );
        if( aToken.GetChar( 0 ) == '*' )
        {
            aToken.Erase( 0, 1 );
            if( aConstraint.m_pKey1 )
                aConstraint.m_pKey2 = getKey( aToken );
            else
                aConstraint.m_pKey1 = getKey( aToken );
        }
        else
        {
            if( aConstraint.m_pKey2 )
            {
                if( ! ( aConstraint.m_pOption2 = aConstraint.m_pKey2->getValue( aToken ) ) )
                    bFailed = true;
            }
            else if( aConstraint.m_pKey1 )
            {
                if( ! ( aConstraint.m_pOption1 = aConstraint.m_pKey1->getValue( aToken ) ) )
                    bFailed = true;
            }
            else
                // constraint for nonexistent keys; this happens
                // e.g. in HP4PLUS3 (#75636#)
                bFailed = true;
        }
    }
    // there must be two keywords
    if( ! aConstraint.m_pKey1 || ! aConstraint.m_pKey2 || bFailed )
    {
#ifdef __DEBUG
        fprintf( stderr, "Warning: constraint \"%s\" is invalid\n", rLine.GetStr() );
#endif
    }
    else
        m_aConstraints.push_back( aConstraint );
}

const String& PPDParser::getDefaultPaperDimension() const
{
    if( m_pDefaultPaperDimension )
        return m_pDefaultPaperDimension->m_aOption;

    return aEmptyString;
}

bool PPDParser::getMargins(
                           const String& rPaperName,
                           int& rLeft, int& rRight,
                           int& rUpper, int& rLower ) const
{
    if( ! m_pImageableAreas || ! m_pPaperDimensions )
        return false;

    int nPDim=-1, nImArea=-1, i;
    for( i = 0; i < m_pImageableAreas->countValues(); i++ )
        if( rPaperName == m_pImageableAreas->getValue( i )->m_aOption )
            nImArea = i;
    for( i = 0; i < m_pPaperDimensions->countValues(); i++ )
        if( rPaperName == m_pPaperDimensions->getValue( i )->m_aOption )
            nPDim = i;
    if( nPDim == -1 || nImArea == -1 )
        return false;

    double ImLLx, ImLLy, ImURx, ImURy;
    double PDWidth, PDHeight;
    String aArea = m_pImageableAreas->getValue( nImArea )->m_aValue;
    ImLLx = StringToDouble( GetCommandLineToken( 0, aArea ) );
    ImLLy = StringToDouble( GetCommandLineToken( 1, aArea ) );
    ImURx = StringToDouble( GetCommandLineToken( 2, aArea ) );
    ImURy = StringToDouble( GetCommandLineToken( 3, aArea ) );
//  sscanf( m_pImageableAreas->getValue( nImArea )->m_aValue.GetStr(),
//          "%lg%lg%lg%lg", &ImLLx, &ImLLy, &ImURx, &ImURy );
    aArea = m_pPaperDimensions->getValue( nPDim )->m_aValue;
    PDWidth     = StringToDouble( GetCommandLineToken( 0, aArea ) );
    PDHeight    = StringToDouble( GetCommandLineToken( 1, aArea ) );
//  sscanf( m_pPaperDimensions->getValue( nPDim )->m_aValue.GetStr(),
//          "%lg%lg", &PDWidth, &PDHeight );
    rLeft  = (int)(ImLLx + 0.5);
    rLower = (int)(ImLLy + 0.5);
    rUpper = (int)(PDHeight - ImURy + 0.5);
    rRight = (int)(PDWidth - ImURx + 0.5);

    return true;
}

bool PPDParser::getPaperDimension(
                                  const String& rPaperName,
                                  int& rWidth, int& rHeight ) const
{
    if( ! m_pPaperDimensions )
        return false;

    int nPDim=-1;
    for( int i = 0; i < m_pPaperDimensions->countValues(); i++ )
        if( rPaperName == m_pPaperDimensions->getValue( i )->m_aOption )
            nPDim = i;
    if( nPDim == -1 )
        return false;

    double PDWidth, PDHeight;
    String aArea = m_pPaperDimensions->getValue( nPDim )->m_aValue;
    PDWidth     = StringToDouble( GetCommandLineToken( 0, aArea ) );
    PDHeight    = StringToDouble( GetCommandLineToken( 1, aArea ) );
    rHeight = (int)(PDHeight + 0.5);
    rWidth  = (int)(PDWidth + 0.5);

    return true;
}

const String& PPDParser::matchPaper( int nWidth, int nHeight ) const
{
    if( ! m_pPaperDimensions )
        return aEmptyString;

    int nPDim = -1;
    double PDWidth, PDHeight;
    double fSort = 2e36, fNewSort;

    for( int i = 0; i < m_pPaperDimensions->countValues(); i++ )
    {
        String aArea =  m_pPaperDimensions->getValue( i )->m_aValue;
        PDWidth     = StringToDouble( GetCommandLineToken( 0, aArea ) );
        PDHeight    = StringToDouble( GetCommandLineToken( 1, aArea ) );
        PDWidth     /= (double)nWidth;
        PDHeight    /= (double)nHeight;
        if( PDWidth >= 0.9      &&  PDWidth <= 1.1      &&
            PDHeight >= 0.9     &&  PDHeight <= 1.1         )
        {
            fNewSort =
                (1.0-PDWidth)*(1.0-PDWidth) + (1.0-PDHeight)*(1.0-PDHeight);
            if( fNewSort == 0.0 ) // perfect match
                return m_pPaperDimensions->getValue( i )->m_aOption;

            if( fNewSort < fSort )
            {
                fSort = fNewSort;
                nPDim = i;
            }
        }
    }

    static bool bDontSwap = false;
    if( nPDim == -1 && ! bDontSwap )
    {
        // swap portrait/landscape and try again
        bDontSwap = true;
        const String& rRet = matchPaper( nHeight, nWidth );
        bDontSwap = false;
        return rRet;
    }

    return nPDim != -1 ? m_pPaperDimensions->getValue( nPDim )->m_aOption : aEmptyString;
}

const String& PPDParser::getDefaultInputSlot() const
{
    if( m_pDefaultInputSlot )
        return m_pDefaultInputSlot->m_aValue;
    return aEmptyString;
}

const String& PPDParser::getSlot( int nSlot ) const
{
    if( ! m_pInputSlots )
        return aEmptyString;

    if( nSlot > 0 && nSlot < m_pInputSlots->countValues() )
        return m_pInputSlots->getValue( nSlot )->m_aOption;
    else if( m_pInputSlots->countValues() > 0 )
        return m_pInputSlots->getValue( (ULONG)0 )->m_aOption;

    return aEmptyString;
}

const String& PPDParser::getSlotCommand( int nSlot ) const
{
    if( ! m_pInputSlots )
        return aEmptyString;

    if( nSlot > 0 && nSlot < m_pInputSlots->countValues() )
        return m_pInputSlots->getValue( nSlot )->m_aValue;
    else if( m_pInputSlots->countValues() > 0 )
        return m_pInputSlots->getValue( (ULONG)0 )->m_aValue;

    return aEmptyString;
}

const String& PPDParser::getSlotCommand( const String& rSlot ) const
{
    if( ! m_pInputSlots )
        return aEmptyString;

    for( int i=0; i < m_pInputSlots->countValues(); i++ )
    {
        const PPDValue* pValue = m_pInputSlots->getValue( i );
        if( pValue->m_aOption == rSlot )
            return pValue->m_aValue;
    }
    return aEmptyString;
}

const String& PPDParser::getPaperDimension( int nPaperDimension ) const
{
    if( ! m_pPaperDimensions )
        return aEmptyString;

    if( nPaperDimension > 0 && nPaperDimension < m_pPaperDimensions->countValues() )
        return m_pPaperDimensions->getValue( nPaperDimension )->m_aOption;
    else if( m_pPaperDimensions->countValues() > 0 )
        return m_pPaperDimensions->getValue( (ULONG)0 )->m_aOption;

    return aEmptyString;
}

const String& PPDParser::getPaperDimensionCommand( int nPaperDimension ) const
{
    if( ! m_pPaperDimensions )
        return aEmptyString;

    if( nPaperDimension > 0 && nPaperDimension < m_pPaperDimensions->countValues() )
        return m_pPaperDimensions->getValue( nPaperDimension )->m_aValue;
    else if( m_pPaperDimensions->countValues() > 0 )
        return m_pPaperDimensions->getValue( (ULONG)0 )->m_aValue;

    return aEmptyString;
}

const String& PPDParser::getPaperDimensionCommand( const String& rPaperDimension ) const
{
    if( ! m_pPaperDimensions )
        return aEmptyString;

    for( int i=0; i < m_pPaperDimensions->countValues(); i++ )
    {
        const PPDValue* pValue = m_pPaperDimensions->getValue( i );
        if( pValue->m_aOption == rPaperDimension )
            return pValue->m_aValue;
    }
    return aEmptyString;
}

void PPDParser::getResolutionFromString(
                                        const String& rString,
                                        int& rXRes, int& rYRes ) const
{
    int nPos = 0, nDPIPos;

    rXRes = rYRes = 300;

    nDPIPos = rString.SearchAscii( "dpi" );
    if( nDPIPos != STRING_NOTFOUND )
    {
        if( ( nPos = rString.Search( 'x' ) ) != STRING_NOTFOUND )
        {
            rXRes = rString.Copy( 0, nPos ).ToInt32();
            rYRes = rString.GetToken( 1, 'x' ).Erase( nDPIPos - nPos - 1 ).ToInt32();
        }
        else
            rXRes = rYRes = rString.Copy( 0, nDPIPos ).ToInt32();
    }
}

void PPDParser::getDefaultResolution( int& rXRes, int& rYRes ) const
{
    if( m_pDefaultResolution )
    {
        getResolutionFromString( m_pDefaultResolution->m_aValue, rXRes, rYRes );
        return;
    }

    rXRes = 300;
    rYRes = 300;
}

int PPDParser::getResolutions() const
{
    if( ( ! m_pResolutions || m_pResolutions->countValues() == 0 ) &&
        m_pDefaultResolution )
        return 1;
    return m_pResolutions ? m_pResolutions->countValues() : 0;
}

void PPDParser::getResolution( int nNr, int& rXRes, int& rYRes ) const
{
    if( ( ! m_pResolutions || m_pResolutions->countValues() == 0 ) && m_pDefaultResolution && nNr == 0 )
    {
        getDefaultResolution( rXRes, rYRes );
        return;
    }
    if( ! m_pResolutions )
        return;

    getResolutionFromString( m_pResolutions->getValue( nNr )->m_aOption,
                             rXRes, rYRes );
}

const String& PPDParser::getResolutionCommand( int nXRes, int nYRes ) const
{
    if( ( ! m_pResolutions || m_pResolutions->countValues() == 0 ) && m_pDefaultResolution )
        return m_pDefaultResolution->m_aValue;

    if( ! m_pResolutions )
        return aEmptyString;

    int nX, nY;
    for( int i = 0; i < m_pResolutions->countValues(); i++ )
    {
        getResolutionFromString( m_pResolutions->getValue( i )->m_aOption,
                                 nX, nY );
        if( nX == nXRes && nY == nYRes )
            return m_pResolutions->getValue( i )->m_aValue;
    }
    return aEmptyString;
}

const String& PPDParser::getDefaultDuplexType() const
{
    if( m_pDefaultDuplexType )
        return m_pDefaultDuplexType->m_aValue;
    return aEmptyString;
}

const String& PPDParser::getDuplex( int nDuplex ) const
{
    if( ! m_pDuplexTypes )
        return aEmptyString;

    if( nDuplex > 0 && nDuplex < m_pDuplexTypes->countValues() )
        return m_pDuplexTypes->getValue( nDuplex )->m_aOption;
    else if( m_pDuplexTypes->countValues() > 0 )
        return m_pDuplexTypes->getValue( (ULONG)0 )->m_aOption;

    return aEmptyString;
}

const String& PPDParser::getDuplexCommand( int nDuplex ) const
{
    if( ! m_pDuplexTypes )
        return aEmptyString;

    if( nDuplex > 0 && nDuplex < m_pDuplexTypes->countValues() )
        return m_pDuplexTypes->getValue( nDuplex )->m_aValue;
    else if( m_pDuplexTypes->countValues() > 0 )
        return m_pDuplexTypes->getValue( (ULONG)0 )->m_aValue;

    return aEmptyString;
}

const String& PPDParser::getDuplexCommand( const String& rDuplex ) const
{
    if( ! m_pDuplexTypes )
        return aEmptyString;

    for( int i=0; i < m_pDuplexTypes->countValues(); i++ )
    {
        const PPDValue* pValue = m_pDuplexTypes->getValue( i );
        if( pValue->m_aOption == rDuplex )
            return pValue->m_aValue;
    }
    return aEmptyString;
}

void PPDParser::getFontAttributes(
                                  int nFont,
                                  String& rEncoding,
                                  String& rCharset ) const
{
    if( m_pFontList && nFont >= 0 && nFont < m_pFontList->countValues() )
    {
        String aAttribs =
            WhitespaceToSpace( m_pFontList->getValue( nFont )->m_aValue );
        rEncoding   = GetCommandLineToken( 0, aAttribs );
        rCharset    = GetCommandLineToken( 2, aAttribs );
    }
}

void PPDParser::getFontAttributes(
                                  const String& rFont,
                                  String& rEncoding,
                                  String& rCharset ) const
{
    if( m_pFontList )
    {
        for( int i = 0; i < m_pFontList->countValues(); i++ )
            if( m_pFontList->getValue( i )->m_aOption == rFont )
                getFontAttributes( i, rEncoding, rCharset );
    }
}

const String& PPDParser::getFont( int nFont ) const
{
    if( ! m_pFontList )
        return aEmptyString;

    if( nFont >=0 && nFont < m_pFontList->countValues() )
        return m_pFontList->getValue( nFont )->m_aOption;
    return aEmptyString;
}

/*
 *  PPDKey
 */

PPDKey::PPDKey( const String& rKey ) :
        m_aKey( rKey ),
        m_pDefaultValue( NULL ),
        m_bQueryValue( false ),
        m_bUIOption( false ),
        m_eUIType( PickOne ),
        m_nOrderDependency( 100 ),
        m_eSetupType( AnySetup )
{
}

// -------------------------------------------------------------------

PPDKey::~PPDKey()
{
}

// -------------------------------------------------------------------

const PPDValue* PPDKey::getValue( int n ) const
{
    return ((unsigned int)n < m_aOrderedValues.size() && n >= 0) ? m_aOrderedValues[n] : NULL;
}

// -------------------------------------------------------------------

const PPDValue* PPDKey::getValue( const String& rOption ) const
{
    PPDKey::hash_type::const_iterator it = m_aValues.find( rOption );
    return it != m_aValues.end() ? &it->second : NULL;
}

// -------------------------------------------------------------------

void PPDKey::eraseValue( const String& rOption )
{
    PPDKey::hash_type::iterator it = m_aValues.find( rOption );
    if( it == m_aValues.end() )
        return;

    for( PPDKey::value_type::iterator vit = m_aOrderedValues.begin(); vit != m_aOrderedValues.end(); ++vit )
    {
        if( *vit == &(it->second ) )
        {
            m_aOrderedValues.erase( vit );
            break;
        }
    }
    m_aValues.erase( it );
}

// -------------------------------------------------------------------

PPDValue* PPDKey::insertValue( const String& rOption )
{
    if( m_aValues.find( rOption ) != m_aValues.end() )
        return NULL;

    PPDValue aValue;
    aValue.m_aOption = rOption;
    m_aValues[ rOption ] = aValue;
    PPDValue* pValue = &m_aValues[rOption];
    m_aOrderedValues.push_back( pValue );
    return pValue;
}

// -------------------------------------------------------------------

/*
 * PPDContext
 */

PPDContext::PPDContext( const PPDParser* pParser ) :
        m_pParser( pParser )
{
}

// -------------------------------------------------------------------

PPDContext& PPDContext::operator=( const PPDContext& rCopy )
{
    m_pParser           = rCopy.m_pParser;
    m_aCurrentValues    = rCopy.m_aCurrentValues;
    return *this;
}

// -------------------------------------------------------------------

PPDContext::~PPDContext()
{
}

// -------------------------------------------------------------------

const PPDKey* PPDContext::getModifiedKey( int n ) const
{
    ::std::hash_map< const PPDKey*, const PPDValue* >::const_iterator it;
    for( it = m_aCurrentValues.begin(); it != m_aCurrentValues.end() && n--; ++it )
        ;
    return it != m_aCurrentValues.end() ? it->first : NULL;
}

// -------------------------------------------------------------------

void PPDContext::setParser( const PPDParser* pParser )
{
    if( pParser != m_pParser )
    {
        m_aCurrentValues.clear();
        m_pParser = pParser;
    }
}

// -------------------------------------------------------------------

const PPDValue* PPDContext::getValue( const PPDKey* pKey ) const
{
    if( ! m_pParser )
        return NULL;

    ::std::hash_map< const PPDKey*, const PPDValue* >::const_iterator it;
    it = m_aCurrentValues.find( pKey );
    if( it != m_aCurrentValues.end() )
        return it->second;

    if( ! m_pParser->hasKey( pKey ) )
        return NULL;

    const PPDValue* pValue = pKey->getDefaultValue();
    if( ! pValue )
        pValue = pKey->getValue( 0 );

    return pValue;
}

// -------------------------------------------------------------------

const PPDValue* PPDContext::setValue( const PPDKey* pKey, const PPDValue* pValue, bool bDontCareForConstraints )
{
    if( ! m_pParser || ! pKey )
        return NULL;

    // pValue can be NULL - it means ignore this option

    if( ! m_pParser->hasKey( pKey ) )
        return NULL;

    // check constraints
    if( pValue )
    {
        if( bDontCareForConstraints )
        {
            m_aCurrentValues[ pKey ] = pValue;
        }
        else if( checkConstraints( pKey, pValue, true ) )
        {
            m_aCurrentValues[ pKey ] = pValue;

            // after setting this value, check all constraints !
            ::std::hash_map< const PPDKey*, const PPDValue* >::iterator it = m_aCurrentValues.begin();
            while(  it != m_aCurrentValues.end() )
            {
                if( it->first != pKey &&
                    ! checkConstraints( it->first, it->second, false ) )
                {
#ifdef __DEBUG
                    fprintf( stderr, "PPDContext::setValue: option %s (%s) is constrained after setting %s to %s\n",
                             it->first->getKey().GetStr(),
                             it->second->m_aOption.GetStr(),
                             pKey->getKey().GetStr(),
                             pValue->m_aOption.GetStr() );
#endif
                    resetValue( it->first, true );
                    it = m_aCurrentValues.begin();
                }
                else
                    ++it;
            }
        }
    }
    else
        m_aCurrentValues[ pKey ] = NULL;

    return pValue;
}

// -------------------------------------------------------------------

bool PPDContext::checkConstraints( const PPDKey* pKey, const PPDValue* pValue )
{
    if( ! m_pParser || ! pKey || ! pValue )
        return false;

    // ensure that this key is already in the list if it exists at all
    if( m_aCurrentValues.find( pKey ) != m_aCurrentValues.end() )
        return checkConstraints( pKey, pValue, false );

    // it is not in the list, insert it temporarily
    bool bRet = false;
    if( m_pParser->hasKey( pKey ) )
    {
        const PPDValue* pDefValue = pKey->getDefaultValue();
        m_aCurrentValues[ pKey ] = pDefValue;
        bRet = checkConstraints( pKey, pValue, false );
        m_aCurrentValues.erase( pKey );
    }

    return bRet;
}

// -------------------------------------------------------------------

bool PPDContext::resetValue( const PPDKey* pKey, bool bDefaultable )
{
    if( ! pKey || ! m_pParser || ! m_pParser->hasKey( pKey ) )
        return false;

#ifdef __DEBUG
    fprintf( stderr, "resetValue( %s, %s ) ", pKey->getKey().GetStr(),
             bDefaultable ? "true" : "false" );
#endif

    const PPDValue* pResetValue = pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "None" ) ) );
    if( ! pResetValue )
        pResetValue = pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "False" ) ) );
    if( ! pResetValue && bDefaultable )
        pResetValue = pKey->getDefaultValue();

    bool bRet = pResetValue ? ( setValue( pKey, pResetValue ) == pResetValue ? true : false ) : false;

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "%s\n", bRet ? "succeeded" : "failed" );
#endif

    return bRet;
}

// -------------------------------------------------------------------

bool PPDContext::checkConstraints( const PPDKey* pKey, const PPDValue* pNewValue, bool bDoReset )
{
    if( ! pNewValue )
        return true;

    // sanity checks
    if( ! m_pParser )
        return false;

    if( pKey->getValue( pNewValue->m_aOption ) != pNewValue )
        return false;

    // None / False and the default can always be set, but be careful !
    // setting them might influence constrained values
    if( pNewValue->m_aOption.EqualsAscii( "None" ) || pNewValue->m_aOption.EqualsAscii( "False" ) ||
        pNewValue == pKey->getDefaultValue() )
        return true;

    const ::std::list< PPDParser::PPDConstraint >& rConstraints( m_pParser->getConstraints() );
    for( ::std::list< PPDParser::PPDConstraint >::const_iterator it = rConstraints.begin(); it != rConstraints.end(); ++it )
    {
        const PPDKey* pLeft     = it->m_pKey1;
        const PPDKey* pRight    = it->m_pKey2;
        if( ! pLeft || ! pRight || ( pKey != pLeft && pKey != pRight ) )
            continue;

        const PPDKey* pOtherKey = pKey == pLeft ? pRight : pLeft;
        const PPDValue* pOtherKeyOption = pKey == pLeft ? it->m_pOption2 : it->m_pOption1;
        const PPDValue* pKeyOption = pKey == pLeft ? it->m_pOption1 : it->m_pOption2;

        // syntax *Key1 option1 *Key2 option2
        if( pKeyOption && pOtherKeyOption )
        {
            if( pNewValue != pKeyOption )
                continue;
            if( pOtherKeyOption == getValue( pOtherKey ) )
            {
                return false;
            }
        }
        // syntax *Key1 option *Key2  or  *Key1 *Key2 option
        else if( pOtherKeyOption || pKeyOption )
        {
            if( pKeyOption )
            {
                if( ! ( pOtherKeyOption = getValue( pOtherKey ) ) )
                    continue; // this should not happen, PPD broken

                if( pKeyOption == pNewValue &&
                    ! pOtherKeyOption->m_aOption.EqualsAscii( "None" ) &&
                    ! pOtherKeyOption->m_aOption.EqualsAscii( "False" ) )
                {
                    // check if the other value can be reset and
                    // do so if possible
                    if( bDoReset && resetValue( pOtherKey ) )
                        continue;

                    return false;
                }
            }
            else if( pOtherKeyOption )
            {
                if( getValue( pOtherKey ) == pOtherKeyOption &&
                    ! pNewValue->m_aOption.EqualsAscii( "None" ) &&
                    ! pNewValue->m_aOption.EqualsAscii( "False" ) )
                    return false;
            }
            else
            {
                // this should not happen, PPD is broken
            }
        }
        // syntax *Key1 *Key2
        else
        {
            const PPDValue* pOtherValue = getValue( pOtherKey );
            if( ! pOtherValue->m_aOption.EqualsAscii( "None" )  &&
                ! pOtherValue->m_aOption.EqualsAscii( "False" )     &&
                ! pNewValue->m_aOption.EqualsAscii( "None" )        &&
                ! pNewValue->m_aOption.EqualsAscii( "False" ) )
                return false;
        }
    }
    return true;
}

// -------------------------------------------------------------------

void PPDContext::getUnconstrainedValues( const PPDKey* pKey, ::std::list< const PPDValue* >& rValues )
{
    rValues.clear();

    if( ! m_pParser || ! pKey || ! m_pParser->hasKey( pKey ) )
        return;

    int nValues = pKey->countValues();
    for( int i = 0; i < nValues; i++ )
    {
        const PPDValue* pValue = pKey->getValue( i );
        if( checkConstraints( pKey, pValue ) )
            rValues.push_back( pValue );
    }
}


// -------------------------------------------------------------------

void* PPDContext::getStreamableBuffer( ULONG& rBytes ) const
{
    rBytes = 0;
    if( ! m_aCurrentValues.size() )
        return NULL;
    ::std::hash_map< const PPDKey*, const PPDValue* >::const_iterator it;
    for( it = m_aCurrentValues.begin(); it != m_aCurrentValues.end(); ++it )
    {
        ByteString aCopy( it->first->getKey(), RTL_TEXTENCODING_MS_1252 );
        rBytes += aCopy.Len();
        rBytes += 1; // for ':'
        if( it->second )
        {
            aCopy = ByteString( it->second->m_aOption, RTL_TEXTENCODING_MS_1252 );
            rBytes += aCopy.Len();
        }
        else
            rBytes += 4;
        rBytes += 1; // for '\0'
    }
    rBytes += 1;
    void* pBuffer = new char[ rBytes ];
    memset( pBuffer, 0, rBytes );
    char* pRun = (char*)pBuffer;
    for( it = m_aCurrentValues.begin(); it != m_aCurrentValues.end(); ++it )
    {
        ByteString aCopy( it->first->getKey(), RTL_TEXTENCODING_MS_1252 );
        int nBytes = aCopy.Len();
        memcpy( pRun, aCopy.GetBuffer(), nBytes );
        pRun += nBytes;
        *pRun++ = ':';
        if( it->second )
            aCopy = ByteString( it->second->m_aOption, RTL_TEXTENCODING_MS_1252 );
        else
            aCopy = "*nil";
        nBytes = aCopy.Len();
        memcpy( pRun, aCopy.GetBuffer(), nBytes );
        pRun += nBytes;

        *pRun++ = 0;
    }
    return pBuffer;
}

// -------------------------------------------------------------------

void PPDContext::rebuildFromStreamBuffer( void* pBuffer, ULONG nBytes )
{
    if( ! m_pParser )
        return;

    m_aCurrentValues.clear();

    char* pRun = (char*)pBuffer;
    while( *pRun && nBytes )
    {
        ByteString aLine( pRun );
        int nPos = aLine.Search( ':' );
        if( nPos != STRING_NOTFOUND )
        {
            const PPDKey* pKey = m_pParser->getKey( String( aLine.Copy( 0, nPos ), RTL_TEXTENCODING_MS_1252 ) );
            if( pKey )
            {
                const PPDValue* pValue = NULL;
                String aOption( aLine.Copy( nPos+1 ), RTL_TEXTENCODING_MS_1252 );
                if( ! aOption.EqualsAscii( "*nil" ) )
                    pValue = pKey->getValue( aOption );
                m_aCurrentValues[ pKey ] = pValue;
#ifdef __DEBUG
                fprintf( stderr, "PPDContext::rebuildFromStreamBuffer: read PPDKeyValue { %s, %s }\n", pKV->m_pKey->getKey().GetStr(), pKV->m_pCurrentValue ? pKV->m_pCurrentValue->m_aOption.GetStr() : "<nil>" );
#endif
            }
        }
        nBytes -= aLine.Len()+1;
        pRun += aLine.Len()+1;
    }
}

// -------------------------------------------------------------------

int PPDContext::getRenderResolution() const
{
    // initialize to reasonable default, if parser is not set
    int nDPI = 300;
    if( m_pParser )
    {
        int nDPIx = 300, nDPIy = 300;
        const PPDKey* pKey = m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Resolution" ) ) );
        if( pKey )
        {
            const PPDValue* pValue = getValue( pKey );
            if( pValue )
                m_pParser->getResolutionFromString( pValue->m_aOption, nDPIx, nDPIy );
            else
                m_pParser->getDefaultResolution( nDPIx, nDPIy );
        }
        else
            m_pParser->getDefaultResolution( nDPIx, nDPIy );

        nDPI = (nDPIx > nDPIy) ? nDPIx : nDPIy;
    }
    return  nDPI;
}

// -------------------------------------------------------------------

void PPDContext::getPageSize( String& rPaper, int& rWidth, int& rHeight ) const
{
    // initialize to reasonable default, if parser is not set
    rPaper  = String( RTL_CONSTASCII_USTRINGPARAM( "A4" ) );
    rWidth  = 595;
    rHeight = 842;
    if( m_pParser )
    {
        const PPDKey* pKey = m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) ) );
        if( pKey )
        {
            const PPDValue* pValue = getValue( pKey );
            if( pValue )
            {
                rPaper = pValue->m_aOption;
                m_pParser->getPaperDimension( rPaper, rWidth, rHeight );
            }
            else
            {
                rPaper = m_pParser->getDefaultPaperDimension();
                m_pParser->getDefaultPaperDimension( rWidth, rHeight );
            }
        }
    }
}
