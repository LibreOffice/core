/*************************************************************************
 *
 *  $RCSfile: SwXMLTextBlocks.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: dvo $ $Date: 2001-03-09 14:46:57 $
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
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SV_FONTTYPE_HXX //autogen
#include <vcl/fonttype.hxx>
#endif
#ifndef _SV_FONT_HXX //autogen
#include <vcl/font.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SOT_STORINFO_HXX //autogen
#include <sot/storinfo.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif

#ifndef _SWBLOCKS_HXX
#include <swblocks.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif

#ifndef _SW_XMLTEXTBLOCKS_HXX
#include <SwXMLTextBlocks.hxx>
#endif

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _RSCSFX_HXX
#include <rsc/rscsfx.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_INPUTSOURCE_HPP_
#include <com/sun/star/xml/sax/InputSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif

#ifndef _SFXDOCFILE_HXX //autogen wg. SfxMedium
#include <sfx2/docfile.hxx>
#endif
#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _ERRHDL_HXX //autogen wg. ASSERT
#include <errhdl.hxx>
#endif
#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#ifndef _DOCSH_HXX //autogen wg. SwDoc
#include <docsh.hxx>
#endif

#ifndef _SW_XMLBLOCKIMPORT_HXX
#include <SwXMLBlockImport.hxx>
#endif

#ifndef _SW_XMLBLOCKEXPORT_HXX
#include <SwXMLBlockExport.hxx>
#endif

#ifndef _SFXMACITEM_HXX
#include <svtools/macitem.hxx>
#endif

#ifndef _UNOEVENT_HXX
#include <unoevent.hxx>
#endif



#define STREAM_STGREAD  ( STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE )
#define STREAM_STGWRITE ( STREAM_READ | STREAM_WRITE | STREAM_SHARE_DENYWRITE )

sal_Char __FAR_DATA XMLN_BLOCKLIST[] = "BlockList.xml";

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::rtl;


/*
These are unnecessary as we now have the package name stored in SwBlockName
void xmllcl_EncryptBlockName( String& rName )
{
    //ByteString sByte( rName, RTL_TEXTENCODING_UTF7 );
    //rName = String (sByte, RTL_TEXTENCODING_ASCII_US );
    rName.Insert( '#', 0 );
    xub_StrLen nPos = 1;
    sal_Unicode pDelims[] = { '!', '/', ':', '.', '\\', 0 };
    while( STRING_NOTFOUND != ( nPos = rName.SearchChar( pDelims, nPos )))
    {
        rName.SetChar( nPos, rName.GetChar( nPos ) & 0x0f );
        ++nPos;
    }
}

void xmllcl_DecryptBlockName( String& rName )
{
    if( '#' == rName.GetChar( 0 ) )
    {
        rName.Erase( 0, 1 );

        sal_Unicode c;
        xub_StrLen nLen = rName.Len();
        while( nLen-- )
        {
            switch( rName.GetChar( nLen ) )
            {
            case 0x01:  c = '!';    break;
            case 0x0A:  c = ':';    break;
            case 0x0C:  c = '\\';   break;
            case 0x0E:  c = '.';    break;
            case 0x0F:  c = '/';    break;
            default:    c = 0;      break;
            }
            if( c )
                rName.SetChar( nLen, c );
        }
    }
}
*/
SwXMLTextBlocks::SwXMLTextBlocks( const String& rFile )
: SwImpBlocks( rFile ), bAutocorrBlock( FALSE ), nFlags ( 0 )
{
    SwDocShell* pDocSh = new SwDocShell ( SFX_CREATE_MODE_INTERNAL );
    if( !pDocSh->DoInitNew( 0 ) )
        return;
    bReadOnly = FALSE;
    pDoc = pDocSh->GetDoc();
    xDocShellRef = pDocSh;
    pDoc->SetOle2Link( Link() );
    pDoc->DoUndo( FALSE );      // always FALSE
    pDoc->AddLink();

    SvStorageRef refStg = new SvStorage ( TRUE, aFile, STREAM_STD_READWRITE, STORAGE_TRANSACTED );
    bReadOnly = 0 != refStg->GetError();

    BOOL bIsStg = IsFileUCBStorage( rFile );

    if( !bIsStg || !aDateModified.GetDate() || !aTimeModified.GetTime() )
        Touch();        // falls neu angelegt -> neuen ZeitStempel besorgen

    refStg.Clear();
    refStg = new SvStorage( TRUE, rFile, STREAM_STD_READWRITE | STREAM_SHARE_DENYNONE );

    const OUString sDocName( RTL_CONSTASCII_USTRINGPARAM( XMLN_BLOCKLIST ) );
    if (bIsStg && refStg->IsContained ( sDocName ) )
    {
        Reference< lang::XMultiServiceFactory > xServiceFactory =
                comphelper::getProcessServiceFactory();
        ASSERT( xServiceFactory.is(),
                "XMLReader::Read: got no service manager" );
        if( !xServiceFactory.is() )
        {
            // Throw an exception ?
        }

        xml::sax::InputSource aParserInput;
        aParserInput.sSystemId = rFile;

        SvStorageStreamRef xDocStream = refStg->OpenStream( sDocName, STREAM_STGREAD );
        xDocStream->Seek( 0L );
        xDocStream->SetBufferSize( 16*1024 );
        aParserInput.aInputStream = new utl::OInputStreamWrapper( *xDocStream );

        // get parser
        Reference< XInterface > xXMLParser = xServiceFactory->createInstance(
            OUString::createFromAscii("com.sun.star.xml.sax.Parser") );
        ASSERT( xXMLParser.is(),
            "XMLReader::Read: com.sun.star.xml.sax.Parser service missing" );
        if( !xXMLParser.is() )
        {
            // Maybe throw an exception?
        }

        // get filter
        Reference< xml::sax::XDocumentHandler > xFilter = new SwXMLBlockListImport( *this );

        // connect parser and filter
        Reference< xml::sax::XParser > xParser( xXMLParser, UNO_QUERY );
        xParser->setDocumentHandler( xFilter );

        // parse
        try
        {
            xParser->parseStream( aParserInput );
        }
        catch( xml::sax::SAXParseException&  )
        {
            // re throw ?
        }
        catch( xml::sax::SAXException&  )
        {
            // re throw ?
        }
        catch( io::IOException& )
        {
            // re throw ?
        }
    }
    bInfoChanged = FALSE;
    refStg.Clear();
}

