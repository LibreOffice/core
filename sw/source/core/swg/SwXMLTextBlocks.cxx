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
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <osl/file.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sot/exchange.hxx>
#include <sot/stg.hxx>
#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
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
#include <swerror.h>

using namespace ::com::sun::star;

void SwXMLTextBlocks::InitBlockMode ( const uno::Reference < embed::XStorage >& rStorage )
{
    xBlkRoot = rStorage;
    xRoot = nullptr;
}

void SwXMLTextBlocks::ResetBlockMode ( )
{
    xBlkRoot = nullptr;
    xRoot = nullptr;
}

SwXMLTextBlocks::SwXMLTextBlocks( const OUString& rFile )
    : SwImpBlocks(rFile)
    , nFlags(SwXmlFlags::NONE)
{
    SwDocShell* pDocSh = new SwDocShell ( SfxObjectCreateMode::INTERNAL );
    if( !pDocSh->DoInitNew() )
        return;
    m_bReadOnly = true;
    m_xDoc = pDocSh->GetDoc();
    xDocShellRef = pDocSh;
    m_xDoc->SetOle2Link( Link<bool,void>() );
    m_xDoc->GetIDocumentUndoRedo().DoUndo(false);
    uno::Reference< embed::XStorage > refStg;
    if( !m_aDateModified.GetDate() || !m_aTimeModified.GetTime() )
        Touch(); // If it's created anew -> get a new timestamp

    try
    {
        refStg  = comphelper::OStorageHelper::GetStorageFromURL( rFile, embed::ElementModes::READWRITE );
        m_bReadOnly = false;
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
            TOOLS_WARN_EXCEPTION( "sw", "exception while creating AutoText storage");
        }
    }
    InitBlockMode ( refStg );
    ReadInfo();
    ResetBlockMode ();
    m_bInfoChanged = false;
}

SwXMLTextBlocks::SwXMLTextBlocks( const uno::Reference < embed::XStorage >& rStg, const OUString& rName )
    : SwImpBlocks( rName )
    , nFlags(SwXmlFlags::NONE)
{
    SwDocShell* pDocSh = new SwDocShell ( SfxObjectCreateMode::INTERNAL );
    if( !pDocSh->DoInitNew() )
        return;
    m_bReadOnly = false;
    m_xDoc = pDocSh->GetDoc();
    xDocShellRef = pDocSh;
    m_xDoc->SetOle2Link( Link<bool,void>() );
    m_xDoc->GetIDocumentUndoRedo().DoUndo(false);

    InitBlockMode ( rStg );
    ReadInfo();
    m_bInfoChanged = false;
}

SwXMLTextBlocks::~SwXMLTextBlocks()
{
    if ( m_bInfoChanged )
        WriteInfo();
    ResetBlockMode ();
    if(xDocShellRef.is())
        xDocShellRef->DoClose();
    xDocShellRef = nullptr;
}

void SwXMLTextBlocks::ClearDoc()
{
    SwDocShell * pDocShell = m_xDoc->GetDocShell();
    pDocShell->InvalidateModel();
    pDocShell->ReactivateModel();

    m_xDoc->ClearDoc();
    pDocShell->ClearEmbeddedObjects();
}

void SwXMLTextBlocks::AddName( const OUString& rShort, const OUString& rLong, bool bOnlyText )
{
    aPackageName = GeneratePackageName( rShort );
    AddName(rShort, rLong, aPackageName, bOnlyText);
}

void SwXMLTextBlocks::AddName( const OUString& rShort, const OUString& rLong,
                               const OUString& rPackageName, bool bOnlyText )
{
    sal_uInt16 nIdx = GetIndex( rShort );
    if (nIdx != USHRT_MAX)
    {
        m_aNames.erase( m_aNames.begin() + nIdx );
    }
    std::unique_ptr<SwBlockName> pNew(new SwBlockName( rShort, rLong, rPackageName ));
    pNew->bIsOnlyTextFlagInit = true;
    pNew->bIsOnlyText = bOnlyText;
    m_aNames.insert( std::move(pNew) );
    m_bInfoChanged = true;
}

ErrCode SwXMLTextBlocks::Delete( sal_uInt16 n )
{
    const OUString aPckName (m_aNames[n]->aPackageName);
    if ( xBlkRoot.is() &&
            xBlkRoot->hasByName( aPckName ) && xBlkRoot->isStreamElement( aPckName ) )
    {
        try
        {
            xBlkRoot->removeElement ( aPckName );
            uno::Reference < embed::XTransactedObject > xTrans( xBlkRoot, uno::UNO_QUERY );
            if ( xTrans.is() )
                xTrans->commit();
            return ERRCODE_NONE;
        }
        catch (const uno::Exception&)
        {
            return ERR_SWG_WRITE_ERROR;
        }
    }
    return ERRCODE_NONE;
}

