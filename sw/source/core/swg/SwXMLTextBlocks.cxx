/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwXMLTextBlocks.cxx,v $
 *
 *  $Revision: 1.38 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:10:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif

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

#include <comphelper/storagehelper.hxx>

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

#define STREAM_STGREAD  ( STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE )
#define STREAM_STGWRITE ( STREAM_READ | STREAM_WRITE | STREAM_SHARE_DENYWRITE )

using namespace ::com::sun::star;


void SwXMLTextBlocks::InitBlockMode ( const uno::Reference < embed::XStorage >& rStorage )
{
    xBlkRoot = rStorage;
    xRoot = 0;
}

void SwXMLTextBlocks::ResetBlockMode ( )
{
    xBlkRoot = 0;
    xRoot = 0;
}

SwXMLTextBlocks::SwXMLTextBlocks( const String& rFile )
: SwImpBlocks( rFile ), bAutocorrBlock( FALSE ), nFlags ( 0 )
{
    SwDocShell* pDocSh = new SwDocShell ( SFX_CREATE_MODE_INTERNAL );
    if( !pDocSh->DoInitNew( 0 ) )
        return;
    bReadOnly = TRUE;
    pDoc = pDocSh->GetDoc();
    xDocShellRef = pDocSh;
    pDoc->SetOle2Link( Link() );
    pDoc->DoUndo( FALSE );      // always FALSE
    pDoc->acquire();
    uno::Reference< embed::XStorage > refStg;
    if( !aDateModified.GetDate() || !aTimeModified.GetTime() )
        Touch();        // falls neu angelegt -> neuen ZeitStempel besorgen
    try
    {
        refStg  = comphelper::OStorageHelper::GetStorageFromURL( rFile, embed::ElementModes::READWRITE );
        bReadOnly = FALSE;
    }
    catch( const uno::Exception& )
    {
        //couldn't open the file - maybe it's readonly
    }
    if( !refStg.is())
    {
        try
        {
            refStg = comphelper::OStorageHelper::GetStorageFromURL( rFile, embed::ElementModes::READ );
        }
        catch( const uno::Exception& )
        {
            DBG_ERROR("exception while creating AutoText storage")
        }
    }
    InitBlockMode ( refStg );
    ReadInfo();
    ResetBlockMode ();
    bInfoChanged = FALSE;
}

SwXMLTextBlocks::SwXMLTextBlocks( const uno::Reference < embed::XStorage >& rStg, const String& rName )
: SwImpBlocks( rName )
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
    pDoc->acquire();

    InitBlockMode ( rStg );
    ReadInfo();
    bInfoChanged = FALSE;
}

SwXMLTextBlocks::~SwXMLTextBlocks()
{
    if ( bInfoChanged )
        WriteInfo();
    ResetBlockMode ();
    if(xDocShellRef.Is())
        xDocShellRef->DoClose();
    xDocShellRef = 0;
    if( pDoc && !pDoc->release() )
        delete pDoc;
}

void SwXMLTextBlocks::ClearDoc()
{
    SwDocShell * pDocShell = pDoc->GetDocShell();
    pDocShell->InvalidateModel();
    pDocShell->ReactivateModel();

    pDoc->ClearDoc();
    pDocShell->ClearEmbeddedObjects();
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
    String aPckName (aNames[ n ]->aPackageName);
    uno::Reference < container::XNameAccess > xAccess( xBlkRoot, uno::UNO_QUERY );
    if ( xAccess.is() &&
            xAccess->hasByName( aPckName ) && xBlkRoot->isStreamElement( aPckName ) )
    {
        try
        {
            xBlkRoot->removeElement ( aPckName );
            uno::Reference < embed::XTransactedObject > xTrans( xBlkRoot, uno::UNO_QUERY );
            if ( xTrans.is() )
                xTrans->commit();
            return 0;
        }
        catch ( uno::Exception)
        {
            return ERR_SWG_WRITE_ERROR;
        }
    }
    return 0;
}

