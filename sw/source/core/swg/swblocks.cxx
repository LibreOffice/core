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

#include <osl/diagnose.h>
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
SwImpBlocks::FileType SwImpBlocks::GetFileType( const OUString& rFile )
{
    if( !FStatHelper::IsDocument( rFile ) )
        return FileType::NoFile;
    if( SwXMLTextBlocks::IsFileUCBStorage( rFile ) )
        return FileType::XML;
    if( SotStorage::IsStorageFile( rFile ) )
        return FileType::SW3;
    //otherwise return NONE
    return FileType::None;
}

SwImpBlocks::SwImpBlocks( const OUString& rFile )
    : m_aFile( rFile ),
    m_aDateModified( Date::EMPTY ),
    m_aTimeModified( tools::Time::EMPTY ),
    m_nCurrentIndex( USHRT_MAX ),
    m_bReadOnly( true ), m_bInPutMuchBlocks( false ),
    m_bInfoChanged(false)
{
    FStatHelper::GetModifiedDateTimeOfFile( rFile,
                                            &m_aDateModified, &m_aTimeModified );
    INetURLObject aObj(rFile);
    aObj.setExtension( OUString() );
    m_aName = aObj.GetBase();
}

SwImpBlocks::~SwImpBlocks()
{
}

/**
 * Delete the document's content
 */
void SwImpBlocks::ClearDoc()
{
    m_xDoc->ClearDoc();
}

/**
 * Creating a PaM, that spans the whole document
 */
std::unique_ptr<SwPaM> SwImpBlocks::MakePaM()
{
    std::unique_ptr<SwPaM> pPam(new SwPaM( m_xDoc->GetNodes().GetEndOfContent() ));
    pPam->Move( fnMoveBackward, GoInDoc );
    pPam->SetMark();
    pPam->Move( fnMoveForward, GoInDoc );
    pPam->Exchange();
    return pPam;
}

sal_uInt16 SwImpBlocks::GetCount() const
{
    return m_aNames.size();
}

/**
 * Case Insensitive
 */
sal_uInt16 SwImpBlocks::GetIndex( const OUString& rShort ) const
{
    const OUString s( GetAppCharClass().uppercase( rShort ) );
    const sal_uInt16 nHash = Hash( s );
    for( size_t i = 0; i < m_aNames.size(); i++ )
    {
        const SwBlockName* pName = m_aNames[ i ].get();
        if( pName->nHashS == nHash
         && pName->aShort == s )
            return i;
    }
    return USHRT_MAX;
}

sal_uInt16 SwImpBlocks::GetLongIndex( const OUString& rLong ) const
{
    sal_uInt16 nHash = Hash( rLong );
    for( size_t i = 0; i < m_aNames.size(); i++ )
    {
        const SwBlockName* pName = m_aNames[ i ].get();
        if( pName->nHashL == nHash
         && pName->aLong == rLong )
            return i;
    }
    return USHRT_MAX;
}

OUString SwImpBlocks::GetShortName( sal_uInt16 n ) const
{
    if( n < m_aNames.size() )
        return m_aNames[n]->aShort;
    return OUString();
}

OUString SwImpBlocks::GetLongName( sal_uInt16 n ) const
{
    if( n < m_aNames.size() )
        return m_aNames[n]->aLong;
    return OUString();
}

OUString SwImpBlocks::GetPackageName( sal_uInt16 n ) const
{
    if( n < m_aNames.size() )
        return m_aNames[n]->aPackageName;
    return OUString();
}

void SwImpBlocks::AddName( const OUString& rShort, const OUString& rLong,
                           bool bOnlyText )
{
    sal_uInt16 nIdx = GetIndex( rShort );
    if( nIdx != USHRT_MAX )
    {
        m_aNames.erase( m_aNames.begin() + nIdx );
    }
    std::unique_ptr<SwBlockName> pNew(new SwBlockName( rShort, rLong ));
    pNew->bIsOnlyTextFlagInit = true;
    pNew->bIsOnlyText = bOnlyText;
    m_aNames.insert( std::move(pNew) );
}

bool SwImpBlocks::IsFileChanged() const
{
    Date aTempDateModified( m_aDateModified );
    tools::Time aTempTimeModified( m_aTimeModified );
    return FStatHelper::GetModifiedDateTimeOfFile( m_aFile, &aTempDateModified, &aTempTimeModified ) &&
          ( m_aDateModified != aTempDateModified ||
            m_aTimeModified != aTempTimeModified );
}

void SwImpBlocks::Touch()
{
    FStatHelper::GetModifiedDateTimeOfFile( m_aFile, &m_aDateModified, &m_aTimeModified );
}

