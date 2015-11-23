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

#include <algorithm>

#include <sfx2/docfilt.hxx>
#include <sot/storage.hxx>
#include <tools/urlobj.hxx>
#include <svl/fstathelper.hxx>
#include <svl/macitem.hxx>
#include <unotools/charclass.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <shellio.hxx>
#include <swblocks.hxx>
#include <ndtxt.hxx>
#include <mdiexp.hxx>
#include <SwXMLTextBlocks.hxx>
#include <docsh.hxx>
#include <swunohelper.hxx>

#include <statstr.hrc>
#include <swerror.h>

/**
 * Calculate hash code (is not guaranteed to be unique)
 */
sal_uInt16 SwImpBlocks::Hash( const OUString& r )
{
    sal_uInt16 n = 0;
    // std::min requires an explicit cast to sal_Int32 on 32bit platforms
    const sal_Int32 nLen = std::min(r.getLength(), static_cast<sal_Int32>(8));
    for (sal_Int32 i=0; i<nLen; ++i)
    {
        n = ( n << 1 ) + r[i];
    }
    return n;
}

SwBlockName::SwBlockName( const OUString& rShort, const OUString& rLong )
    : aShort( rShort ), aLong( rLong ), aPackageName (rShort),
    bIsOnlyTextFlagInit( false ), bIsOnlyText( false )
{
    nHashS = SwImpBlocks::Hash( rShort );
    nHashL = SwImpBlocks::Hash( rLong );
}

SwBlockName::SwBlockName( const OUString& rShort, const OUString& rLong, const OUString& rPackageName)
    : aShort( rShort ), aLong( rLong ), aPackageName (rPackageName),
    bIsOnlyTextFlagInit( false ), bIsOnlyText( false )
{
    nHashS = SwImpBlocks::Hash( rShort );
    nHashL = SwImpBlocks::Hash( rLong );
}

/**
 * Is the provided file a storage or doesn't it exist?
 */
short SwImpBlocks::GetFileType( const OUString& rFile )
{
    if( !FStatHelper::IsDocument( rFile ) )
        return SWBLK_NO_FILE;
    if( SwXMLTextBlocks::IsFileUCBStorage( rFile ) )
        return SWBLK_XML;
    if( SotStorage::IsStorageFile( rFile ) )
        return SWBLK_SW3;
    //otherwise return NONE
    return SWBLK_NONE;
}

SwImpBlocks::SwImpBlocks( const OUString& rFile, bool )
    : aFile( rFile ),
    aDateModified( Date::EMPTY ),
    aTimeModified( tools::Time::EMPTY ),
    pDoc( nullptr ), nCur( USHRT_MAX ),
    bReadOnly( true ), bInPutMuchBlocks( false ),
    bInfoChanged(false)
{
    FStatHelper::GetModifiedDateTimeOfFile( rFile,
                                            &aDateModified, &aTimeModified );
    INetURLObject aObj(rFile);
    aObj.setExtension( OUString() );
    aName = aObj.GetBase();
}

SwImpBlocks::~SwImpBlocks()
{
    aNames.DeleteAndDestroyAll();
}

/**
 * Delete the document's content
 */
void SwImpBlocks::ClearDoc()
{
    pDoc->ClearDoc();
}

/**
 * Creating a PaM, that spans the whole document
 */
SwPaM* SwImpBlocks::MakePaM()
{
    SwPaM* pPam = new SwPaM( pDoc->GetNodes().GetEndOfContent() );
    pPam->Move( fnMoveBackward, fnGoDoc );
    pPam->SetMark();
    pPam->Move( fnMoveForward, fnGoDoc );
    pPam->Exchange();
    return pPam;
}

sal_uInt16 SwImpBlocks::GetCount() const
{
    return aNames.size();
}

/**
 * Case Insensitive
 */
sal_uInt16 SwImpBlocks::GetIndex( const OUString& rShort ) const
{
    const OUString s( GetAppCharClass().uppercase( rShort ) );
    const sal_uInt16 nHash = Hash( s );
    for( size_t i = 0; i < aNames.size(); i++ )
    {
        const SwBlockName* pName = aNames[ i ];
        if( pName->nHashS == nHash
         && pName->aShort == s )
            return i;
    }
    return USHRT_MAX;
}

sal_uInt16 SwImpBlocks::GetLongIndex( const OUString& rLong ) const
{
    sal_uInt16 nHash = Hash( rLong );
    for( size_t i = 0; i < aNames.size(); i++ )
    {
        const SwBlockName* pName = aNames[ i ];
        if( pName->nHashL == nHash
         && pName->aLong == rLong )
            return i;
    }
    return USHRT_MAX;
}

