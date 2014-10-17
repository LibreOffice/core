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

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <rtl/ustring.hxx>
#include <sot/stg.hxx>
#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <comphelper/storagehelper.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <docsh.hxx>
#include <pam.hxx>
#include <swblocks.hxx>
#include <ndtxt.hxx>
#include <shellio.hxx>
#include <poolfmt.hxx>
#include <SwXMLTextBlocks.hxx>
#include <SwXMLBlockImport.hxx>
#include <SwXMLBlockExport.hxx>
#include <swerror.h>

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

SwXMLTextBlocks::SwXMLTextBlocks( const OUString& rFile )
    : SwImpBlocks(rFile)
    , bAutocorrBlock(false)
    , bBlock(false)
    , nFlags(0)
    , nCurBlk(0)
{
    SwDocShell* pDocSh = new SwDocShell ( SFX_CREATE_MODE_INTERNAL );
    if( !pDocSh->DoInitNew( 0 ) )
        return;
    bReadOnly = true;
    pDoc = pDocSh->GetDoc();
    xDocShellRef = pDocSh;
    pDoc->SetOle2Link( Link() );
    pDoc->GetIDocumentUndoRedo().DoUndo(false);
    pDoc->acquire();
    uno::Reference< embed::XStorage > refStg;
    if( !aDateModified.GetDate() || !aTimeModified.GetTime() )
        Touch(); // If it's created anew -> get a new timestamp

    try
    {
        refStg  = comphelper::OStorageHelper::GetStorageFromURL( rFile, embed::ElementModes::READWRITE );
        bReadOnly = false;
    }
    catch(const uno::Exception&)
    {
        //FIXME: couldn't open the file - maybe it's readonly
    }
    if( !refStg.is())
    {
        try
        {
            refStg = comphelper::OStorageHelper::GetStorageFromURL( rFile, embed::ElementModes::READ );
        }
        catch(const uno::Exception&)
        {
            OSL_FAIL("exception while creating AutoText storage");
        }
    }
    InitBlockMode ( refStg );
    ReadInfo();
    ResetBlockMode ();
    bInfoChanged = false;
}

SwXMLTextBlocks::SwXMLTextBlocks( const uno::Reference < embed::XStorage >& rStg, const OUString& rName )
    : SwImpBlocks( rName )
    , bAutocorrBlock(false)
    , bBlock(false)
    , nFlags(0)
    , nCurBlk(0)
{
    SwDocShell* pDocSh = new SwDocShell ( SFX_CREATE_MODE_INTERNAL );
    if( !pDocSh->DoInitNew( 0 ) )
        return;
    bReadOnly = false;
    pDoc = pDocSh->GetDoc();
    xDocShellRef = pDocSh;
    pDoc->SetOle2Link( Link() );
    pDoc->GetIDocumentUndoRedo().DoUndo(false);
    pDoc->acquire();

    InitBlockMode ( rStg );
    ReadInfo();
    bInfoChanged = false;
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
void SwXMLTextBlocks::AddName( const OUString& rShort, const OUString& rLong, bool bOnlyTxt )
{
    sal_uInt16 nIdx = GetIndex( rShort );
    SwBlockName* pNew = NULL;
    if( nIdx != (sal_uInt16) -1 )
    {
        delete aNames[nIdx];
        aNames.erase( aNames.begin() + nIdx );
    }

    aPackageName = GeneratePackageName( rShort );
    pNew = new SwBlockName( rShort, rLong, aPackageName );

    pNew->bIsOnlyTxtFlagInit = true;
    pNew->bIsOnlyTxt = bOnlyTxt;
    aNames.insert( pNew );
    bInfoChanged = true;
}
void SwXMLTextBlocks::AddName( const OUString& rShort, const OUString& rLong,
                               const OUString& rPackageName, bool bOnlyTxt )
{
    sal_uInt16 nIdx = GetIndex( rShort );
    if( nIdx != (sal_uInt16) -1 )
    {
        delete aNames[nIdx];
        aNames.erase( aNames.begin() + nIdx );
    }
    SwBlockName* pNew = new SwBlockName( rShort, rLong, rPackageName );
    pNew->bIsOnlyTxtFlagInit = true;
    pNew->bIsOnlyTxt = bOnlyTxt;
    aNames.insert( pNew );
    bInfoChanged = true;
}

sal_uLong SwXMLTextBlocks::Delete( sal_uInt16 n )
{
    const OUString aPckName (aNames[n]->aPackageName);
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
        catch (const uno::Exception&)
        {
            return ERR_SWG_WRITE_ERROR;
        }
    }
    return 0;
}