bool SwImpBlocks::IsOnlyTextBlock( const OUString& ) const
{
    return false;
}

ErrCode SwImpBlocks::GetMacroTable( sal_uInt16, SvxMacroTableDtor& )
{
    return ERRCODE_NONE;
}

ErrCode SwImpBlocks::SetMacroTable( sal_uInt16 , const SvxMacroTableDtor& )
{
    return ERRCODE_NONE;
}

bool SwImpBlocks::PutMuchEntries( bool )
{
    return false;
}

SwTextBlocks::SwTextBlocks( const OUString& rFile )
    : nErr( 0 )
{
    INetURLObject aObj(rFile);
    const OUString sFileName = aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    switch( SwImpBlocks::GetFileType( rFile ) )
    {
    case SwImpBlocks::FileType::XML:    pImp.reset( new SwXMLTextBlocks( sFileName ) ); break;
    case SwImpBlocks::FileType::NoFile: pImp.reset( new SwXMLTextBlocks( sFileName ) ); break;
    default: break;
    }
    if( !pImp )
        nErr = ERR_SWG_FILE_FORMAT_ERROR;
}

SwTextBlocks::~SwTextBlocks()
{
}

OUString SwTextBlocks::GetName()
{
    return pImp ? pImp->m_aName : OUString();
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
        SwImpBlocks::FileType nType = pImp->GetFileType();
        if (SwImpBlocks::FileType::SW3 == nType)
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
    if( pImp && !pImp->m_bInPutMuchBlocks )
    {
        if( pImp->IsFileChanged() )
            nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
        else if( ERRCODE_NONE == (nErr = pImp->OpenFile( false ) ))
        {
            nErr = pImp->Delete( n );
            if( !nErr )
            {
                pImp->m_aNames.erase( pImp->m_aNames.begin() + n );
            }
            if( n == pImp->m_nCurrentIndex )
                pImp->m_nCurrentIndex = USHRT_MAX;
            if( !nErr )
                nErr = pImp->MakeBlockList();
        }
        pImp->CloseFile();
        pImp->Touch();

        return ( nErr == ERRCODE_NONE );
    }
    return false;
}

void SwTextBlocks::Rename( sal_uInt16 n, const OUString* s, const OUString* l )
{
    if( pImp && !pImp->m_bInPutMuchBlocks )
    {
        pImp->m_nCurrentIndex = USHRT_MAX;
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
            return;
        }

        if( pImp->IsFileChanged() )
            nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
        else if( ERRCODE_NONE == ( nErr = pImp->OpenFile( false )))
        {
            // Set the new entry in the list before we do that!
            aNew = GetAppCharClass().uppercase( aNew );
            nErr = pImp->Rename( n, aNew, aLong );
            if( !nErr )
            {
                bool bOnlyText = pImp->m_aNames[ n ]->bIsOnlyText;
                pImp->m_aNames.erase( pImp->m_aNames.begin() + n );
                pImp->AddName( aNew, aLong, bOnlyText );
                nErr = pImp->MakeBlockList();
            }
        }
        pImp->CloseFile();
        pImp->Touch();
    }
}

ErrCode const & SwTextBlocks::CopyBlock( SwTextBlocks const & rSource, OUString& rSrcShort,
                                const OUString& rLong )
{
    bool bIsOld = false;
    if (rSource.pImp)
    {
        SwImpBlocks::FileType nType = rSource.pImp->GetFileType();
        if (SwImpBlocks::FileType::SW3 == nType)
            bIsOld = true;
    }
    if( bIsOld ) //rSource.IsOld() )
        nErr = ERR_SWG_OLD_GLOSSARY;
    else if( pImp->m_bInPutMuchBlocks )
        nErr = ERR_SWG_INTERNAL_ERROR;
    else
        nErr = pImp->CopyBlock(*rSource.pImp, rSrcShort, rLong);
    return nErr;
}

bool SwTextBlocks::BeginGetDoc( sal_uInt16 n )
{
    if( pImp && !pImp->m_bInPutMuchBlocks )
    {
        if( pImp->IsFileChanged() )
            nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
        else if( ERRCODE_NONE == ( nErr = pImp->OpenFile()))
        {
            pImp->ClearDoc();
            nErr = pImp->GetDoc( n );
            if( nErr )
                pImp->m_nCurrentIndex = USHRT_MAX;
            else
                pImp->m_nCurrentIndex = n;
        }
        return ( nErr == ERRCODE_NONE );
    }
    return false;
}

void SwTextBlocks::EndGetDoc()
{
    if( pImp && !pImp->m_bInPutMuchBlocks )
        pImp->CloseFile();
}