ULONG SwXMLTextBlocks::Rename( USHORT nIdx, const String& rNewShort, const String& )
{
    DBG_ASSERT( xBlkRoot.is(), "No storage set" )
    if(!xBlkRoot.is())
        return 0;
    String aOldName (aNames[ nIdx ]->aPackageName);
    aShort = rNewShort;
    GeneratePackageName( aShort, aPackageName );
    if (IsOnlyTextBlock ( nIdx ) )
    {
        String sExt( String::CreateFromAscii( ".xml" ));
        String aOldStreamName( aOldName ); aOldStreamName += sExt;
        String aNewStreamName( aPackageName ); aNewStreamName += sExt;

        xRoot = xBlkRoot->openStorageElement( aOldName, embed::ElementModes::READWRITE );
        xRoot->renameElement ( aOldStreamName, aNewStreamName );
        uno::Reference < embed::XTransactedObject > xTrans( xRoot, uno::UNO_QUERY );
        if ( xTrans.is() )
            xTrans->commit();
        xRoot = 0;
    }

    if(aOldName != aPackageName)
        xBlkRoot->renameElement ( aOldName, aPackageName );
    uno::Reference < embed::XTransactedObject > xTrans( xBlkRoot, uno::UNO_QUERY );
    if ( xTrans.is() )
        xTrans->commit();
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

    DBG_ASSERT( xBlkRoot.is(), "No storage set" )
    if(!xBlkRoot.is())
        return ERR_SWG_WRITE_ERROR;

    uno::Reference < container::XNameAccess > xAccess( ((SwXMLTextBlocks&)rDestImp).xBlkRoot, uno::UNO_QUERY );
    while ( xAccess->hasByName( sDestShortName ) )
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

    try
    {
        uno::Reference < embed::XStorage > rSourceRoot = xBlkRoot->openStorageElement( aGroup, embed::ElementModes::READ );
        uno::Reference < embed::XStorage > rDestRoot = ((SwXMLTextBlocks&)rDestImp).xBlkRoot->openStorageElement( sDestShortName, embed::ElementModes::READWRITE );
        //if(!rSourceRoot.Is())
        //    nError = ERR_SWG_READ_ERROR;
        //else
        //{
        rSourceRoot->copyToStorage( rDestRoot );
    }
    catch ( uno::Exception& )
    {
        nError = ERR_SWG_WRITE_ERROR;
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
        //((SwXMLTextBlocks&)rDestImp).xBlkRoot->Commit();
        ((SwXMLTextBlocks&)rDestImp).AddName( rShort, rLong, bTextOnly );
        ((SwXMLTextBlocks&)rDestImp).MakeBlockList();
    }
    CloseFile();
    rDestImp.CloseFile();
    return nError;
}