SwXMLTextBlocks::SwXMLTextBlocks( SvStorage& rStg )
    : SwImpBlocks( rStg.GetName() ),
      bAutocorrBlock( TRUE ), nFlags ( 0 )
{
    // This is unused by Marty right now - dont forget...
    SwDocShell* pDocSh = new SwDocShell ( SFX_CREATE_MODE_INTERNAL );
    if( !pDocSh->DoInitNew( 0 ) )
        return;
    bReadOnly = FALSE;
    pDoc = pDocSh->GetDoc();
    xDocShellRef = pDocSh;

    pDoc->DoUndo( FALSE );
    pDoc->AddLink();


    SvStorageRef refStg = &rStg;
    const OUString sDocName( RTL_CONSTASCII_USTRINGPARAM( XMLN_BLOCKLIST ) );
    if ( refStg->IsContained ( sDocName ) )
    {
        Reference< lang::XMultiServiceFactory > xServiceFactory =
                comphelper::getProcessServiceFactory();
        ASSERT( xServiceFactory.is(),
                "XMLReader::Read: got no service manager" );
        if( !xServiceFactory.is() )
        {
            // Throw an exception ?
        }

        xml::sax::InputSource aParserInput;
        aParserInput.sSystemId = sDocName;

        SvStorageStreamRef xDocStream = refStg->OpenStream( sDocName, STREAM_STGREAD );
        xDocStream->Seek( 0L );
        xDocStream->SetBufferSize( 16*1024 );
        aParserInput.aInputStream = new utl::OInputStreamWrapper( *xDocStream );

        // get parser
        Reference< XInterface > xXMLParser = xServiceFactory->createInstance(
            OUString::createFromAscii("com.sun.star.xml.sax.Parser") );
        ASSERT( xXMLParser.is(),
            "XMLReader::Read: com.sun.star.xml.sax.Parser service missing" );
        if( !xXMLParser.is() )
        {
            // Maybe throw an exception?
        }

        // get filter
        Reference< xml::sax::XDocumentHandler > xFilter = new SwXMLBlockListImport( *this );

        // connect parser and filter
        Reference< xml::sax::XParser > xParser( xXMLParser, UNO_QUERY );
        xParser->setDocumentHandler( xFilter );

        // parse
        try
        {
            xParser->parseStream( aParserInput );
        }
        catch( xml::sax::SAXParseException&  )
        {
            // re throw ?
        }
        catch( xml::sax::SAXException&  )
        {
            // re throw ?
        }
        catch( io::IOException& )
        {
            // re throw ?
        }
    }
    bInfoChanged = FALSE;
}

SwXMLTextBlocks::~SwXMLTextBlocks()
{
    if ( bInfoChanged )
        WriteInfo();
    if( pDoc && !pDoc->RemoveLink() )
        delete pDoc;
}

void SwXMLTextBlocks::AddName( const String& rShort, const String& rLong, BOOL bOnlyTxt )
{
    USHORT nIdx = GetIndex( rShort );
    SwBlockName* pNew = NULL;
    if( nIdx != (USHORT) -1 )
        aNames.DeleteAndDestroy( nIdx );
    if (aShort == rShort)
        pNew = new SwBlockName( rShort, rLong, aPackageName );
    else
        pNew = new SwBlockName( rShort, rLong, 0L );

    pNew->bIsOnlyTxtFlagInit = TRUE;
    pNew->bIsOnlyTxt = bOnlyTxt;
    aNames.C40_PTR_INSERT( SwBlockName, pNew );
}
void SwXMLTextBlocks::AddName( const String& rShort, const String& rLong,
                           const String& rPackageName, BOOL bOnlyTxt )
{
    USHORT nIdx = GetIndex( rShort );
    if( nIdx != (USHORT) -1 )
        aNames.DeleteAndDestroy( nIdx );
    SwBlockName* pNew = new SwBlockName( rShort, rLong, rPackageName );
    pNew->bIsOnlyTxtFlagInit = TRUE;
    pNew->bIsOnlyTxt = bOnlyTxt;
    aNames.C40_PTR_INSERT( SwBlockName, pNew );
}