sal_uLong SwXMLTextBlocks::Rename( sal_uInt16 nIdx, const OUString& rNewShort, const OUString& )
{
    OSL_ENSURE( xBlkRoot.is(), "No storage set" );
    if(!xBlkRoot.is())
        return 0;
    OUString aOldName (aNames[nIdx]->aPackageName);
    aShort = rNewShort;
    aPackageName = GeneratePackageName( aShort );

    if(aOldName != aPackageName)
    {
        if (IsOnlyTextBlock ( nIdx ) )
        {
            OUString sExt(".xml");
            OUString aOldStreamName( aOldName ); aOldStreamName += sExt;
            OUString aNewStreamName( aPackageName ); aNewStreamName += sExt;

            xRoot = xBlkRoot->openStorageElement( aOldName, embed::ElementModes::READWRITE );
            try
            {
                xRoot->renameElement ( aOldStreamName, aNewStreamName );
            }
            catch(const container::ElementExistException&)
            {
                SAL_WARN("sw", "Couldn't rename " << aOldStreamName << " to " << aNewStreamName);
            }
            uno::Reference < embed::XTransactedObject > xTrans( xRoot, uno::UNO_QUERY );
            if ( xTrans.is() )
                xTrans->commit();
            xRoot = 0;
        }

        try
        {
            xBlkRoot->renameElement ( aOldName, aPackageName );
        }
        catch(const container::ElementExistException&)
        {
            SAL_WARN("sw", "Couldn't rename " << aOldName << " to " << aPackageName);
        }
    }
    uno::Reference < embed::XTransactedObject > xTrans( xBlkRoot, uno::UNO_QUERY );
    if ( xTrans.is() )
        xTrans->commit();
    // No need to commit xBlkRoot here as SwTextBlocks::Rename calls
    // WriteInfo which does the commit
    return 0;
}

sal_uLong SwXMLTextBlocks::CopyBlock( SwImpBlocks& rDestImp, OUString& rShort,
                                                    const OUString& rLong)
{
    sal_uLong nError = 0;
    OpenFile(true);
    rDestImp.OpenFile(false);
    const OUString aGroup( rShort );
    bool bTextOnly = IsOnlyTextBlock ( rShort ) ;//pImp->pBlkRoot->IsStream( aGroup );
    sal_uInt16 nIndex = GetIndex ( rShort );
    OUString sDestShortName( GetPackageName (nIndex) );
    sal_uInt16 nIdx = 0;

    OSL_ENSURE( xBlkRoot.is(), "No storage set" );
    if(!xBlkRoot.is())
        return ERR_SWG_WRITE_ERROR;

    uno::Reference < container::XNameAccess > xAccess( ((SwXMLTextBlocks&)rDestImp).xBlkRoot, uno::UNO_QUERY );
    while ( xAccess->hasByName( sDestShortName ) )
    {
        ++nIdx;
        // If someone is that crazy ...
        if(USHRT_MAX == nIdx)
        {
            CloseFile();
            rDestImp.CloseFile();
            return ERR_SWG_WRITE_ERROR;
        }
        sDestShortName += OUString::number( nIdx );
    }

    try
    {
        uno::Reference < embed::XStorage > rSourceRoot = xBlkRoot->openStorageElement( aGroup, embed::ElementModes::READ );
        uno::Reference < embed::XStorage > rDestRoot = ((SwXMLTextBlocks&)rDestImp).xBlkRoot->openStorageElement( sDestShortName, embed::ElementModes::READWRITE );
        rSourceRoot->copyToStorage( rDestRoot );
    }
    catch (const uno::Exception&)
    {
        nError = ERR_SWG_WRITE_ERROR;
    }

    if(!nError)
    {
        rShort = sDestShortName;
        ((SwXMLTextBlocks&)rDestImp).AddName( rShort, rLong, bTextOnly );
        ((SwXMLTextBlocks&)rDestImp).MakeBlockList();
    }
    CloseFile();
    rDestImp.CloseFile();
    return nError;
}