bool SwTextBlocks::BeginPutDoc( const OUString& s, const OUString& l )
{
    if( pImp )
    {
        bool bOk = pImp->m_bInPutMuchBlocks;
        if( !bOk )
        {
            if( pImp->IsFileChanged() )
                nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
            else
                nErr = pImp->OpenFile( false );
            bOk = ERRCODE_NONE == nErr;
        }
        if( bOk )
        {
            const OUString aNew = GetAppCharClass().uppercase(s);
            nErr = pImp->BeginPutDoc( aNew, l );
        }
        if( nErr )
            pImp->CloseFile();
    }
    return ERRCODE_NONE == nErr;
}

sal_uInt16 SwTextBlocks::PutDoc()
{
    sal_uInt16 nIdx = USHRT_MAX;
    if( pImp )
    {
        nErr = pImp->PutDoc();
        if( !nErr )
        {
            pImp->m_nCurrentIndex = GetIndex( pImp->m_aShort );
            if( pImp->m_nCurrentIndex != USHRT_MAX )
                pImp->m_aNames[ pImp->m_nCurrentIndex ]->aLong = pImp->m_aLong;
            else
            {
                pImp->AddName( pImp->m_aShort, pImp->m_aLong );
                pImp->m_nCurrentIndex = pImp->GetIndex( pImp->m_aShort );
            }
            if( !pImp->m_bInPutMuchBlocks )
                nErr = pImp->MakeBlockList();
        }
        if( !pImp->m_bInPutMuchBlocks )
        {
            pImp->CloseFile();
            pImp->Touch();
        }
        nIdx = pImp->m_nCurrentIndex;
    }
    return nIdx;
}

sal_uInt16 SwTextBlocks::PutText( const OUString& rShort, const OUString& rName,
                                  const OUString& rText )
{
    sal_uInt16 nIdx = USHRT_MAX;
    if( pImp )
    {
        bool bOk = pImp->m_bInPutMuchBlocks;
        if( !bOk )
        {
            if( pImp->IsFileChanged() )
                nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
            else
                nErr = pImp->OpenFile( false );
            bOk = ERRCODE_NONE == nErr;
        }
        if( bOk )
        {
            OUString aNew = GetAppCharClass().uppercase( rShort );
            nErr = pImp->PutText( aNew, rName, rText );
            pImp->m_nCurrentIndex = USHRT_MAX;
            if( !nErr )
            {
                nIdx = GetIndex( pImp->m_aShort );
                if( nIdx != USHRT_MAX )
                    pImp->m_aNames[ nIdx ]->aLong = rName;
                else
                {
                    pImp->AddName( pImp->m_aShort, rName, true );
                    nIdx = pImp->GetIndex( pImp->m_aShort );
                }
                if( !pImp->m_bInPutMuchBlocks )
                    nErr = pImp->MakeBlockList();
            }
        }
        if( !pImp->m_bInPutMuchBlocks )
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
        return pImp->m_xDoc.get();
    return nullptr;
}

void SwTextBlocks::ClearDoc()
{
    if( pImp )
    {
        pImp->ClearDoc();
        pImp->m_nCurrentIndex = USHRT_MAX;
    }
}

OUString const & SwTextBlocks::GetFileName() const
{
    return pImp->GetFileName();
}

bool SwTextBlocks::IsReadOnly() const
{
    return pImp->m_bReadOnly;
}

bool SwTextBlocks::IsOnlyTextBlock( sal_uInt16 nIdx ) const
{
    bool bRet = false;
    if( pImp && !pImp->m_bInPutMuchBlocks )
    {
        SwBlockName* pBlkNm = pImp->m_aNames[ nIdx ].get();
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
        if( pImp->m_aNames[ nIdx ]->bIsOnlyTextFlagInit )
            return pImp->m_aNames[ nIdx ]->bIsOnlyText;
        return IsOnlyTextBlock( nIdx );
    }

    OSL_ENSURE( false, "Invalid name" );
    return false;
}

bool SwTextBlocks::GetMacroTable( sal_uInt16 nIdx, SvxMacroTableDtor& rMacroTable )
{
    bool bRet = true;
    if ( pImp && !pImp->m_bInPutMuchBlocks )
        bRet = ( ERRCODE_NONE == pImp->GetMacroTable( nIdx, rMacroTable ) );
    return bRet;
}

bool SwTextBlocks::SetMacroTable( sal_uInt16 nIdx, const SvxMacroTableDtor& rMacroTable )
{
    bool bRet = true;
    if ( pImp && !pImp->m_bInPutMuchBlocks )
        bRet = ( ERRCODE_NONE == pImp->SetMacroTable( nIdx, rMacroTable ) );
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