ULONG SwXMLTextBlocks::Delete( USHORT n )
{
    String aName (aNames[ n ]->aPackageName);

    if ( xBlkRoot->IsContained( aName ) )
    {
        xBlkRoot->Remove ( aName );
        return xBlkRoot->Commit() ? 0 : ERR_SWG_WRITE_ERROR;
    }
    return 0;
}

ULONG SwXMLTextBlocks::Rename( USHORT nIdx, const String& s, const String& l )
{
    aNames [ nIdx ]->aShort = s;
    aNames [ nIdx ]->aLong = l;
    GeneratePackageName( s, aNames [ nIdx ]->aPackageName);
    return 0;
}

ULONG SwXMLTextBlocks::CopyBlock( SwImpBlocks& rDestImp, String& rShort,
                                                    const String& rLong)
{
    ULONG nError = 0;
    OpenFile(TRUE);
    rDestImp.OpenFile(FALSE);
    String aGroup( rShort );
    BOOL bTextOnly = IsOnlyTextBlock ( rShort ) ;//pImp->pBlkRoot->IsStream( aGroup );
    USHORT nIndex = GetIndex ( rShort );
    String sDestShortName( GetPackageName (nIndex) );
    USHORT nIdx = 0;

    while(((SwXMLTextBlocks&)rDestImp).xBlkRoot->IsContained( sDestShortName ))
    {
        ++nIdx;
        //falls wirklich mal einer so verrueckt ist
        if(USHRT_MAX == nIdx)
        {
            CloseFile();
            rDestImp.CloseFile();
            return ERR_SWG_WRITE_ERROR;
        }
        sDestShortName += String::CreateFromInt32( nIdx );
    }

    /*if(!bTextOnly)*/
    {
        SvStorageRef rSourceRoot = xBlkRoot->OpenUCBStorage( aGroup, STREAM_STGREAD );
        DBG_ASSERT(rSourceRoot.Is(), "Block existiert nicht!")
        if(!rSourceRoot.Is())
            nError = ERR_SWG_READ_ERROR;
        else
        {
            //jetzt muss der Storage kopiert werden
            if(!xBlkRoot->CopyTo( aGroup, ((SwXMLTextBlocks&)rDestImp).xBlkRoot, sDestShortName ))
            {
                //nError = rSourceRoot->GetError();
                nError = ERR_SWG_WRITE_ERROR;
            }
        }
    }
    /* I think this should work now that text only blocks are in sub-storages as well
    else
    {
        SvStorageStreamRef rSourceStream = xBlkRoot->OpenStream( aGroup, STREAM_STGREAD );
        SvStorageStreamRef rDestStream = ((SwXMLTextBlocks&)rDestImp).xBlkRoot-> OpenStream( sDestShortName, STREAM_STGWRITE );
        if(!rDestStream.Is())
            nError = ERR_SWG_WRITE_ERROR;
        else
        {
            if(!rSourceStream->CopyTo(&rDestStream))
                nError = ERR_SWG_WRITE_ERROR;
            else
                rDestStream->Commit();
        }
    }
    */
    if(!nError)
    {
        rShort = sDestShortName;
        ((SwXMLTextBlocks&)rDestImp).xBlkRoot->Commit();
        ((SwXMLTextBlocks&)rDestImp).AddName( rShort, rLong, bTextOnly );
        ((SwXMLTextBlocks&)rDestImp).MakeBlockList();
    }
    CloseFile();
    rDestImp.CloseFile();
    return nError;
}


ULONG SwXMLTextBlocks::GetDoc( USHORT nIdx )
{
    String aFolderName ( GetPackageName ( nIdx ) );

    if (!IsOnlyTextBlock ( nIdx ) )
    {
        xRoot = xBlkRoot->OpenUCBStorage( aFolderName, STREAM_STGREAD );
        SwReader aReader(*xRoot, aFolderName, pDoc );
        aReader.Read( *ReadXML );
        xRoot.Clear();
    }
    else
    {
        String aStreamName = aFolderName + String::CreateFromAscii(".xml");

        xRoot = xBlkRoot->OpenUCBStorage( aFolderName, STREAM_STGREAD );
        SvStorageStreamRef xContents = xRoot->OpenStream( aStreamName, STREAM_STGREAD );

        Reference< lang::XMultiServiceFactory > xServiceFactory =
            comphelper::getProcessServiceFactory();
        ASSERT( xServiceFactory.is(), "XMLReader::Read: got no service manager" );
        if( !xServiceFactory.is() )
        {
            // Throw an exception ?
        }

        xml::sax::InputSource aParserInput;
        aParserInput.sSystemId = aNames [ nIdx ] ->aPackageName;

        xContents->Seek( 0L );
        xContents->SetBufferSize( 16*1024 );
        aParserInput.aInputStream = new utl::OInputStreamWrapper( *xContents );

        // get parser
        Reference< XInterface > xXMLParser = xServiceFactory->createInstance(
                OUString::createFromAscii("com.sun.star.xml.sax.Parser") );
        ASSERT( xXMLParser.is(),
                "XMLReader::Read: com.sun.star.xml.sax.Parser service missing" );
        if( !xXMLParser.is() )
        {
            // Maybe throw an exception?
        }

        // get filter
        Reference< xml::sax::XDocumentHandler > xFilter = new SwXMLTextBlockImport( *this );

        // connect parser and filter
        Reference< xml::sax::XParser > xParser( xXMLParser, UNO_QUERY );
        xParser->setDocumentHandler( xFilter );

        // parse
        try
        {
            xParser->parseStream( aParserInput );
        }
        catch( xml::sax::SAXParseException&  )
        {
            // re throw ?
        }
        catch( xml::sax::SAXException&  )
        {
            // re throw ?
        }
        catch( io::IOException& )
        {
            // re throw ?
        }
        bInfoChanged = FALSE;
        MakeBlockText(aCur);
        xRoot.Clear();
        xContents.Clear();
    }
    return 0;
}

