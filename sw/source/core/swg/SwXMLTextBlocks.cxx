/*************************************************************************
 *
 *  $RCSfile: SwXMLTextBlocks.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:05:15 $
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


#pragma hdrstop

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _STG_HXX
#include <sot/stg.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SWBLOCKS_HXX
#include <swblocks.hxx>
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
#ifndef _SW_XMLTEXTBLOCKS_HXX
#include <SwXMLTextBlocks.hxx>
#endif
#ifndef _ERRHDL_HXX //autogen wg. ASSERT
#include <errhdl.hxx>
#endif
#ifndef _SW_XMLBLOCKIMPORT_HXX
#include <SwXMLBlockImport.hxx>
#endif
#ifndef _SW_XMLBLOCKEXPORT_HXX
#include <SwXMLBlockExport.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif


#define STREAM_STGREAD  ( STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE )
#define STREAM_STGWRITE ( STREAM_READ | STREAM_WRITE | STREAM_SHARE_DENYWRITE )



void SwXMLTextBlocks::InitBlockMode ( SvStorageRef & rStorage )
{
    xBlkRoot = rStorage;
    xRoot.Clear();
}

void SwXMLTextBlocks::ResetBlockMode ( )
{
    xBlkRoot.Clear();
    xRoot.Clear();
}

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

    if( !aDateModified.GetDate() || !aTimeModified.GetTime() )
        Touch();        // falls neu angelegt -> neuen ZeitStempel besorgen
    SvStorageRef refStg  = new SvStorage( TRUE, rFile, STREAM_STD_READWRITE | STREAM_SHARE_DENYNONE );
    InitBlockMode ( refStg );
    ReadInfo();
    ResetBlockMode ();
    bInfoChanged = FALSE;
}

SwXMLTextBlocks::SwXMLTextBlocks( SvStorage& rStg )
: SwImpBlocks( rStg.GetName() )
, bAutocorrBlock( TRUE )
, nFlags ( 0 )
{
    SwDocShell* pDocSh = new SwDocShell ( SFX_CREATE_MODE_INTERNAL );
    if( !pDocSh->DoInitNew( 0 ) )
        return;
    bReadOnly = FALSE;
    pDoc = pDocSh->GetDoc();
    xDocShellRef = pDocSh;
    pDoc->SetOle2Link( Link() );
    pDoc->DoUndo( FALSE );
    pDoc->AddLink();

    SvStorageRef refStg = &rStg;
    InitBlockMode ( refStg );
    ReadInfo();
    bInfoChanged = FALSE;
}

SwXMLTextBlocks::~SwXMLTextBlocks()
{
    if ( bInfoChanged )
        WriteInfo();
    ResetBlockMode ();
    if( pDoc && !pDoc->RemoveLink() )
        delete pDoc;
}

void SwXMLTextBlocks::ClearDoc()
{
    SwDocShell * pDocShell = pDoc->GetDocShell();
    pDocShell->InvalidateModel();
    pDocShell->ReactivateModel();
    static_cast < SfxObjectShell * > (pDocShell)->Clear();
    pDoc->ClearDoc();
}
void SwXMLTextBlocks::AddName( const String& rShort, const String& rLong, BOOL bOnlyTxt )
{
    USHORT nIdx = GetIndex( rShort );
    SwBlockName* pNew = NULL;
    if( nIdx != (USHORT) -1 )
        aNames.DeleteAndDestroy( nIdx );

    GeneratePackageName( rShort, aPackageName );
    pNew = new SwBlockName( rShort, rLong, aPackageName );

    pNew->bIsOnlyTxtFlagInit = TRUE;
    pNew->bIsOnlyTxt = bOnlyTxt;
    aNames.C40_PTR_INSERT( SwBlockName, pNew );
    bInfoChanged = TRUE;
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
    bInfoChanged = TRUE;
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

ULONG SwXMLTextBlocks::Rename( USHORT nIdx, const String& rNewShort, const String& rNewLong )
{
    String aOldName (aNames[ nIdx ]->aPackageName);
    aShort = rNewShort;
    GeneratePackageName( aShort, aPackageName );
    if (IsOnlyTextBlock ( nIdx ) )
    {
        String sExt( String::CreateFromAscii( ".xml" ));
        String aOldStreamName( aOldName ); aOldStreamName += sExt;
        String aNewStreamName( aPackageName ); aNewStreamName += sExt;

        xRoot = xBlkRoot->OpenUCBStorage ( aOldName, STREAM_STGWRITE );
        xRoot->Rename ( aOldStreamName, aNewStreamName );
        xRoot->Commit();
        xRoot.Clear();
    }
    xBlkRoot->Rename ( aOldName, aPackageName );
    xBlkRoot->Commit();
    // No need to commit xBlkRoot here as SwTextBlocks::Rename calls
    // WriteInfo which does the commit
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


ULONG SwXMLTextBlocks::StartPutBlock( const String& rShort, const String& rPackageName )
{
    USHORT nIndex = GetIndex ( rShort );
    /*
    if( xBlkRoot->IsContained( rPackageName ) )
    {
        xBlkRoot->Remove( rPackageName );
        xBlkRoot->Commit();
    }
    */
    xRoot = xBlkRoot->OpenUCBStorage( rPackageName, STREAM_STGWRITE );
    String aPropName( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM("MediaType") ) );
    ::rtl::OUString aMime( SotExchange::GetFormatMimeType( SOT_FORMATSTR_ID_STARWRITER_8 ) );
    ::com::sun::star::uno::Any aAny;
    aAny <<= aMime;
    xRoot->SetProperty( aPropName, aAny );
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

    xWrt->bBlock = sal_True;
    nRes = aWriter.Write ( xWrt );
    xWrt->bBlock = sal_False;
    // Save OLE objects if there are some
    SwDocShell *pDocSh = pDoc->GetDocShell();

    sal_Bool bHasChilds = pDocSh && pDocSh->GetObjectList() &&
                          pDocSh->GetObjectList()->Count() > 0;
    if( !nRes && bHasChilds )
    {
        sal_Bool bOK = pDocSh->SaveAsChilds( xRoot );
        if( bOK )
            bOK = pDocSh->SaveCompletedChilds( xRoot );
        if( !bOK )
            nRes = ERR_SWG_WRITE_ERROR;
    }

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

    SvStorageRef refStg = new SvStorage( TRUE, aFile,
                            bReadOnly ? (STREAM_READ | STREAM_SHARE_DENYNONE)
                                      : STREAM_STD_READWRITE);
    InitBlockMode ( refStg );
    return xBlkRoot->GetError();
}