OUString SwImpBlocks::GetShortName( sal_uInt16 n ) const
{
    if( n < aNames.size() )
        return aNames[n]->aShort;
    return OUString();
}

OUString SwImpBlocks::GetLongName( sal_uInt16 n ) const
{
    if( n < aNames.size() )
        return aNames[n]->aLong;
    return OUString();
}

OUString SwImpBlocks::GetPackageName( sal_uInt16 n ) const
{
    if( n < aNames.size() )
        return aNames[n]->aPackageName;
    return OUString();
}

void SwImpBlocks::AddName( const OUString& rShort, const OUString& rLong,
                           bool bOnlyText )
{
    sal_uInt16 nIdx = GetIndex( rShort );
    if( nIdx != USHRT_MAX )
    {
        delete aNames[nIdx];
        aNames.erase( aNames.begin() + nIdx );
    }
    SwBlockName* pNew = new SwBlockName( rShort, rLong );
    pNew->bIsOnlyTextFlagInit = true;
    pNew->bIsOnlyText = bOnlyText;
    aNames.insert( pNew );
}

bool SwImpBlocks::IsFileChanged() const
{
    Date aTempDateModified( aDateModified );
    tools::Time aTempTimeModified( aTimeModified );
    return FStatHelper::GetModifiedDateTimeOfFile( aFile, &aTempDateModified, &aTempTimeModified ) &&
          ( aDateModified != aTempDateModified ||
            aTimeModified != aTempTimeModified );
}

void SwImpBlocks::Touch()
{
    FStatHelper::GetModifiedDateTimeOfFile( aFile, &aDateModified, &aTimeModified );
}

bool SwImpBlocks::IsOnlyTextBlock( const OUString& ) const
{
    return false;
}

sal_uLong SwImpBlocks::GetMacroTable( sal_uInt16, SvxMacroTableDtor&, bool )
{
    return 0;
}

sal_uLong SwImpBlocks::SetMacroTable( sal_uInt16 , const SvxMacroTableDtor& , bool )
{
    return 0;
}

bool SwImpBlocks::PutMuchEntries( bool )
{
    return false;
}

SwTextBlocks::SwTextBlocks( const OUString& rFile )
    : pImp( nullptr ), nErr( 0 )
{
    INetURLObject aObj(rFile);
    const OUString sFileName = aObj.GetMainURL( INetURLObject::NO_DECODE );
    switch( SwImpBlocks::GetFileType( rFile ) )
    {
    case SWBLK_XML:     pImp = new SwXMLTextBlocks( sFileName ); break;
    case SWBLK_NO_FILE: pImp = new SwXMLTextBlocks( sFileName ); break;
    }
    if( !pImp )
        nErr = ERR_SWG_FILE_FORMAT_ERROR;
}

SwTextBlocks::~SwTextBlocks()
{
    delete pImp;
}

OUString SwTextBlocks::GetName()
{
    return pImp ? pImp->aName : OUString();
}

void SwTextBlocks::SetName( const OUString& r )
{
    if( pImp )
        pImp->SetName( r );
}

bool SwTextBlocks::IsOld() const
{
    if (pImp)
    {
        short nType = pImp->GetFileType();
        if (SWBLK_SW3 == nType || SWBLK_SW2 == nType )
            return true;
    }
    return false;
}

sal_uInt16 SwTextBlocks::GetCount() const
{
    return pImp ? pImp->GetCount() : 0;
}

sal_uInt16 SwTextBlocks::GetIndex( const OUString& r ) const
{
    return pImp ? pImp->GetIndex( r ) : USHRT_MAX;
}

sal_uInt16 SwTextBlocks::GetLongIndex( const OUString& r ) const
{
    return pImp ? pImp->GetLongIndex( r ) : USHRT_MAX;
}

OUString SwTextBlocks::GetShortName( sal_uInt16 n ) const
{
    if( pImp )
        return pImp->GetShortName( n );
    return OUString();
}

OUString SwTextBlocks::GetLongName( sal_uInt16 n ) const
{
    if( pImp )
        return pImp->GetLongName( n );
    return OUString();
}