ULONG SwXMLTextBlocks::StartPutBlock( const String& rShort, const String& rPackageName )
{
    USHORT nIndex = GetIndex ( rShort );
    if( xBlkRoot->IsContained( rPackageName ) )
    {
        xBlkRoot->Remove( rPackageName );
        xBlkRoot->Commit();
    }
    xRoot = xBlkRoot->OpenUCBStorage( rPackageName, STREAM_STGWRITE );
    return 0;
}
ULONG SwXMLTextBlocks::BeginPutDoc( const String& rShort, const String& rLong )
{
    // In der Basisklasse ablegen!
    aShort = rShort;
    aLong = rLong;
    GeneratePackageName( rShort, aPackageName );
    SetIsTextOnly( rShort, FALSE);
    return StartPutBlock (rShort, aPackageName);
}

ULONG SwXMLTextBlocks::PutBlock( SwPaM& rPam, const String& rLong )
{
    ULONG nRes = 0;
    USHORT nCommitFlags = nFlags & (SWXML_CONVBLOCK|SWXML_NOROOTCOMMIT);

    nFlags |= nCommitFlags;

    WriterRef xWrt;
    //::GetXMLWriter ( String::CreateFromAscii(FILTER_XML), xWrt);
    ::GetXMLWriter ( aEmptyStr, xWrt);
    SwWriter aWriter (*xRoot, *pDoc );

    aWriter.Write ( xWrt );

    xRoot->Commit();
    xRoot.Clear();

    if ( !nCommitFlags )
        xBlkRoot->Commit();
    ULONG nErr = xBlkRoot->GetError();
    if( nErr == SVSTREAM_DISK_FULL )
        nRes = ERR_W4W_WRITE_FULL;
    else if( nErr != SVSTREAM_OK )
        nRes = ERR_SWG_WRITE_ERROR;
    nFlags |= nCommitFlags;
    return nErr;
}

ULONG SwXMLTextBlocks::PutDoc()
{
    SwPaM* pPaM = MakePaM();
    ULONG nErr = PutBlock(*pPaM, aLong);
    delete pPaM;
    return nErr;
}

ULONG SwXMLTextBlocks::GetText( USHORT nIdx, String& rText )
{
    return GetBlockText( aNames[ nIdx ]->aShort, rText );
}

ULONG SwXMLTextBlocks::GetText( const String& rShort, String& rText )
{
    return GetBlockText( rShort, rText );
}


ULONG SwXMLTextBlocks::MakeBlockList()
{
    WriteInfo();
    return 0;
}

ULONG SwXMLTextBlocks::SetConvertMode( BOOL bOn )
{
    if (bOn)
        nFlags |= SWXML_CONVBLOCK;
    else
    {
        nFlags &= SWXML_CONVBLOCK;
        if ( xBlkRoot.Is() )
        {
            xBlkRoot->Commit();
            if ( xBlkRoot->GetError() != SVSTREAM_OK )
                return ERR_SWG_WRITE_ERROR;
        }
    }
    return 0;
}

BOOL SwXMLTextBlocks::PutMuchEntries( BOOL bOn )
{
    BOOL bRet = FALSE;
    if( bOn )
    {
        if( bInPutMuchBlocks )
        {
            ASSERT( !this, "verschachtelte Aufrufe sind nicht erlaubt" );
        }
        else if( !IsFileChanged() )
        {
            bRet = 0 == OpenFile( FALSE );
            if( bRet )
            {
                nFlags |= SWXML_NOROOTCOMMIT;
                bInPutMuchBlocks = TRUE;
            }
        }
    }
    else if( bInPutMuchBlocks )
    {
        nFlags &= ~SWXML_NOROOTCOMMIT;
        if( xBlkRoot.Is() )
        {
            xBlkRoot->Commit();
            if( SVSTREAM_OK == xBlkRoot->GetError() )
            {
                MakeBlockList();
                CloseFile();
                Touch();
                bInPutMuchBlocks = FALSE;
                bRet = TRUE;
            }
        }
    }
    return bRet;
}



ULONG SwXMLTextBlocks::OpenFile( BOOL bReadOnly )
{
    if( bAutocorrBlock )
        return 0;

    xBlkRoot = new SvStorage( TRUE, aFile,
                            bReadOnly ? (STREAM_READ | STREAM_SHARE_DENYNONE)
                                      : STREAM_STD_READWRITE);
    return xBlkRoot->GetError();
}

