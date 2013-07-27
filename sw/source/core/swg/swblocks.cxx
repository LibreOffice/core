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
#include <mdiexp.hxx>       // Progress
#include <SwXMLTextBlocks.hxx>
#include <docsh.hxx>
#include <swunohelper.hxx>

#include <statstr.hrc>
#include <swerror.h>

//////////////////////////////////////////////////////////////////////////

// Hash-Code errechnen (muss nicht eindeutig sein)


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
    bIsOnlyTxtFlagInit( sal_False ), bIsOnlyTxt( sal_False )
{
    nHashS = SwImpBlocks::Hash( rShort );
    nHashL = SwImpBlocks::Hash( rLong );
}
SwBlockName::SwBlockName( const OUString& rShort, const OUString& rLong, const OUString& rPackageName)
    : aShort( rShort ), aLong( rLong ), aPackageName (rPackageName),
    bIsOnlyTxtFlagInit( sal_False ), bIsOnlyTxt( sal_False )
{
    nHashS = SwImpBlocks::Hash( rShort );
    nHashL = SwImpBlocks::Hash( rLong );
}


// Ist die angegebene Datei ein Storage oder gibt es sie nicht?

short SwImpBlocks::GetFileType( const OUString& rFile )
{
    if( !FStatHelper::IsDocument( rFile ) )
        return SWBLK_NO_FILE;
    if( SwXMLTextBlocks::IsFileUCBStorage( rFile ) )
        return SWBLK_XML;
    if( SvStorage::IsStorageFile( rFile ) )
        return SWBLK_SW3;
    //otherwise return NONE
    return SWBLK_NONE;
}


SwImpBlocks::SwImpBlocks( const OUString& rFile, sal_Bool )
    : aFile( rFile ),
    aDateModified( Date::EMPTY ),
    aTimeModified( Time::EMPTY ),
    pDoc( 0 ), nCur( (sal_uInt16)-1 ),
    bReadOnly( sal_True ), bInPutMuchBlocks( sal_False )
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

// Loeschen des Inhaltes des Dokuments
void SwImpBlocks::ClearDoc()
{
    pDoc->ClearDoc();
}

// Erzeugen eines PaMs, der das ganze Dokument umfasst
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

// Case Insensitive
sal_uInt16 SwImpBlocks::GetIndex( const OUString& rShort ) const
{
    const OUString s( GetAppCharClass().uppercase( rShort ) );
    const sal_uInt16 nHash = Hash( s );
    for( sal_uInt16 i = 0; i < aNames.size(); i++ )
    {
        const SwBlockName* pName = aNames[ i ];
        if( pName->nHashS == nHash
         && pName->aShort == s )
            return i;
    }
    return (sal_uInt16) -1;
}


sal_uInt16 SwImpBlocks::GetLongIndex( const OUString& rLong ) const
{
    sal_uInt16 nHash = Hash( rLong );
    for( sal_uInt16 i = 0; i < aNames.size(); i++ )
    {
        const SwBlockName* pName = aNames[ i ];
        if( pName->nHashL == nHash
         && pName->aLong == rLong )
            return i;
    }
    return (sal_uInt16) -1;
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
                            sal_Bool bOnlyTxt )
{
    sal_uInt16 nIdx = GetIndex( rShort );
    if( nIdx != (sal_uInt16) -1 )
    {
        delete aNames[nIdx];
        aNames.erase( aNames.begin() + nIdx );
    }
    SwBlockName* pNew = new SwBlockName( rShort, rLong );
    pNew->bIsOnlyTxtFlagInit = sal_True;
    pNew->bIsOnlyTxt = bOnlyTxt;
    aNames.insert( pNew );
}



sal_Bool SwImpBlocks::IsFileChanged() const
{
    Date aTempDateModified( aDateModified );
    Time aTempTimeModified( aTimeModified );
    return FStatHelper::GetModifiedDateTimeOfFile( aFile,
                            &aTempDateModified, &aTempTimeModified ) &&
          ( aDateModified != aTempDateModified ||
            aTimeModified != aTempTimeModified );
}


void SwImpBlocks::Touch()
{
    FStatHelper::GetModifiedDateTimeOfFile( aFile,
                                            &aDateModified, &aTimeModified );
}

sal_Bool SwImpBlocks::IsOnlyTextBlock( const OUString& ) const
{
    return sal_False;
}