bool SwTextBlocks::Delete( sal_uInt16 n )
{
    if( pImp && !pImp->bInPutMuchBlocks )
    {
        if( pImp->IsFileChanged() )
            nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
        else if( 0 == (nErr = pImp->OpenFile( false ) ))
        {
            nErr = pImp->Delete( n );
            if( !nErr )
            {
                delete pImp->aNames[n];
                pImp->aNames.erase( pImp->aNames.begin() + n );
            }
            if( n == pImp->nCur )
                pImp->nCur = USHRT_MAX;
            if( !nErr )
                nErr = pImp->MakeBlockList();
        }
        pImp->CloseFile();
        pImp->Touch();

        return ( nErr == 0 );
    }
    return false;
}

sal_uInt16 SwTextBlocks::Rename( sal_uInt16 n, const OUString* s, const OUString* l )
{
    sal_uInt16 nIdx = USHRT_MAX;
    if( pImp && !pImp->bInPutMuchBlocks )
    {
        pImp->nCur = nIdx;
        OUString aNew;
        OUString aLong;
        if( s )
            aNew = aLong = *s;
        if( l )
            aLong = *l;
        if( aNew.isEmpty() )
        {
            OSL_ENSURE( false, "No short name provided in the rename" );
            nErr = ERR_SWG_INTERNAL_ERROR;
            return USHRT_MAX;
        }

        if( pImp->IsFileChanged() )
            nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
        else if( 0 == ( nErr = pImp->OpenFile( false )))
        {
            // Set the new entry in the list before we do that!
            aNew = GetAppCharClass().uppercase( aNew );
             nErr = pImp->Rename( n, aNew, aLong );
            if( !nErr )
            {
                bool bOnlyText = pImp->aNames[ n ]->bIsOnlyText;
                delete pImp->aNames[n];
                pImp->aNames.erase( pImp->aNames.begin() + n );
                pImp->AddName( aNew, aLong, bOnlyText );
                nErr = pImp->MakeBlockList();
            }
        }
        pImp->CloseFile();
        pImp->Touch();
        if( !nErr )
            nIdx = pImp->GetIndex( aNew );
    }
    return nIdx;
}

sal_uLong SwTextBlocks::CopyBlock( SwTextBlocks& rSource, OUString& rSrcShort,
                                const OUString& rLong )
{
    bool bIsOld = false;
    if (rSource.pImp)
    {
        short nType = rSource.pImp->GetFileType();
        if (SWBLK_SW2 == nType || SWBLK_SW3 == nType )
            bIsOld = true;
    }
    if( bIsOld ) //rSource.IsOld() )
        nErr = ERR_SWG_OLD_GLOSSARY;
    else if( pImp->bInPutMuchBlocks )
        nErr = ERR_SWG_INTERNAL_ERROR;
    else
        nErr = pImp->CopyBlock(*rSource.pImp, rSrcShort, rLong);
    return nErr;
}

bool SwTextBlocks::BeginGetDoc( sal_uInt16 n )
{
    if( pImp && !pImp->bInPutMuchBlocks )
    {
        if( pImp->IsFileChanged() )
            nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
        else if( 0 == ( nErr = pImp->OpenFile()))
        {
            pImp->ClearDoc();
            nErr = pImp->GetDoc( n );
            if( nErr )
                pImp->nCur = USHRT_MAX;
            else
                pImp->nCur = n;
        }
        return ( nErr == 0 );
    }
    return false;
}

void SwTextBlocks::EndGetDoc()
{
    if( pImp && !pImp->bInPutMuchBlocks )
        pImp->CloseFile();
}

bool SwTextBlocks::BeginPutDoc( const OUString& s, const OUString& l )
{
    if( pImp )
    {
        bool bOk = pImp->bInPutMuchBlocks;
        if( !bOk )
        {
            if( pImp->IsFileChanged() )
                nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
            else
                nErr = pImp->OpenFile( false );
            bOk = 0 == nErr;
        }
        if( bOk )
        {
            const OUString aNew = GetAppCharClass().uppercase(s);
            nErr = pImp->BeginPutDoc( aNew, l );
        }
        if( nErr )
            pImp->CloseFile();
    }
    return 0 == nErr;
}

sal_uInt16 SwTextBlocks::PutDoc()
{
    sal_uInt16 nIdx = USHRT_MAX;
    if( pImp )
    {
        nErr = pImp->PutDoc();
        if( !nErr )
        {
            pImp->nCur = GetIndex( pImp->aShort );
            if( pImp->nCur != USHRT_MAX )
                pImp->aNames[ pImp->nCur ]->aLong = pImp->aLong;
            else
            {
                pImp->AddName( pImp->aShort, pImp->aLong );
                pImp->nCur = pImp->GetIndex( pImp->aShort );
            }
            if( !pImp->bInPutMuchBlocks )
                nErr = pImp->MakeBlockList();
        }
        if( !pImp->bInPutMuchBlocks )
        {
            pImp->CloseFile();
            pImp->Touch();
        }
        nIdx = pImp->nCur;
    }
    return nIdx;
}