ErrCode SwXMLTextBlocks::Rename( sal_uInt16 nIdx, const OUString& rNewShort )
{
    OSL_ENSURE( xBlkRoot.is(), "No storage set" );
    if(!xBlkRoot.is())
        return ERRCODE_NONE;
    OUString aOldName (m_aNames[nIdx]->aPackageName);
    m_aShort = rNewShort;
    aPackageName = GeneratePackageName( m_aShort );

    if(aOldName != aPackageName)
    {
        if (IsOnlyTextBlock ( nIdx ) )
        {
            OUString sExt(".xml");
            OUString aOldStreamName = aOldName  + sExt;
            OUString aNewStreamName = aPackageName + sExt;

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
            xRoot = nullptr;
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
    return ERRCODE_NONE;
}

ErrCode SwXMLTextBlocks::StartPutBlock( const OUString& rShort, const OUString& rPackageName )
{
    OSL_ENSURE( xBlkRoot.is(), "No storage set" );
    if(!xBlkRoot.is())
        return ERRCODE_NONE;
    GetIndex ( rShort );
    try
    {
        xRoot = xBlkRoot->openStorageElement( rPackageName, embed::ElementModes::READWRITE );

        uno::Reference< beans::XPropertySet > xRootProps( xRoot, uno::UNO_QUERY_THROW );
        OUString aMime( SotExchange::GetFormatMimeType( SotClipboardFormatId::STARWRITER_8 ) );
        xRootProps->setPropertyValue( "MediaType", uno::makeAny( aMime ) );
    }
    catch (const uno::Exception&)
    {
    }
    return ERRCODE_NONE;
}

ErrCode SwXMLTextBlocks::BeginPutDoc( const OUString& rShort, const OUString& rLong )
{
    // Store in base class
    m_aShort = rShort;
    m_aLong = rLong;
    aPackageName = GeneratePackageName( rShort );
    SetIsTextOnly( rShort, false);
    return StartPutBlock (rShort, aPackageName);
}

ErrCode SwXMLTextBlocks::PutBlock()
{
    ErrCode nRes = ERRCODE_NONE; // dead variable, this always returns 0
    SwXmlFlags nCommitFlags = nFlags;

    WriterRef xWrt;
    ::GetXMLWriter ( OUString(), GetBaseURL(), xWrt);
    SwWriter aWriter (xRoot, *m_xDoc );

    xWrt->m_bBlock = true;
    nRes = aWriter.Write ( xWrt );
    xWrt->m_bBlock = false;
    // Save OLE objects if there are some
    SwDocShell *pDocSh = m_xDoc->GetDocShell();

    bool bHasChildren = pDocSh && pDocSh->GetEmbeddedObjectContainer().HasEmbeddedObjects();
    if( !nRes && bHasChildren )
    {
        // we have to write to the temporary storage first, since the used below functions are optimized
        // TODO/LATER: it is only a temporary solution, that should be changed soon, the used methods should be
        // called without optimization
        bool bOK = false;

        if ( xRoot.is() )
        {
            std::unique_ptr<SfxMedium> pTmpMedium;
            try
            {
                uno::Reference< embed::XStorage > xTempStorage =
                    ::comphelper::OStorageHelper::GetTemporaryStorage();

                xRoot->copyToStorage( xTempStorage );

                // TODO/LATER: no progress bar?!
                // TODO/MBA: strange construct
                pTmpMedium.reset(new SfxMedium(xTempStorage, GetBaseURL()));
                bool bTmpOK = pDocSh->SaveAsChildren( *pTmpMedium );
                if( bTmpOK )
                    bTmpOK = pDocSh->SaveCompletedChildren();

                xTempStorage->copyToStorage( xRoot );
                bOK = bTmpOK;
            }
            catch(const uno::Exception&)
            {
            }
        }

        if( !bOK )
            nRes = ERR_SWG_WRITE_ERROR;
    }

    try
    {
        uno::Reference < embed::XTransactedObject > xTrans( xRoot, uno::UNO_QUERY );
        if ( xTrans.is() )
            xTrans->commit();
        xRoot = nullptr;
        if ( nCommitFlags == SwXmlFlags::NONE )
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
    return ERRCODE_NONE;
}

ErrCode SwXMLTextBlocks::PutDoc()
{
    std::unique_ptr<SwPaM> pPaM = MakePaM();
    ErrCode nErr = PutBlock();
    return nErr;
}

ErrCode SwXMLTextBlocks::GetText( const OUString& rShort, OUString& rText )
{
    return GetBlockText( rShort, rText );
}

ErrCode SwXMLTextBlocks::MakeBlockList()
{
    WriteInfo();
    return ERRCODE_NONE;
}

bool SwXMLTextBlocks::PutMuchEntries( bool bOn )
{
    bool bRet = false;
    if( bOn )
    {
        if( m_bInPutMuchBlocks )
        {
            OSL_ENSURE( false, "Nested calls are not allowed");
        }
        else if( !IsFileChanged() )
        {
            bRet = ERRCODE_NONE == OpenFile( false );
            if( bRet )
            {
                nFlags |= SwXmlFlags::NoRootCommit;
                m_bInPutMuchBlocks = true;
            }
        }
    }
    else if( m_bInPutMuchBlocks )
    {
        nFlags &= ~SwXmlFlags::NoRootCommit;
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
                m_bInPutMuchBlocks = false;
                bRet = true;
            }
            catch (const uno::Exception&)
            {
            }
        }
    }
    return bRet;
}

ErrCode SwXMLTextBlocks::OpenFile( bool bRdOnly )
{
    ErrCode nRet = ERRCODE_NONE;
    try
    {
        uno::Reference < embed::XStorage > refStg  = comphelper::OStorageHelper::GetStorageFromURL( m_aFile,
                bRdOnly ? embed::ElementModes::READ : embed::ElementModes::READWRITE );
        InitBlockMode ( refStg );
    }
    catch (const uno::Exception&)
    {
        //TODO/LATER: error handling
        nRet = ErrCode(1);
    }

    return nRet;
}

void SwXMLTextBlocks::CloseFile()
{
    if (m_bInfoChanged)
        WriteInfo();
    ResetBlockMode();
}

void SwXMLTextBlocks::SetIsTextOnly( const OUString& rShort, bool bNewValue )
{
    sal_uInt16 nIdx = GetIndex ( rShort );
    if (nIdx != USHRT_MAX)
        m_aNames[nIdx]->bIsOnlyText = bNewValue;
}

bool SwXMLTextBlocks::IsOnlyTextBlock( const OUString& rShort ) const
{
    sal_uInt16 nIdx = GetIndex ( rShort );
    bool bRet = false;
    if (nIdx != USHRT_MAX)
    {
        bRet = m_aNames[nIdx]->bIsOnlyText;
    }
    return bRet;
}
bool SwXMLTextBlocks::IsOnlyTextBlock( sal_uInt16 nIdx ) const
{
    return m_aNames[nIdx]->bIsOnlyText;
}

bool SwXMLTextBlocks::IsFileUCBStorage( const OUString & rFileName)
{
    OUString aName( rFileName );
    INetURLObject aObj( aName );
    if ( aObj.GetProtocol() == INetProtocol::NotValid )
    {
        OUString aURL;
        osl::FileBase::getFileURLFromSystemPath( aName, aURL );
        aObj.SetURL( aURL );
        aName = aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    }

    std::unique_ptr<SvStream> pStm = ::utl::UcbStreamHelper::CreateStream( aName, StreamMode::STD_READ );
    bool bRet = UCBStorage::IsStorageFile( pStm.get() );
    return bRet;
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

ErrCode SwXMLTextBlocks::PutText( const OUString& rShort, const OUString& rName,
                                    const OUString& rText )
{
    ErrCode nRes = ERRCODE_NONE;
    m_aShort = rShort;
    m_aLong = rName;
    m_aCurrentText = rText;
    SetIsTextOnly( m_aShort, true );
    aPackageName = GeneratePackageName( rShort );
    ClearDoc();
    nRes = PutBlockText( rShort, rText, aPackageName );
    return nRes;
}

void SwXMLTextBlocks::MakeBlockText( const OUString& rText )
{
    SwTextNode* pTextNode = m_xDoc->GetNodes()[ m_xDoc->GetNodes().GetEndOfContent().
                                        GetIndex() - 1 ]->GetTextNode();
    if( pTextNode->GetTextColl() == m_xDoc->GetDfltTextFormatColl() )
        pTextNode->ChgFormatColl( m_xDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD ));

    sal_Int32 nPos = 0;
    do
    {
        if ( nPos )
        {
            pTextNode = static_cast<SwTextNode*>(pTextNode->AppendNode( SwPosition( *pTextNode ) ));
        }
        SwIndex aIdx( pTextNode );
        pTextNode->InsertText( rText.getToken( 0, '\015', nPos ), aIdx );
    } while ( -1 != nPos );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