ULONG SwXMLTextBlocks::StartPutBlock( const String& rShort, const String& rPackageName )
{
    DBG_ASSERT( xBlkRoot.is(), "No storage set" )
    if(!xBlkRoot.is())
        return 0;
    GetIndex ( rShort );
    /*
    if( xBlkRoot->IsContained( rPackageName ) )
    {
        xBlkRoot->Remove( rPackageName );
        xBlkRoot->Commit();
    }
    */
    try
    {
        xRoot = xBlkRoot->openStorageElement( rPackageName, embed::ElementModes::READWRITE );

        uno::Reference< beans::XPropertySet > xRootProps( xRoot, uno::UNO_QUERY_THROW );
        ::rtl::OUString aPropName( RTL_CONSTASCII_USTRINGPARAM("MediaType") );
        ::rtl::OUString aMime( SotExchange::GetFormatMimeType( SOT_FORMATSTR_ID_STARWRITER_8 ) );
        xRootProps->setPropertyValue( aPropName, uno::makeAny( aMime ) );
    }
    catch (uno::Exception&)
    {
    }
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

ULONG SwXMLTextBlocks::PutBlock( SwPaM& , const String& )
{
    ULONG nRes = 0;
    USHORT nCommitFlags = nFlags & (SWXML_CONVBLOCK|SWXML_NOROOTCOMMIT);

    nFlags |= nCommitFlags;

    WriterRef xWrt;
    ::GetXMLWriter ( aEmptyStr, GetBaseURL(), xWrt);
    SwWriter aWriter (xRoot, *pDoc );

    xWrt->bBlock = sal_True;
    nRes = aWriter.Write ( xWrt );
    xWrt->bBlock = sal_False;
    // Save OLE objects if there are some
    SwDocShell *pDocSh = pDoc->GetDocShell();

    sal_Bool bHasChildren = pDocSh && pDocSh->GetEmbeddedObjectContainer().HasEmbeddedObjects();
    if( !nRes && bHasChildren )
    {
        // we have to write to the temporary storage first, since the used below functions are optimized
        // TODO/LATER: it is only a temporary solution, that should be changed soon, the used methods should be
        // called without optimization

        sal_Bool bOK = sal_False;

        if ( xRoot.is() )
        {
            SfxMedium* pTmpMedium = NULL;
            try
            {
                uno::Reference< embed::XStorage > xTempStorage =
                    ::comphelper::OStorageHelper::GetTemporaryStorage();

                xRoot->copyToStorage( xTempStorage );

                // TODO/LATER: no progress bar?!
                // TODO/MBA: strange construct
                pTmpMedium = new SfxMedium( xTempStorage, GetBaseURL() );
                sal_Bool bTmpOK = pDocSh->SaveAsChildren( *pTmpMedium );
                if( bTmpOK )
                    bTmpOK = pDocSh->SaveCompletedChildren( sal_False );

                xTempStorage->copyToStorage( xRoot );
                bOK = bTmpOK;
            }
            catch( uno::Exception& )
            {
            }

            if ( pTmpMedium )
                DELETEZ( pTmpMedium );
        }

        if( !bOK )
            nRes = ERR_SWG_WRITE_ERROR;
    }

    try
    {
        uno::Reference < embed::XTransactedObject > xTrans( xRoot, uno::UNO_QUERY );
        if ( xTrans.is() )
            xTrans->commit();
        xRoot = 0;
        if ( !nCommitFlags )
        {
            uno::Reference < embed::XTransactedObject > xTmpTrans( xBlkRoot, uno::UNO_QUERY );
            if ( xTmpTrans.is() )
                xTmpTrans->commit();
        }
    }
    catch (uno::Exception&)
    {
    }

    //TODO/LATER: error handling
    /*
    ULONG nErr = xBlkRoot->GetError();
    if( nErr == SVSTREAM_DISK_FULL )
        nRes = ERR_W4W_WRITE_FULL;
    else if( nErr != SVSTREAM_OK )
        nRes = ERR_SWG_WRITE_ERROR;
    nFlags |= nCommitFlags;
    return nErr;*/
    return 0;
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
        if( xBlkRoot.is() )
        {
            try
            {
                uno::Reference < embed::XTransactedObject > xTrans( xBlkRoot, uno::UNO_QUERY );
                if ( xTrans.is() )
                    xTrans->commit();
                MakeBlockList();
                CloseFile();
                Touch();
                bInPutMuchBlocks = FALSE;
                bRet = TRUE;
            }
            catch (uno::Exception&)
            {
            }
        }
    }
    return bRet;
}

ULONG SwXMLTextBlocks::OpenFile( BOOL bRdOnly )
{
    if( bAutocorrBlock )
        return 0;
    ULONG nRet = 0;
    try
    {
        uno::Reference < embed::XStorage > refStg  = comphelper::OStorageHelper::GetStorageFromURL( aFile,
                bRdOnly ? embed::ElementModes::READ : embed::ElementModes::READWRITE );
        InitBlockMode ( refStg );
    }
    catch ( uno::Exception& )
    {
        //TODO/LATER: error handling
        nRet = 1;
    }

    return nRet;
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