sal_uInt16 SwTextBlocks::PutText( const OUString& rShort, const OUString& rName,
                                  const OUString& rText )
{
    sal_uInt16 nIdx = USHRT_MAX;
    if( pImp )
    {
        bool bOk = pImp->bInPutMuchBlocks;
        if( !bOk )
        {
            if( pImp->IsFileChanged() )
                nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
            else
                nErr = pImp->OpenFile( false );
            bOk = 0 == nErr;
        }
        if( bOk )
        {
            OUString aNew = GetAppCharClass().uppercase( rShort );
            nErr = pImp->PutText( aNew, rName, rText );
            pImp->nCur = USHRT_MAX;
            if( !nErr )
            {
                nIdx = GetIndex( pImp->aShort );
                if( nIdx != USHRT_MAX )
                    pImp->aNames[ nIdx ]->aLong = rName;
                else
                {
                    pImp->AddName( pImp->aShort, rName, true );
                    nIdx = pImp->GetIndex( pImp->aShort );
                }
                if( !pImp->bInPutMuchBlocks )
                    nErr = pImp->MakeBlockList();
            }
        }
        if( !pImp->bInPutMuchBlocks )
        {
            pImp->CloseFile();
            pImp->Touch();
        }
    }
    return nIdx;
}

SwDoc* SwTextBlocks::GetDoc()
{
    if( pImp )
        return pImp->pDoc;
    return nullptr;
}

void SwTextBlocks::ClearDoc()
{
    if( pImp )
    {
        pImp->ClearDoc();
        pImp->nCur = USHRT_MAX;
    }
}

OUString SwTextBlocks::GetFileName() const
{
    return pImp->GetFileName();
}

bool SwTextBlocks::IsReadOnly() const
{
    return pImp->bReadOnly;
}

bool SwTextBlocks::IsOnlyTextBlock( sal_uInt16 nIdx ) const
{
    bool bRet = false;
    if( pImp && !pImp->bInPutMuchBlocks )
    {
        SwBlockName* pBlkNm = pImp->aNames[ nIdx ];
        if( !pBlkNm->bIsOnlyTextFlagInit &&
            !pImp->IsFileChanged() && !pImp->OpenFile() )
        {
            pBlkNm->bIsOnlyText = pImp->IsOnlyTextBlock( pBlkNm->aShort );
            pBlkNm->bIsOnlyTextFlagInit = true;
            pImp->CloseFile();
        }
        bRet = pBlkNm->bIsOnlyText;
    }
    return bRet;
}

bool SwTextBlocks::IsOnlyTextBlock( const OUString& rShort ) const
{
    sal_uInt16 nIdx = pImp->GetIndex( rShort );
    if( USHRT_MAX != nIdx )
    {
        if( pImp->aNames[ nIdx ]->bIsOnlyTextFlagInit )
            return pImp->aNames[ nIdx ]->bIsOnlyText;
        return IsOnlyTextBlock( nIdx );
    }

    OSL_ENSURE( false, "Invalid name" );
    return false;
}

bool SwTextBlocks::GetMacroTable( sal_uInt16 nIdx, SvxMacroTableDtor& rMacroTable )
{
    bool bRet = true;
    if ( pImp && !pImp->bInPutMuchBlocks )
        bRet = ( 0 == pImp->GetMacroTable( nIdx, rMacroTable ) );
    return bRet;
}

bool SwTextBlocks::SetMacroTable( sal_uInt16 nIdx, const SvxMacroTableDtor& rMacroTable )
{
    bool bRet = true;
    if ( pImp && !pImp->bInPutMuchBlocks )
        bRet = ( 0 == pImp->SetMacroTable( nIdx, rMacroTable ) );
    return bRet;
}

bool SwTextBlocks::StartPutMuchBlockEntries()
{
    bool bRet = false;
    if( !IsOld() && pImp )
        bRet = pImp->PutMuchEntries( true );
    return bRet;
}

void SwTextBlocks::EndPutMuchBlockEntries()
{
    if( pImp )
        pImp->PutMuchEntries( false );
}

OUString SwTextBlocks::GetBaseURL() const
{
    if(pImp)
        return pImp->GetBaseURL();
    return OUString();
}

void SwTextBlocks::SetBaseURL( const OUString& rURL )
{
    if(pImp)
        pImp->SetBaseURL(rURL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