sal_uLong SwImpBlocks::GetMacroTable( sal_uInt16, SvxMacroTableDtor&, sal_Bool )
{
    return 0;
}

sal_uLong SwImpBlocks::SetMacroTable( sal_uInt16 ,
                                const SvxMacroTableDtor& , sal_Bool )
{
    return 0;
}

sal_Bool SwImpBlocks::PutMuchEntries( sal_Bool )
{
    return sal_False;
}

////////////////////////////////////////////////////////////////////////////


SwTextBlocks::SwTextBlocks( const OUString& rFile )
    : pImp( 0 ), nErr( 0 )
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


sal_Bool SwTextBlocks::IsOld() const
{
    if (pImp)
    {
        short nType = pImp->GetFileType();
        if (SWBLK_SW3 == nType || SWBLK_SW2 == nType )
            return sal_True;
    }
    return sal_False;
}


sal_uInt16 SwTextBlocks::GetCount() const
{
    return pImp ? pImp->GetCount() : 0;
}


sal_uInt16 SwTextBlocks::GetIndex( const OUString& r ) const
{
    return pImp ? pImp->GetIndex( r ) : (sal_uInt16) -1;
}


sal_uInt16 SwTextBlocks::GetLongIndex( const OUString& r ) const
{
    return pImp ? (sal_uInt16)(pImp->GetLongIndex( r )) : (sal_uInt16) -1;
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
    return aEmptyStr;
}


sal_Bool SwTextBlocks::Delete( sal_uInt16 n )
{
    if( pImp && !pImp->bInPutMuchBlocks )
    {
        if( pImp->IsFileChanged() )
            nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
        else if( 0 == (nErr = pImp->OpenFile( sal_False ) ))
        {
            nErr = pImp->Delete( n );
            if( !nErr )
            {
                delete pImp->aNames[n];
                pImp->aNames.erase( pImp->aNames.begin() + n );
            }
            if( n == pImp->nCur )
                pImp->nCur = (sal_uInt16) -1;
            if( !nErr )
                nErr = pImp->MakeBlockList();
        }
        pImp->CloseFile();
        pImp->Touch();

        return sal_Bool( nErr == 0 );
    }
    return sal_False;
}