void SwXMLTextBlocks::CloseFile()
{
    if ( !bAutocorrBlock )
    {
        if (bInfoChanged)
            WriteInfo();
    }
    xBlkRoot.Clear();
}

void SwXMLTextBlocks::SetIsTextOnly( const String& rShort, BOOL bNewValue )
{
    USHORT nIdx = GetIndex ( rShort );
    if (nIdx != (USHORT) -1  && nIdx != USHRT_MAX)
        aNames[nIdx]->bIsOnlyTxt = bNewValue;
}

void SwXMLTextBlocks::SetIsTextOnly( USHORT nIdx, BOOL bNewValue )
{
    aNames[nIdx]->bIsOnlyTxt = bNewValue;
}

BOOL SwXMLTextBlocks::IsOnlyTextBlock( const String& rShort ) const
{
    USHORT nIdx = GetIndex ( rShort );
    BOOL bRet = FALSE;
    if (nIdx != (USHORT) -1  && nIdx != USHRT_MAX)
    {
        bRet = aNames[nIdx]->bIsOnlyTxt;
    }
    return bRet;
}
BOOL SwXMLTextBlocks::IsOnlyTextBlock( USHORT nIdx ) const
{
    return aNames[nIdx]->bIsOnlyTxt;
}


ULONG SwXMLTextBlocks::GetMacroTable( USHORT nIdx, SvxMacroTableDtor& rMacroTbl )
{
    // set current auto text
    aShort = aNames[ nIdx ]->aShort;
    aLong = aNames[ nIdx ]->aLong;
    aPackageName = aNames[ nIdx ]->aPackageName;

    ULONG sRet = 0;

    // open stream in proper sub-storage
    CloseFile();
    if ( 0 == OpenFile ( TRUE ) )
    {
        xRoot = xBlkRoot->OpenUCBStorage( aPackageName, STREAM_STGREAD );

        OUString sStreamName = OUString::createFromAscii("atevent.xml");
        SvStorageStreamRef xDocStream = xRoot->OpenStream(
            sStreamName, STREAM_WRITE | STREAM_SHARE_DENYWRITE );
        DBG_ASSERT(xDocStream.Is(), "Can't create output stream");
        if ( xDocStream.Is() )
        {
            xDocStream->SetBufferSize( 16*1024 );
            Reference<io::XInputStream> xInputStream =
                new utl::OInputStreamWrapper( *xDocStream );

            // prepare ParserInputSrouce
            xml::sax::InputSource aParserInput;
            aParserInput.sSystemId = aName;
            aParserInput.aInputStream = xInputStream;

            // get service factory
            Reference< lang::XMultiServiceFactory > xServiceFactory =
                comphelper::getProcessServiceFactory();
            if ( xServiceFactory.is() )
            {

                // get parser
                OUString sParserService( RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.xml.sax.Parser" ) );
                Reference< xml::sax::XParser > xParser(
                    xServiceFactory->createInstance(sParserService),
                    UNO_QUERY );
                DBG_ASSERT( xParser.is(), "Can't create parser" );
                if( xParser.is() )
                {
                    // create descriptor and reference to it. Either
                    // both or neither must be kept because of the
                    // reference counting!
                    SwMacroTableEventDescriptor* pDescriptor =
                        new SwMacroTableEventDescriptor();
                    Reference<XNameReplace> xReplace = pDescriptor;
                    Sequence<Any> aFilterArguments( 1 );
                    aFilterArguments[0] <<= xReplace;

                    // get filter
                    OUString sFilterComponent( RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.comp.Writer.XMLAutotextEventsImporter"));
                    Reference< xml::sax::XDocumentHandler > xFilter(
                        xServiceFactory->createInstanceWithArguments(
                            sFilterComponent, aFilterArguments),
                        UNO_QUERY );
                    DBG_ASSERT( xFilter.is(),
                                "can't instantiate atevents filter");
                    if ( xFilter.is() )
                    {
                        // connect parser and filter
                        xParser->setDocumentHandler( xFilter );

                        // connect model and filter
                        Reference<document::XImporter> xImporter( xFilter,
                                                                  UNO_QUERY );

                        // we don't need a model
                        // xImporter->setTargetDocument( xModelComponent );

                        // parse the stream
                        try
                        {
                            xParser->parseStream( aParserInput );
                        }
                        catch( xml::sax::SAXParseException& )
                        {
                            // workaround for #83452#: SetSize doesn't work
                            // sRet = ERR_SWG_READ_ERROR;
                        }
                        catch( xml::sax::SAXException& )
                        {
                            sRet = ERR_SWG_READ_ERROR;
                        }
                        catch( io::IOException& )
                        {
                            sRet = ERR_SWG_READ_ERROR;
                        }

                        // and finally, copy macro into table
                        if (0 == sRet)
                            pDescriptor->copyMacrosIntoTable(rMacroTbl);
                    }
                    else
                        sRet = ERR_SWG_READ_ERROR;
                }
                else
                    sRet = ERR_SWG_READ_ERROR;

            }
            else
                sRet = ERR_SWG_READ_ERROR;
        }
        else
            sRet = ERR_SWG_READ_ERROR;
    }
    else
        sRet = ERR_SWG_READ_ERROR;

    // success!
    return sRet;
}


BOOL SwXMLTextBlocks::IsFileUCBStorage( const String & rFileName)
{
    String aName( rFileName );
    INetURLObject aObj( aName );
    if ( aObj.GetProtocol() == INET_PROT_NOT_VALID )
    {
        String aURL;
        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aURL );
        aObj.SetURL( aURL );
        aName = aObj.GetMainURL();
    }

    SvStream * pStm = ::utl::UcbStreamHelper::CreateStream( aName, STREAM_STD_READ );
    BOOL bRet = UCBStorage::IsStorageFile( pStm );
    delete pStm;
    return bRet;
}


ULONG SwXMLTextBlocks::GetBlockText( const String& rShort, String& rText )
{
    ULONG n = 0;
    USHORT nIndex = GetIndex ( rShort );
    String aFolderName( GetPackageName ( nIndex ) );
    String aStreamName = aFolderName + String::CreateFromAscii(".xml");

    SvStorageStreamRef xContents;

    // Kurzform!
    xRoot = xBlkRoot->OpenUCBStorage( aFolderName, STREAM_STGREAD );
    xContents = xBlkRoot->OpenStream( aStreamName, STREAM_STGREAD );

    xContents->Seek( 0L );
    xContents->SetBufferSize( 1024 * 2 );
    Reference< lang::XMultiServiceFactory > xServiceFactory =
        comphelper::getProcessServiceFactory();
    ASSERT( xServiceFactory.is(), "XMLReader::Read: got no service manager" );
    if( !xServiceFactory.is() )
    {
        // Throw an exception ?
    }

    xml::sax::InputSource aParserInput;
    aParserInput.sSystemId = aName;
    aParserInput.aInputStream = new utl::OInputStreamWrapper( *xContents );

    // get parser
    Reference< XInterface > xXMLParser = xServiceFactory->createInstance(
            OUString::createFromAscii("com.sun.star.xml.sax.Parser") );
    ASSERT( xXMLParser.is(),
            "XMLReader::Read: com.sun.star.xml.sax.Parser service missing" );
    if( !xXMLParser.is() )
    {
        // Maybe throw an exception?
    }

    // get filter
    Reference< xml::sax::XDocumentHandler > xFilter = new SwXMLTextBlockImport( *this );

    // connect parser and filter
    Reference< xml::sax::XParser > xParser( xXMLParser, UNO_QUERY );
    xParser->setDocumentHandler( xFilter );

    // parse
    try
    {
        xParser->parseStream( aParserInput );
    }
    catch( xml::sax::SAXParseException&  )
    {
        // re throw ?
    }
    catch( xml::sax::SAXException&  )
    {
        // re throw ?
    }
    catch( io::IOException& )
    {
        // re throw ?
    }
    bInfoChanged = FALSE;

    xRoot.Clear();
    xContents.Clear();
    return n;
}

short SwXMLTextBlocks::GetFileType ( void ) const
{
    return SWBLK_XML;
}

ULONG SwXMLTextBlocks::PutBlockText( const String& rShort, const String& rName,
                                     const String& rText,  const String& rPackageName )
{
    USHORT nIndex = GetIndex ( rShort );
    if (xBlkRoot->IsContained ( rPackageName ) )
    {
        xBlkRoot->Remove ( rPackageName );
        xBlkRoot->Commit ( );
    }
    String aFolderName( rPackageName );
    String aStreamName = aFolderName + String::CreateFromAscii(".xml");

    Reference< lang::XMultiServiceFactory > xServiceFactory =
        comphelper::getProcessServiceFactory();
    ASSERT( xServiceFactory.is(),
            "XMLReader::Read: got no service manager" );
    if( !xServiceFactory.is() )
    {
        // Throw an exception ?
    }

       Reference < XInterface > xWriter (xServiceFactory->createInstance(
           OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer"))));
       DBG_ASSERT(xWriter.is(),"com.sun.star.xml.sax.Writer service missing");

    xRoot = xBlkRoot->OpenUCBStorage( aFolderName, STREAM_STGWRITE );
    SvStorageStreamRef xDocStream = xRoot->OpenStream( aStreamName, STREAM_WRITE | STREAM_TRUNC );

    xDocStream->SetSize ( 0L );
    xDocStream->SetBufferSize( 2*1024 );
    Reference < io::XOutputStream > xOut = new  utl::OOutputStreamWrapper(*xDocStream);
       uno::Reference<io::XActiveDataSource> xSrc(xWriter, uno::UNO_QUERY);
       xSrc->setOutputStream(xOut);

       uno::Reference<xml::sax::XDocumentHandler> xHandler(xWriter,
        uno::UNO_QUERY);

       SwXMLTextBlockExport aExp(*this, OUString::createFromAscii(sXML_unformatted_text), xHandler);
    aExp.exportDoc( rText );

    xDocStream->Commit();
    xDocStream.Clear();
    xRoot->Commit();
    xRoot.Clear();

    if (! (nFlags & SWXML_NOROOTCOMMIT) )
        xBlkRoot->Commit();

    ULONG nErr = xBlkRoot->GetError();
    ULONG nRes = 0;
    if( nErr == SVSTREAM_DISK_FULL )
        nRes = ERR_W4W_WRITE_FULL;
    else if( nErr != SVSTREAM_OK )
        nRes = ERR_SWG_WRITE_ERROR;

    if( !nRes )         // damit ueber GetText & nCur aufs Doc zugegriffen
        MakeBlockText( rText );

    return nRes;
}
void SwXMLTextBlocks::GeneratePackageName ( const String& rShort, String& rPackageName )
{
    rPackageName = rShort;
    xub_StrLen nPos = 1;
    sal_Unicode pDelims[] = { '!', '/', ':', '.', '\\', 0 };
    ByteString sByte ( rPackageName, RTL_TEXTENCODING_UTF7);
    rPackageName = String (sByte, RTL_TEXTENCODING_ASCII_US);
    while( STRING_NOTFOUND != ( nPos = rPackageName.SearchChar( pDelims, nPos )))
    {
        rPackageName.SetChar( nPos, rPackageName.GetChar( nPos ) & 0x0f );
        ++nPos;
    }
#if 0
    /*
     * We can assume that if the package name we generate is already in the storage,
     * then we are over-writing it...*/
    if (xBlkRoot.Is() || 0 == OpenFile ( FALSE ) )
    {
        if ( xBlkRoot->IsContained( rPackageName ) )
        {
            xBlkRoot->Remove ( rPackageName  );
            xBlkRoot->Commit();
        }

        /*
        ULONG nIdx=0;
        while ( xBlkRoot->IsContained( rPackageName ))
        {
            ++nIdx;
            rPackageName += String::CreateFromInt32( nIdx );
        }
        */
    }
#endif

}
ULONG SwXMLTextBlocks::PutText( const String& rShort, const String& rName,
                                const String& rText )
{
    ULONG nRes = 0;
    aShort = rShort;
    aLong = rName;
    aCur = rText;
    SetIsTextOnly( aShort, TRUE );
    GeneratePackageName( rShort, aPackageName );
    ClearDoc();
    nRes = PutBlockText( rShort, rName, rText, aPackageName );
    return nRes;
}

void SwXMLTextBlocks::ReadInfo( void )
{
}
void SwXMLTextBlocks::WriteInfo( void )
{
    if ( xBlkRoot.Is() || 0 == OpenFile ( FALSE ) )
    {
        Reference< lang::XMultiServiceFactory > xServiceFactory =
            comphelper::getProcessServiceFactory();
        DBG_ASSERT( xServiceFactory.is(),
                "XMLReader::Read: got no service manager" );
        if( !xServiceFactory.is() )
        {
            // Throw an exception ?
        }

        Reference < XInterface > xWriter (xServiceFactory->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer"))));
        DBG_ASSERT(xWriter.is(),"com.sun.star.xml.sax.Writer service missing");
        OUString sDocName( RTL_CONSTASCII_USTRINGPARAM( XMLN_BLOCKLIST ) );

        if ( xBlkRoot->IsContained( sDocName) )
        {
            xBlkRoot->Remove ( sDocName );
            xBlkRoot->Commit();
        }

        SvStorageStreamRef xDocStream = xBlkRoot->OpenStream( sDocName, STREAM_WRITE | STREAM_TRUNC );
        xDocStream->SetSize ( 0L );
        xDocStream->SetBufferSize( 16*1024 );
        Reference < io::XOutputStream> xOut = new utl::OOutputStreamWrapper(*xDocStream);
        uno::Reference<io::XActiveDataSource> xSrc(xWriter, uno::UNO_QUERY);
        xSrc->setOutputStream(xOut);

        uno::Reference<xml::sax::XDocumentHandler> xHandler(xWriter, uno::UNO_QUERY);

        SwXMLBlockListExport aExp(*this, OUString::createFromAscii(XMLN_BLOCKLIST), xHandler);
        aExp.exportDoc( sXML_block_list );
        xDocStream->Commit();
        xDocStream.Clear();
        xBlkRoot->Commit();
        bInfoChanged = FALSE;
        return;
    }
}
void SwXMLTextBlocks::SetCurrentText( const String& rText )
{
    if (!aCur.Len())
    {
        aCur = rText;
    }
    else
        aCur += rText;
}

void SwXMLTextBlocks::MakeBlockText( const String& rText )
{
    SwTxtNode* pTxtNode = pDoc->GetNodes()[ pDoc->GetNodes().GetEndOfContent().
                                        GetIndex() - 1 ]->GetTxtNode();
    //JP 18.09.98: Bug 56706 - Standard sollte zumindest gesetzt sein!
    if( pTxtNode->GetTxtColl() == pDoc->GetDfltTxtFmtColl() )
        pTxtNode->ChgFmtColl( pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));

    xub_StrLen nPos = 0;
    do
    {
        if ( nPos )
        {
            pTxtNode = (SwTxtNode*)pTxtNode->AppendNode( SwPosition( *pTxtNode ) );
        }
        SwIndex aIdx( pTxtNode );
        String sTemp(rText.GetToken( 0, '\015', nPos ) );
        pTxtNode->Insert( sTemp, aIdx );
    } while ( STRING_NOTFOUND != nPos );
}

void SwXMLTextBlocks::AddTextNode ( const OUString & rText )
{
    SwTxtNode* pTxtNode = pDoc->GetNodes()[ pDoc->GetNodes().GetEndOfContent().
                                        GetIndex() - 1 ]->GetTxtNode();
    //JP 18.09.98: Bug 56706 - Standard sollte zumindest gesetzt sein!
    if( pTxtNode->GetTxtColl() == pDoc->GetDfltTxtFmtColl() )
        pTxtNode->ChgFmtColl( pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));
    SwIndex aIdx ( pTxtNode );
    pTxtNode->Insert ( rText, aIdx );
}

ULONG SwXMLTextBlocks::SetMacroTable( USHORT nIdx, const SvxMacroTableDtor& rMacroTbl )
{
    // set current autotext
    aShort = aNames[ nIdx ]->aShort;
    aLong = aNames[ nIdx ]->aLong;
    aPackageName = aNames[ nIdx ]->aPackageName;

    // start XML autotext event export
    ULONG nRes = 0;

    Reference< lang::XMultiServiceFactory > xServiceFactory =
        comphelper::getProcessServiceFactory();
    ASSERT( xServiceFactory.is(),
            "XML autotext event write:: got no service manager" );
    if( !xServiceFactory.is() )
        return ERR_SWG_WRITE_ERROR;

    // Get model
    Reference< lang::XComponent > xModelComp(
        pDoc->GetDocShell()->GetModel(), UNO_QUERY );
    ASSERT( xModelComp.is(), "XMLWriter::Write: got no model" );
    if( !xModelComp.is() )
        return ERR_SWG_WRITE_ERROR;

    // open stream in proper sub-storage
    CloseFile();    // close (it may be open in read-only-mode)
    if ( 0 == OpenFile ( FALSE ) )
    {
        xRoot = xBlkRoot->OpenUCBStorage( aPackageName, STREAM_STGWRITE );
        OUString sStreamName( RTL_CONSTASCII_USTRINGPARAM("atevent.xml") );

        // workaround for bug: storages do not get overwritten
        // (This workaround is SLOOOOWWW! Remove this ASAP.)
        if( xRoot->IsContained( sStreamName ) )
        {
            xRoot->Remove( sStreamName );
            xRoot->Commit();
            xBlkRoot->Commit();
        }

        SvStorageStreamRef xDocStream = xRoot->OpenStream(
            sStreamName, STREAM_WRITE | STREAM_SHARE_DENYWRITE );
        DBG_ASSERT(xDocStream.Is(), "Can't create output stream");
        if ( xDocStream.Is() )
        {
            xDocStream->SetSize( 0 );
            xDocStream->SetBufferSize( 16*1024 );
            Reference<io::XOutputStream> xOutputStream =
                new utl::OOutputStreamWrapper( *xDocStream );

            // get XML writer
            Reference< io::XActiveDataSource > xSaxWriter(
                xServiceFactory->createInstance(
                    OUString::createFromAscii("com.sun.star.xml.sax.Writer") ),
                UNO_QUERY );
            ASSERT( xSaxWriter.is(), "can't instantiate XML writer" );
            if( xSaxWriter.is() )
            {

                // connect XML writer to output stream
                xSaxWriter->setOutputStream( xOutputStream );
                Reference<xml::sax::XDocumentHandler> xDocHandler(
                    xSaxWriter, UNO_QUERY);

                // construct events object
                Reference<XNameAccess> xEvents =
                    new SwMacroTableEventDescriptor(rMacroTbl);

                // prepare arguments (prepend doc handler to given arguments)
                Sequence<Any> aParams(2);
                aParams[0] <<= xDocHandler;
                aParams[1] <<= xEvents;

                // get filter component
                Reference< document::XExporter > xExporter(
                    xServiceFactory->createInstanceWithArguments(
                        OUString::createFromAscii(
                         "com.sun.star.comp.Writer.XMLAutotextEventsExporter"),
                        aParams), UNO_QUERY);
                ASSERT( xExporter.is(),
                        "can't instantiate export filter component" );
                if( xExporter.is() )
                {
                    // connect model and filter
                    xExporter->setSourceDocument( xModelComp );

                    // filter!
                    Sequence<beans::PropertyValue> aFilterProps( 0 );
                    Reference < document::XFilter > xFilter( xExporter,
                                                             UNO_QUERY );
                    xFilter->filter( aFilterProps );
                }
                else
                    nRes = ERR_SWG_WRITE_ERROR;
            }
            else
                nRes = ERR_SWG_WRITE_ERROR;

            // finally, commit stream, sub-storage and storage
            xDocStream->Commit();
            xDocStream.Clear();

            xRoot->Commit();
            xRoot.Clear();
            xBlkRoot->Commit();
        }
        else
            nRes = ERR_SWG_WRITE_ERROR;

        // close file (because it's in write-mode)
        xRoot.Clear();
        CloseFile();
    }
    else
        nRes = ERR_SWG_WRITE_ERROR;

    return nRes;
}