sal_uLong SwXMLTextBlocks::StartPutBlock( const OUString& rShort, const OUString& rPackageName )
{
    OSL_ENSURE( xBlkRoot.is(), "No storage set" );
    if(!xBlkRoot.is())
        return 0;
    GetIndex ( rShort );
    try
    {
        xRoot = xBlkRoot->openStorageElement( rPackageName, embed::ElementModes::READWRITE );

        uno::Reference< beans::XPropertySet > xRootProps( xRoot, uno::UNO_QUERY_THROW );
        OUString aPropName( "MediaType" );
        OUString aMime( SotExchange::GetFormatMimeType( SOT_FORMATSTR_ID_STARWRITER_8 ) );
        xRootProps->setPropertyValue( aPropName, uno::makeAny( aMime ) );
    }
    catch (const uno::Exception&)
    {
    }
    return 0;
}

sal_uLong SwXMLTextBlocks::BeginPutDoc( const OUString& rShort, const OUString& rLong )
{
    // Store in base class
    aShort = rShort;
    aLong = rLong;
    aPackageName = GeneratePackageName( rShort );
    SetIsTextOnly( rShort, false);
    return StartPutBlock (rShort, aPackageName);
}

sal_uLong SwXMLTextBlocks::PutBlock( SwPaM& , const OUString& )
{
    sal_uLong nRes = 0;
    sal_uInt16 nCommitFlags = nFlags & (SWXML_CONVBLOCK|SWXML_NOROOTCOMMIT);

    nFlags |= nCommitFlags;

    WriterRef xWrt;
    ::GetXMLWriter ( OUString(), GetBaseURL(), xWrt);
    SwWriter aWriter (xRoot, *pDoc );

    xWrt->bBlock = true;
    nRes = aWriter.Write ( xWrt );
    xWrt->bBlock = false;
    // Save OLE objects if there are some
    SwDocShell *pDocSh = pDoc->GetDocShell();

    bool bHasChildren = pDocSh && pDocSh->GetEmbeddedObjectContainer().HasEmbeddedObjects();
    if( !nRes && bHasChildren )
    {
        // we have to write to the temporary storage first, since the used below functions are optimized
        // TODO/LATER: it is only a temporary solution, that should be changed soon, the used methods should be
        // called without optimization
        bool bOK = false;

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
                bool bTmpOK = pDocSh->SaveAsChildren( *pTmpMedium );
                if( bTmpOK )
                    bTmpOK = pDocSh->SaveCompletedChildren( false );

                xTempStorage->copyToStorage( xRoot );
                bOK = bTmpOK;
            }
            catch(const uno::Exception&)
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
    catch (const uno::Exception&)
    {
    }

    //TODO/LATER: error handling
    return 0;
}

sal_uLong SwXMLTextBlocks::PutDoc()
{
    SwPaM* pPaM = MakePaM();
    sal_uLong nErr = PutBlock(*pPaM, aLong);
    delete pPaM;
    return nErr;
}

sal_uLong SwXMLTextBlocks::GetText( sal_uInt16 nIdx, OUString& rText )
{
    return GetBlockText( aNames[ nIdx ]->aShort, rText );
}

sal_uLong SwXMLTextBlocks::GetText( const OUString& rShort, OUString& rText )
{
    return GetBlockText( rShort, rText );
}

sal_uLong SwXMLTextBlocks::MakeBlockList()
{
    WriteInfo();
    return 0;
}

bool SwXMLTextBlocks::PutMuchEntries( bool bOn )
{
    bool bRet = false;
    if( bOn )
    {
        if( bInPutMuchBlocks )
        {
            OSL_ENSURE( false, "Nested calls are not allowed");
        }
        else if( !IsFileChanged() )
        {
            bRet = 0 == OpenFile( false );
            if( bRet )
            {
                nFlags |= SWXML_NOROOTCOMMIT;
                bInPutMuchBlocks = true;
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
                bInPutMuchBlocks = false;
                bRet = true;
            }
            catch (const uno::Exception&)
            {
            }
        }
    }
    return bRet;
}

