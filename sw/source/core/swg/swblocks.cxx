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
#include <tools/urlobj.hxx>
#include <svl/fstathelper.hxx>
#include <svl/macitem.hxx>
#include <unotools/charclass.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <shellio.hxx>
#include <swblocks.hxx>
#include <SwXMLTextBlocks.hxx>

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
    : m_aShort( rShort ), m_aLong( rLong ), m_aPackageName (rShort),
    m_bIsOnlyTextFlagInit( false ), m_bIsOnlyText( false )
{
    m_nHashS = SwImpBlocks::Hash( rShort );
    m_nHashL = SwImpBlocks::Hash( rLong );
}

SwBlockName::SwBlockName( const OUString& rShort, const OUString& rLong, const OUString& rPackageName)
    : m_aShort( rShort ), m_aLong( rLong ), m_aPackageName (rPackageName),
    m_bIsOnlyTextFlagInit( false ), m_bIsOnlyText( false )
{
    m_nHashS = SwImpBlocks::Hash( rShort );
    m_nHashL = SwImpBlocks::Hash( rLong );
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
    aObj.setExtension( u"" );
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
        if( pName->m_nHashS == nHash
         && pName->m_aShort == s )
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
        if( pName->m_nHashL == nHash
         && pName->m_aLong == rLong )
            return i;
    }
    return USHRT_MAX;
}

OUString SwImpBlocks::GetShortName( sal_uInt16 n ) const
{
    if( n < m_aNames.size() )
        return m_aNames[n]->m_aShort;
    return OUString();
}

OUString SwImpBlocks::GetLongName( sal_uInt16 n ) const
{
    if( n < m_aNames.size() )
        return m_aNames[n]->m_aLong;
    return OUString();
}