sal_uInt16 SwTextBlocks::Rename( sal_uInt16 n, const OUString* s, const OUString* l )
{
    sal_uInt16 nIdx = (sal_uInt16)-1;
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
            OSL_ENSURE( !this, "Kein Kurzname in Rename angegeben" );
            nErr = ERR_SWG_INTERNAL_ERROR; return (sal_uInt16) -1;
        }

        if( pImp->IsFileChanged() )
            nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
        else if( 0 == ( nErr = pImp->OpenFile( sal_False )))
        {
            // Vorher den neuen Eintrag in die Liste setzen!
            aNew = GetAppCharClass().uppercase( aNew );
             nErr = pImp->Rename( n, aNew, aLong );
            if( !nErr )
            {
                sal_Bool bOnlyTxt = pImp->aNames[ n ]->bIsOnlyTxt;
                delete pImp->aNames[n];
                pImp->aNames.erase( pImp->aNames.begin() + n );
                pImp->AddName( aNew, aLong, bOnlyTxt );
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

sal_Bool SwTextBlocks::BeginGetDoc( sal_uInt16 n )
{
    if( pImp && !pImp->bInPutMuchBlocks )
    {
// diese Optimierierung darf es nicht mehr geben. OLE-Objecte muessen auf
// ihre SubStorages zugreifem koennen!
//      if( n == pImp->nCur )
//          return sal_True;

        if( pImp->IsFileChanged() )
            nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
        else if( 0 == ( nErr = pImp->OpenFile( sal_True )))
        {
            pImp->ClearDoc();
            nErr = pImp->GetDoc( n );
            if( nErr )
                pImp->nCur = (sal_uInt16)-1;
            else
                pImp->nCur = n;
        }
        return sal_Bool( nErr == 0 );
    }
    return sal_False;
}


void SwTextBlocks::EndGetDoc()
{
    if( pImp && !pImp->bInPutMuchBlocks )
        pImp->CloseFile();
}


sal_Bool SwTextBlocks::BeginPutDoc( const OUString& s, const OUString& l )
{
    if( pImp )
    {
        sal_Bool bOk = pImp->bInPutMuchBlocks;
        if( !bOk )
        {
            if( pImp->IsFileChanged() )
                nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
            else
                nErr = pImp->OpenFile( sal_False );
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
    sal_uInt16 nIdx = (sal_uInt16)-1;
    if( pImp )
    {
        nErr = pImp->PutDoc();
        if( !nErr )
        {
            pImp->nCur = GetIndex( pImp->aShort );
            if( pImp->nCur != (sal_uInt16) -1 )
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
                                  const OUString& rTxt )
{
    sal_uInt16 nIdx = (sal_uInt16) -1;
    if( pImp )
    {
        sal_Bool bOk = pImp->bInPutMuchBlocks;
        if( !bOk )
        {
            if( pImp->IsFileChanged() )
                nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
            else
                nErr = pImp->OpenFile( sal_False );
            bOk = 0 == nErr;
        }
        if( bOk )
        {
            OUString aNew = GetAppCharClass().uppercase( rShort );
            nErr = pImp->PutText( aNew, rName, rTxt );
            pImp->nCur = (sal_uInt16) -1;
            if( !nErr )
            {
                nIdx = GetIndex( pImp->aShort );
                if( nIdx != (sal_uInt16) -1 )
                    pImp->aNames[ nIdx ]->aLong = rName;
                else
                {
                    pImp->AddName( pImp->aShort, rName, sal_True );
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
    return 0;
}


void SwTextBlocks::ClearDoc()
{
    if( pImp )
        pImp->ClearDoc();
    pImp->nCur = (sal_uInt16) -1;
}


OUString SwTextBlocks::GetFileName() const
{
    return pImp->GetFileName();
}


sal_Bool SwTextBlocks::IsReadOnly() const
{
    return pImp->bReadOnly;
}

sal_Bool SwTextBlocks::IsOnlyTextBlock( sal_uInt16 nIdx ) const
{
    sal_Bool bRet = sal_False;
    if( pImp && !pImp->bInPutMuchBlocks )
    {
        SwBlockName* pBlkNm = const_cast<SwBlockName*>( pImp->aNames[ nIdx ] );
        if( !pBlkNm->bIsOnlyTxtFlagInit &&
            !pImp->IsFileChanged() && !pImp->OpenFile( sal_True ) )
        {
            pBlkNm->bIsOnlyTxt = pImp->IsOnlyTextBlock( pBlkNm->aShort );
            pBlkNm->bIsOnlyTxtFlagInit = sal_True;
            pImp->CloseFile();
        }
        bRet = pBlkNm->bIsOnlyTxt;
    }
    return bRet;
}

sal_Bool SwTextBlocks::IsOnlyTextBlock( const OUString& rShort ) const
{
    sal_uInt16 nIdx = pImp->GetIndex( rShort );
    if( USHRT_MAX != nIdx )
    {
        if( pImp->aNames[ nIdx ]->bIsOnlyTxtFlagInit )
            return pImp->aNames[ nIdx ]->bIsOnlyTxt;
        return IsOnlyTextBlock( nIdx );
    }

    OSL_ENSURE( !this, "ungueltiger Name" );
    return sal_False;
}

sal_Bool SwTextBlocks::GetMacroTable( sal_uInt16 nIdx, SvxMacroTableDtor& rMacroTbl )
{
    sal_Bool bRet = sal_True;
    if ( pImp && !pImp->bInPutMuchBlocks )
        bRet = ( 0 == pImp->GetMacroTable( nIdx, rMacroTbl ) );
    return bRet;
}

sal_Bool SwTextBlocks::SetMacroTable( sal_uInt16 nIdx,
                                const SvxMacroTableDtor& rMacroTbl )
{
    sal_Bool bRet = sal_True;
    if ( pImp && !pImp->bInPutMuchBlocks )
        bRet = ( 0 == pImp->SetMacroTable( nIdx, rMacroTbl ) );
    return bRet;
}

sal_Bool SwTextBlocks::StartPutMuchBlockEntries()
{
    sal_Bool bRet = sal_False;
    if( !IsOld() && pImp )
        bRet = pImp->PutMuchEntries( sal_True );
    return bRet;
}

void SwTextBlocks::EndPutMuchBlockEntries()
{
    if( pImp )
        pImp->PutMuchEntries( sal_False );
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