sal_uLong SwXMLTextBlocks::OpenFile( bool bRdOnly )
{
    if( bAutocorrBlock )
        return 0;
    sal_uLong nRet = 0;
    try
    {
        uno::Reference < embed::XStorage > refStg  = comphelper::OStorageHelper::GetStorageFromURL( aFile,
                bRdOnly ? embed::ElementModes::READ : embed::ElementModes::READWRITE );
        InitBlockMode ( refStg );
    }
    catch (const uno::Exception&)
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

void SwXMLTextBlocks::SetIsTextOnly( const OUString& rShort, bool bNewValue )
{
    sal_uInt16 nIdx = GetIndex ( rShort );
    if (nIdx != (sal_uInt16) -1  && nIdx != USHRT_MAX)
        aNames[nIdx]->bIsOnlyTxt = bNewValue;
}

void SwXMLTextBlocks::SetIsTextOnly( sal_uInt16 nIdx, bool bNewValue )
{
    aNames[nIdx]->bIsOnlyTxt = bNewValue;
}

bool SwXMLTextBlocks::IsOnlyTextBlock( const OUString& rShort ) const
{
    sal_uInt16 nIdx = GetIndex ( rShort );
    bool bRet = false;
    if (nIdx != (sal_uInt16) -1  && nIdx != USHRT_MAX)
    {
        bRet = aNames[nIdx]->bIsOnlyTxt;
    }
    return bRet;
}
bool SwXMLTextBlocks::IsOnlyTextBlock( sal_uInt16 nIdx ) const
{
    return aNames[nIdx]->bIsOnlyTxt;
}

bool SwXMLTextBlocks::IsFileUCBStorage( const OUString & rFileName)
{
    OUString aName( rFileName );
    INetURLObject aObj( aName );
    if ( aObj.GetProtocol() == INET_PROT_NOT_VALID )
    {
        OUString aURL;
        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aURL );
        aObj.SetURL( aURL );
        aName = aObj.GetMainURL( INetURLObject::NO_DECODE );
    }

    SvStream * pStm = ::utl::UcbStreamHelper::CreateStream( aName, STREAM_STD_READ );
    bool bRet = UCBStorage::IsStorageFile( pStm );
    delete pStm;
    return bRet;
}

short SwXMLTextBlocks::GetFileType ( void ) const
{
    return SWBLK_XML;
}

OUString SwXMLTextBlocks::GeneratePackageName ( const OUString& rShort )
{
    OString sByte(OUStringToOString(rShort, RTL_TEXTENCODING_UTF7));
    OUStringBuffer aBuf(OStringToOUString(sByte, RTL_TEXTENCODING_ASCII_US));
    const sal_Int32 nLen = aBuf.getLength();
    for (sal_Int32 nPos=0; nPos<nLen; ++nPos)
    {
        switch (aBuf[nPos])
        {
            case '!':
            case '/':
            case ':':
            case '.':
            case '\\':
                aBuf[nPos] = '_';
                break;
            default:
                break;
        }
    }
    return aBuf.makeStringAndClear();
}

sal_uLong SwXMLTextBlocks::PutText( const OUString& rShort, const OUString& rName,
                                    const OUString& rText )
{
    sal_uLong nRes = 0;
    aShort = rShort;
    aLong = rName;
    aCur = rText;
    SetIsTextOnly( aShort, true );
    aPackageName = GeneratePackageName( rShort );
    ClearDoc();
    nRes = PutBlockText( rShort, rName, rText, aPackageName );
    return nRes;
}

void SwXMLTextBlocks::MakeBlockText( const OUString& rText )
{
    SwTxtNode* pTxtNode = pDoc->GetNodes()[ pDoc->GetNodes().GetEndOfContent().
                                        GetIndex() - 1 ]->GetTxtNode();
    if( pTxtNode->GetTxtColl() == pDoc->GetDfltTxtFmtColl() )
        pTxtNode->ChgFmtColl( pDoc->getIDocumentStylePoolAccess().GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));

    sal_Int32 nPos = 0;
    do
    {
        if ( nPos )
        {
            pTxtNode = (SwTxtNode*)pTxtNode->AppendNode( SwPosition( *pTxtNode ) );
        }
        SwIndex aIdx( pTxtNode );
        pTxtNode->InsertText( rText.getToken( 0, '\015', nPos ), aIdx );
    } while ( -1 != nPos );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