OUString SwImpBlocks::GetPackageName( sal_uInt16 n ) const
{
    if( n < m_aNames.size() )
        return m_aNames[n]->m_aPackageName;
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
    pNew->m_bIsOnlyTextFlagInit = true;
    pNew->m_bIsOnlyText = bOnlyText;
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
    : m_nErr( 0 )
{
    INetURLObject aObj(rFile);
    const OUString sFileName = aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    switch( SwImpBlocks::GetFileType( rFile ) )
    {
    case SwImpBlocks::FileType::XML:    m_pImp.reset( new SwXMLTextBlocks( sFileName ) ); break;
    case SwImpBlocks::FileType::NoFile: m_pImp.reset( new SwXMLTextBlocks( sFileName ) ); break;
    default: break;
    }
    if( m_pImp )
        return;

    m_nErr = ERR_SWG_FILE_FORMAT_ERROR;
}

SwTextBlocks::~SwTextBlocks()
{
}

OUString SwTextBlocks::GetName() const
{
    return m_pImp ? m_pImp->m_aName : OUString();
}

void SwTextBlocks::SetName( const OUString& r )
{
    if( m_pImp )
        m_pImp->SetName( r );
}

sal_uInt16 SwTextBlocks::GetCount() const
{
    return m_pImp ? m_pImp->GetCount() : 0;
}

sal_uInt16 SwTextBlocks::GetIndex( const OUString& r ) const
{
    return m_pImp ? m_pImp->GetIndex( r ) : USHRT_MAX;
}

sal_uInt16 SwTextBlocks::GetLongIndex( const OUString& r ) const
{
    return m_pImp ? m_pImp->GetLongIndex( r ) : USHRT_MAX;
}

OUString SwTextBlocks::GetShortName( sal_uInt16 n ) const
{
    if( m_pImp )
        return m_pImp->GetShortName( n );
    return OUString();
}

OUString SwTextBlocks::GetLongName( sal_uInt16 n ) const
{
    if( m_pImp )
        return m_pImp->GetLongName( n );
    return OUString();
}

bool SwTextBlocks::Delete( sal_uInt16 n )
{
    if( m_pImp && !m_pImp->m_bInPutMuchBlocks )
    {
        if( m_pImp->IsFileChanged() )
            m_nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
        else if( ERRCODE_NONE == (m_nErr = m_pImp->OpenFile( false ) ))
        {
            m_nErr = m_pImp->Delete( n );
            if( !m_nErr )
            {
                m_pImp->m_aNames.erase( m_pImp->m_aNames.begin() + n );
            }
            if( n == m_pImp->m_nCurrentIndex )
                m_pImp->m_nCurrentIndex = USHRT_MAX;
            if( !m_nErr )
                m_nErr = m_pImp->MakeBlockList();
        }
        m_pImp->CloseFile();
        m_pImp->Touch();

        return ( m_nErr == ERRCODE_NONE );
    }
    return false;
}

void SwTextBlocks::Rename( sal_uInt16 n, const OUString* s, const OUString* l )
{
    if( !m_pImp || m_pImp->m_bInPutMuchBlocks )
        return;

    m_pImp->m_nCurrentIndex = USHRT_MAX;
    OUString aNew;
    OUString aLong;
    if( s )
        aNew = aLong = *s;
    if( l )
        aLong = *l;
    if( aNew.isEmpty() )
    {
        OSL_ENSURE( false, "No short name provided in the rename" );
        m_nErr = ERR_SWG_INTERNAL_ERROR;
        return;
    }

    if( m_pImp->IsFileChanged() )
        m_nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
    else if( ERRCODE_NONE == ( m_nErr = m_pImp->OpenFile( false )))
    {
        // Set the new entry in the list before we do that!
        aNew = GetAppCharClass().uppercase( aNew );
        m_nErr = m_pImp->Rename( n, aNew );
        if( !m_nErr )
        {
            bool bOnlyText = m_pImp->m_aNames[ n ]->m_bIsOnlyText;
            m_pImp->m_aNames.erase( m_pImp->m_aNames.begin() + n );
            m_pImp->AddName( aNew, aLong, bOnlyText );
            m_nErr = m_pImp->MakeBlockList();
        }
    }
    m_pImp->CloseFile();
    m_pImp->Touch();
}

bool SwTextBlocks::BeginGetDoc( sal_uInt16 n )
{
    if( m_pImp && !m_pImp->m_bInPutMuchBlocks )
    {
        if( m_pImp->IsFileChanged() )
            m_nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
        else if( ERRCODE_NONE == ( m_nErr = m_pImp->OpenFile()))
        {
            m_pImp->ClearDoc();
            m_nErr = m_pImp->GetDoc( n );
            if( m_nErr )
                m_pImp->m_nCurrentIndex = USHRT_MAX;
            else
                m_pImp->m_nCurrentIndex = n;
        }
        return ( m_nErr == ERRCODE_NONE );
    }
    return false;
}

void SwTextBlocks::EndGetDoc()
{
    if( m_pImp && !m_pImp->m_bInPutMuchBlocks )
        m_pImp->CloseFile();
}

bool SwTextBlocks::BeginPutDoc( const OUString& s, const OUString& l )
{
    if( m_pImp )
    {
        bool bOk = m_pImp->m_bInPutMuchBlocks;
        if( !bOk )
        {
            if( m_pImp->IsFileChanged() )
                m_nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
            else
                m_nErr = m_pImp->OpenFile( false );
            bOk = ERRCODE_NONE == m_nErr;
        }
        if( bOk )
        {
            const OUString aNew = GetAppCharClass().uppercase(s);
            m_nErr = m_pImp->BeginPutDoc( aNew, l );
        }
        if( m_nErr )
            m_pImp->CloseFile();
    }
    return ERRCODE_NONE == m_nErr;
}

sal_uInt16 SwTextBlocks::PutDoc()
{
    sal_uInt16 nIdx = USHRT_MAX;
    if( m_pImp )
    {
        m_nErr = m_pImp->PutDoc();
        if( !m_nErr )
        {
            m_pImp->m_nCurrentIndex = GetIndex( m_pImp->m_aShort );
            if( m_pImp->m_nCurrentIndex != USHRT_MAX )
                m_pImp->m_aNames[ m_pImp->m_nCurrentIndex ]->m_aLong = m_pImp->m_aLong;
            else
            {
                m_pImp->AddName( m_pImp->m_aShort, m_pImp->m_aLong );
                m_pImp->m_nCurrentIndex = m_pImp->GetIndex( m_pImp->m_aShort );
            }
            if( !m_pImp->m_bInPutMuchBlocks )
                m_nErr = m_pImp->MakeBlockList();
        }
        if( !m_pImp->m_bInPutMuchBlocks )
        {
            m_pImp->CloseFile();
            m_pImp->Touch();
        }
        nIdx = m_pImp->m_nCurrentIndex;
    }
    return nIdx;
}

sal_uInt16 SwTextBlocks::PutText( const OUString& rShort, const OUString& rName,
                                  const OUString& rText )
{
    sal_uInt16 nIdx = USHRT_MAX;
    if( m_pImp )
    {
        bool bOk = m_pImp->m_bInPutMuchBlocks;
        if( !bOk )
        {
            if( m_pImp->IsFileChanged() )
                m_nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
            else
                m_nErr = m_pImp->OpenFile( false );
            bOk = ERRCODE_NONE == m_nErr;
        }
        if( bOk )
        {
            OUString aNew = GetAppCharClass().uppercase( rShort );
            m_nErr = m_pImp->PutText( aNew, rName, rText );
            m_pImp->m_nCurrentIndex = USHRT_MAX;
            if( !m_nErr )
            {
                nIdx = GetIndex( m_pImp->m_aShort );
                if( nIdx != USHRT_MAX )
                    m_pImp->m_aNames[ nIdx ]->m_aLong = rName;
                else
                {
                    m_pImp->AddName( m_pImp->m_aShort, rName, true );
                    nIdx = m_pImp->GetIndex( m_pImp->m_aShort );
                }
                if( !m_pImp->m_bInPutMuchBlocks )
                    m_nErr = m_pImp->MakeBlockList();
            }
        }
        if( !m_pImp->m_bInPutMuchBlocks )
        {
            m_pImp->CloseFile();
            m_pImp->Touch();
        }
    }
    return nIdx;
}

SwDoc* SwTextBlocks::GetDoc()
{
    if( m_pImp )
        return m_pImp->m_xDoc.get();
    return nullptr;
}

void SwTextBlocks::ClearDoc()
{
    if( m_pImp )
    {
        m_pImp->ClearDoc();
        m_pImp->m_nCurrentIndex = USHRT_MAX;
    }
}

OUString const & SwTextBlocks::GetFileName() const
{
    return m_pImp->GetFileName();
}

bool SwTextBlocks::IsReadOnly() const
{
    return m_pImp->m_bReadOnly;
}

bool SwTextBlocks::IsOnlyTextBlock( sal_uInt16 nIdx ) const
{
    bool bRet = false;
    if( m_pImp && !m_pImp->m_bInPutMuchBlocks )
    {
        SwBlockName* pBlkNm = m_pImp->m_aNames[ nIdx ].get();
        if( !pBlkNm->m_bIsOnlyTextFlagInit &&
            !m_pImp->IsFileChanged() && !m_pImp->OpenFile() )
        {
            pBlkNm->m_bIsOnlyText = m_pImp->IsOnlyTextBlock( pBlkNm->m_aShort );
            pBlkNm->m_bIsOnlyTextFlagInit = true;
            m_pImp->CloseFile();
        }
        bRet = pBlkNm->m_bIsOnlyText;
    }
    return bRet;
}

bool SwTextBlocks::IsOnlyTextBlock( const OUString& rShort ) const
{
    sal_uInt16 nIdx = m_pImp->GetIndex( rShort );
    if( USHRT_MAX != nIdx )
    {
        if( m_pImp->m_aNames[ nIdx ]->m_bIsOnlyTextFlagInit )
            return m_pImp->m_aNames[ nIdx ]->m_bIsOnlyText;
        return IsOnlyTextBlock( nIdx );
    }

    OSL_ENSURE( false, "Invalid name" );
    return false;
}

bool SwTextBlocks::GetMacroTable( sal_uInt16 nIdx, SvxMacroTableDtor& rMacroTable )
{
    bool bRet = true;
    if ( m_pImp && !m_pImp->m_bInPutMuchBlocks )
        bRet = ( ERRCODE_NONE == m_pImp->GetMacroTable( nIdx, rMacroTable ) );
    return bRet;
}

bool SwTextBlocks::SetMacroTable( sal_uInt16 nIdx, const SvxMacroTableDtor& rMacroTable )
{
    bool bRet = true;
    if ( m_pImp && !m_pImp->m_bInPutMuchBlocks )
        bRet = ( ERRCODE_NONE == m_pImp->SetMacroTable( nIdx, rMacroTable ) );
    return bRet;
}

bool SwTextBlocks::StartPutMuchBlockEntries()
{
    bool bRet = false;
    if( m_pImp )
        bRet = m_pImp->PutMuchEntries( true );
    return bRet;
}

void SwTextBlocks::EndPutMuchBlockEntries()
{
    if( m_pImp )
        m_pImp->PutMuchEntries( false );
}

OUString SwTextBlocks::GetBaseURL() const
{
    if(m_pImp)
        return m_pImp->GetBaseURL();
    return OUString();
}

void SwTextBlocks::SetBaseURL( const OUString& rURL )
{
    if(m_pImp)
        m_pImp->SetBaseURL(rURL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