void SwXMLTextBlocks::CloseFile()
{
    if ( !bAutocorrBlock )
    {
        if (bInfoChanged)
            WriteInfo();
        ResetBlockMode();
    }
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

BOOL SwXMLTextBlocks::IsFileUCBStorage( const String & rFileName)
{
    String aName( rFileName );
    INetURLObject aObj( aName );
    if ( aObj.GetProtocol() == INET_PROT_NOT_VALID )
    {
        String aURL;
        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aURL );
        aObj.SetURL( aURL );
        aName = aObj.GetMainURL( INetURLObject::NO_DECODE );
    }

    SvStream * pStm = ::utl::UcbStreamHelper::CreateStream( aName, STREAM_STD_READ );
    BOOL bRet = UCBStorage::IsStorageFile( pStm );
    delete pStm;
    return bRet;
}



short SwXMLTextBlocks::GetFileType ( void ) const
{
    return SWBLK_XML;
}

void SwXMLTextBlocks::GeneratePackageName ( const String& rShort, String& rPackageName )
{
    rPackageName = rShort;
    xub_StrLen nPos = 0;
    sal_Unicode pDelims[] = { '!', '/', ':', '.', '\\', 0 };
    ByteString sByte ( rPackageName, RTL_TEXTENCODING_UTF7);
    rPackageName = String (sByte, RTL_TEXTENCODING_ASCII_US);
    while( STRING_NOTFOUND != ( nPos = rPackageName.SearchChar( pDelims, nPos )))
    {
        rPackageName.SetChar( nPos, '_' );
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

void SwXMLTextBlocks::SetCurrentText( const String& rText )
{
    if (!aCur.Len())
        aCur = rText;
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

void SwXMLTextBlocks::AddTextNode ( const String & rText )
{
    SwTxtNode* pTxtNode = pDoc->GetNodes()[ pDoc->GetNodes().GetEndOfContent().
                                        GetIndex() - 1 ]->GetTxtNode();
    //JP 18.09.98: Bug 56706 - Standard sollte zumindest gesetzt sein!
    if( pTxtNode->GetTxtColl() == pDoc->GetDfltTxtFmtColl() )
        pTxtNode->ChgFmtColl( pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));
    SwIndex aIdx ( pTxtNode );
    pTxtNode->Insert( rText, aIdx );
}


