/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


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

SV_IMPL_OP_PTRARR_SORT( SwBlockNames, SwBlockName* );

//////////////////////////////////////////////////////////////////////////

// Hash-Code errechnen (muss nicht eindeutig sein)


USHORT SwImpBlocks::Hash( const String& r )
{
    USHORT n = 0;
    xub_StrLen nLen = r.Len();
    if( nLen > 8 )
        nLen = 8;
    const sal_Unicode* p = r.GetBuffer();
    while( nLen-- )
        n = ( n << 1 ) + *p++;
    return n;
}


SwBlockName::SwBlockName( const String& rShort, const String& rLong, long n )
    : nPos( n ), aShort( rShort ), aLong( rLong ), aPackageName (rShort),
    bIsOnlyTxtFlagInit( FALSE ), bIsOnlyTxt( FALSE )
{
    nHashS = SwImpBlocks::Hash( rShort );
    nHashL = SwImpBlocks::Hash( rLong );
}
SwBlockName::SwBlockName( const String& rShort, const String& rLong, const String& rPackageName)
    : nPos( 0 ), aShort( rShort ), aLong( rLong ), aPackageName (rPackageName),
    bIsOnlyTxtFlagInit( FALSE ), bIsOnlyTxt( FALSE )
{
    nHashS = SwImpBlocks::Hash( rShort );
    nHashL = SwImpBlocks::Hash( rLong );
}


// Ist die angegebene Datei ein Storage oder gibt es sie nicht?

short SwImpBlocks::GetFileType( const String& rFile )
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


SwImpBlocks::SwImpBlocks( const String& rFile, BOOL )
    : aFile( rFile ), pDoc( 0 ), nCur( (USHORT)-1 ),
    bReadOnly( TRUE ), bInPutMuchBlocks( FALSE )
{
    FStatHelper::GetModifiedDateTimeOfFile( rFile,
                                            &aDateModified, &aTimeModified );
    INetURLObject aObj(rFile);
    aObj.setExtension( aEmptyStr );
    aName = aObj.GetBase();
}


SwImpBlocks::~SwImpBlocks()
{
    aNames.DeleteAndDestroy( 0, aNames.Count() );
}

// Loeschen des Inhaltes des Dokuments
void SwImpBlocks::ClearDoc()
{
    pDoc->ClearDoc();
}

ULONG SwImpBlocks::GetDocForConversion( USHORT n )
{
    return GetDoc( n );
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


USHORT SwImpBlocks::GetCount() const
{
    return aNames.Count();
}

// Case Insensitive
USHORT SwImpBlocks::GetIndex( const String& rShort ) const
{
    String s( GetAppCharClass().upper( rShort ) );
    USHORT nHash = Hash( s );
    for( USHORT i = 0; i < aNames.Count(); i++ )
    {
        SwBlockName* pName = aNames[ i ];
        if( pName->nHashS == nHash
         && pName->aShort == s )
            return i;
    }
    return (USHORT) -1;
}


USHORT SwImpBlocks::GetLongIndex( const String& rLong ) const
{
    USHORT nHash = Hash( rLong );
    for( USHORT i = 0; i < aNames.Count(); i++ )
    {
        SwBlockName* pName = aNames[ i ];
        if( pName->nHashL == nHash
         && pName->aLong == rLong )
            return i;
    }
    return (USHORT) -1;
}


const String& SwImpBlocks::GetShortName( USHORT n ) const
{
    if( n < aNames.Count() )
        return aNames[ n ]->aShort;
    return aEmptyStr;
}


const String& SwImpBlocks::GetLongName( USHORT n ) const
{
    if( n < aNames.Count() )
        return aNames[ n ]->aLong;
    return aEmptyStr;
}

const String& SwImpBlocks::GetPackageName( USHORT n ) const
{
    if( n < aNames.Count() )
        return aNames[ n ]->aPackageName;
    return aEmptyStr;
}

void SwImpBlocks::AddName( const String& rShort, const String& rLong,
                            BOOL bOnlyTxt )
{
    USHORT nIdx = GetIndex( rShort );
    if( nIdx != (USHORT) -1 )
        aNames.DeleteAndDestroy( nIdx );
    SwBlockName* pNew = new SwBlockName( rShort, rLong, 0L );
    pNew->bIsOnlyTxtFlagInit = TRUE;
    pNew->bIsOnlyTxt = bOnlyTxt;
    aNames.C40_PTR_INSERT( SwBlockName, pNew );
}



BOOL SwImpBlocks::IsFileChanged() const
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

BOOL SwImpBlocks::IsOnlyTextBlock( const String& ) const
{
    return FALSE;
}

ULONG SwImpBlocks::GetMacroTable( USHORT, SvxMacroTableDtor&, sal_Bool )
{
    return 0;
}

ULONG SwImpBlocks::SetMacroTable( USHORT ,
                                const SvxMacroTableDtor& , sal_Bool )
{
    return 0;
}

BOOL SwImpBlocks::PutMuchEntries( BOOL )
{
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////


SwTextBlocks::SwTextBlocks( const String& rFile )
    : pImp( 0 ), nErr( 0 )
{
    INetURLObject aObj(rFile);
    String sFileName = aObj.GetMainURL( INetURLObject::NO_DECODE );
    switch( SwImpBlocks::GetFileType( rFile ) )
    {
    //case SWBLK_SW2:     pImp = new Sw2TextBlocks( sFileName );  break;
    //case SWBLK_SW3:     pImp = new Sw3TextBlocks( sFileName );  break;
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

const String& SwTextBlocks::GetName()
{
    return pImp ? pImp->aName : aEmptyStr;
}


void SwTextBlocks::SetName( const String& r )
{
    if( pImp )
        pImp->SetName( r );
}


BOOL SwTextBlocks::IsOld() const
{
    if (pImp)
    {
        short nType = pImp->GetFileType();
        if (SWBLK_SW3 == nType || SWBLK_SW2 == nType )
            return TRUE;
    }
    return FALSE;
}


/*
ULONG SwTextBlocks::ConvertToNew()
{
    // Wir nehmen die aktuelle Datei, benennen diese in .BAK um
    // und kreieren den neuen Storage
    if( IsOld() )
    {
        // Erst mal muessen wir die Datei freigeben
        short nType = pImp->GetFileType();
        Sw2TextBlocks *pTwo = NULL;
        Sw3TextBlocks *pThree = NULL;
        SwImpBlocks *pOld = NULL;

        pImp->nCur = (USHORT) -1;
        String aName( pImp->aFile );
        delete pImp; pImp = NULL;
        // Jetzt wird umbenannt
        INetURLObject aOldFull( aName );
        INetURLObject aNewFull( aName );

        aOldFull.SetExtension( String::CreateFromAscii("bak") );
        String aOld( aOldFull.GetMainURL( INetURLObject::NO_DECODE ) );
        String aNew( aNewFull.GetMainURL( INetURLObject::NO_DECODE ) );

        BOOL bError = !SWUnoHelper::UCB_CopyFile( aNew, aOld, TRUE );
        if( bError )
        {
            if (nType == SWBLK_SW2)
                pImp = new Sw2TextBlocks( aOld );
            else
                pImp = new Sw3TextBlocks( aOld );
            return nErr = ERR_SWG_CANNOT_WRITE;
        }

        // Die Datei ist erfolgreich umbenannt. Jetzt wird der Storage
        // aufgesetzt
        if (nType == SWBLK_SW2)
            pOld = pTwo = new Sw2TextBlocks( aOld );
        else
            pOld = pThree = new Sw3TextBlocks( aOld );
        SwXMLTextBlocks* pNew = new SwXMLTextBlocks( aName );
        pNew->SetName ( pOld->GetName());
        // Wir kopieren den Doc-Ptr in das alte System
        // den alten SvPersist heben wir uns aber auf,
        // da dieser die ganze Zeit leben bleibt
        // und lesen die Dateivorlagen erneut ein
        SvPersist* pPersist2 = pOld->pDoc->GetPersist();
        if (SWBLK_SW2 == nType )
        {
            delete pOld->pDoc;
            pOld->pDoc = pNew->pDoc;nLinkCt
            nErr = pTwo->LoadDoc();
        }
        else
        {
            nErr = pThree->OpenFile ( TRUE );
            // Within this call, Sw3IoImp::SetDoc calls RemoveLink
            // on the old document, and deletes it if the
            // ref count is now zero
            pThree->SetDoc ( pNew->pDoc );
            pOld->pDoc->AddLink();
        }
        if( !nErr && 0 == ( nErr = pNew->OpenFile( FALSE )) )
        {
            nErr = pNew->SetConvertMode( TRUE );
            // jetzt werden die Bausteine einfach umkopiert!
            if( !nErr )
            {
                if (SWBLK_SW2 == nType)
                    pTwo->StatLineStartPercent();
                USHORT nCount = pOld->GetCount();
                for( USHORT i = 0; i < nCount; i++ )
                {
                    pNew->ClearDoc();
                    String aShort( pOld->GetShortName( i ) );
                    String aLong( pOld->GetLongName( i ) );
                    pNew->AddName( aShort, aLong );
                    if ( SWBLK_SW3 == nType && pThree->IsOnlyTextBlock(aShort) )
                    {
                        String sText;
                        pThree->GetText( aShort, sText );
                        pNew->PutText( aShort, aLong, sText );
                    }
                    else
                    {
                        if (SWBLK_SW2 == nType )
                        {
                            // I think this is how it should work (!!!!!!) mtg
                            pNew->pDoc->SetPersist( pPersist2 );
                        }
                        nErr = pOld->GetDocForConversion( i );
                        if( nErr )
                            break;
                        nErr = pNew->BeginPutDoc( aShort, aLong );
                        if( nErr )
                            break;
                        nErr = pNew->PutDoc();
                        if( nErr )
                            break;
                    }

                    // convert macros, too
                    SvxMacroTableDtor aMacroTable;
                    pOld->GetMacroTable( i, aMacroTable, sal_True );
                    pNew->SetMacroTable( i, aMacroTable, sal_True );

                    if (SWBLK_SW2 == nType )
                        pNew->pDoc->SetPersist( 0 );
                }
                if (SWBLK_SW2 == nType )
                    ::EndProgress( pOld->pDoc->GetDocShell() );
            }
            if( !nErr )
                nErr = pNew->SetConvertMode( FALSE );
        }
        if ( SWBLK_SW3 == nType )
        {
            pThree->CloseFile();
        }
        else
        {
            // Haben wir es geschafft?
            pOld->pDoc = NULL;
        }
        pNew->ClearDoc();
        if( !nErr )
        {
            delete pOld;
            pImp = pNew;
            SWUnoHelper::UCB_DeleteFile( aOld );
            pNew->MakeBlockList();
        }
        else
        {
            delete pOld; delete pNew;
            SWUnoHelper::UCB_DeleteFile( aNew );
            SWUnoHelper::UCB_CopyFile( aOld, aNew, TRUE );
            if ( SWBLK_SW2 == nType )
                pImp = new Sw2TextBlocks( aOld );
            else
                pImp = new Sw3TextBlocks( aOld );
        }
        pNew->CloseFile();
        FStatHelper::GetModifiedDateTimeOfFile( aNew,
                            &pImp->aDateModified, &pImp->aTimeModified );
    }
    return nErr;
} */


USHORT SwTextBlocks::GetCount() const
{
    return pImp ? pImp->GetCount() : 0;
}


USHORT SwTextBlocks::GetIndex( const String& r ) const
{
    return pImp ? pImp->GetIndex( r ) : (USHORT) -1;
}


USHORT SwTextBlocks::GetLongIndex( const String& r ) const
{
    return pImp ? (USHORT)(pImp->GetLongIndex( r )) : (USHORT) -1;
}


const String& SwTextBlocks::GetShortName( USHORT n ) const
{
    if( pImp )
        return pImp->GetShortName( n );
    return aEmptyStr;
}


const String& SwTextBlocks::GetLongName( USHORT n ) const
{
    if( pImp )
        return pImp->GetLongName( n );
    return aEmptyStr;
}


BOOL SwTextBlocks::Delete( USHORT n )
{
    if( pImp && !pImp->bInPutMuchBlocks )
    {
        if( pImp->IsFileChanged() )
            nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
        else if( 0 == (nErr = pImp->OpenFile( FALSE ) ))
        {
            nErr = pImp->Delete( n );
            if( !nErr )
                pImp->aNames.DeleteAndDestroy( n );
            if( n == pImp->nCur )
                pImp->nCur = (USHORT) -1;
            if( !nErr )
                nErr = pImp->MakeBlockList();
        }
        pImp->CloseFile();
        pImp->Touch();

        return BOOL( nErr == 0 );
    }
    return FALSE;
}


USHORT SwTextBlocks::Rename( USHORT n, const String* s, const String* l )
{
    USHORT nIdx = (USHORT)-1;
    if( pImp && !pImp->bInPutMuchBlocks )
    {
        pImp->nCur = nIdx;
        String aNew, aLong;
        if( s )
            aNew = aLong = *s;
        if( l )
            aLong = *l;
        if( !aNew.Len() )
        {
            OSL_ENSURE( !this, "Kein Kurzname in Rename angegeben" );
            nErr = ERR_SWG_INTERNAL_ERROR; return (USHORT) -1;
        }

        if( pImp->IsFileChanged() )
            nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
        else if( 0 == ( nErr = pImp->OpenFile( FALSE )))
        {
            // Vorher den neuen Eintrag in die Liste setzen!
            GetAppCharClass().toUpper( aNew );
             nErr = pImp->Rename( n, aNew, aLong );
            if( !nErr )
            {
                BOOL bOnlyTxt = pImp->aNames[ n ]->bIsOnlyTxt;
                pImp->aNames.DeleteAndDestroy( n );
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

ULONG SwTextBlocks::CopyBlock( SwTextBlocks& rSource, String& rSrcShort,
                                const String& rLong )
{
    BOOL bIsOld = FALSE;
    if (rSource.pImp)
    {
        short nType = rSource.pImp->GetFileType();
        if (SWBLK_SW2 == nType || SWBLK_SW3 == nType )
            bIsOld = TRUE;
    }
    if( bIsOld ) //rSource.IsOld() )
        nErr = ERR_SWG_OLD_GLOSSARY;
    else if( pImp->bInPutMuchBlocks )
        nErr = ERR_SWG_INTERNAL_ERROR;
    else
        nErr = pImp->CopyBlock(*rSource.pImp, rSrcShort, rLong);
    return nErr;
}

BOOL SwTextBlocks::BeginGetDoc( USHORT n )
{
    if( pImp && !pImp->bInPutMuchBlocks )
    {
// diese Optimierierung darf es nicht mehr geben. OLE-Objecte muessen auf
// ihre SubStorages zugreifem koennen!
//      if( n == pImp->nCur )
//          return TRUE;

        if( pImp->IsFileChanged() )
            nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
        else if( 0 == ( nErr = pImp->OpenFile( TRUE )))
        {
            pImp->ClearDoc();
            nErr = pImp->GetDoc( n );
            if( nErr )
                pImp->nCur = (USHORT)-1;
            else
                pImp->nCur = n;
        }
        return BOOL( nErr == 0 );
    }
    return FALSE;
}


void SwTextBlocks::EndGetDoc()
{
    if( pImp && !pImp->bInPutMuchBlocks )
        pImp->CloseFile();
}


BOOL SwTextBlocks::BeginPutDoc( const String& s, const String& l )
{
    if( pImp )
    {
        BOOL bOk = pImp->bInPutMuchBlocks;
        if( !bOk )
        {
            if( pImp->IsFileChanged() )
                nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
            else
                nErr = pImp->OpenFile( FALSE );
            bOk = 0 == nErr;
        }
        if( bOk )
        {
            String aNew( s );
            GetAppCharClass().toUpper( aNew );
            nErr = pImp->BeginPutDoc( aNew, l );
        }
        if( nErr )
            pImp->CloseFile();
    }
    return 0 == nErr;
}


USHORT SwTextBlocks::PutDoc()
{
    USHORT nIdx = (USHORT)-1;
    if( pImp )
    {
        nErr = pImp->PutDoc();
        if( !nErr )
        {
            pImp->nCur = GetIndex( pImp->aShort );
            if( pImp->nCur != (USHORT) -1 )
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

USHORT SwTextBlocks::PutText( const String& rShort, const String& rName,
                              const String& rTxt )
{
    USHORT nIdx = (USHORT) -1;
    if( pImp )
    {
        BOOL bOk = pImp->bInPutMuchBlocks;
        if( !bOk )
        {
            if( pImp->IsFileChanged() )
                nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
            else
                nErr = pImp->OpenFile( FALSE );
            bOk = 0 == nErr;
        }
        if( bOk )
        {
            String aNew( rShort );
            GetAppCharClass().toUpper( aNew );
            nErr = pImp->PutText( aNew, rName, rTxt );
            pImp->nCur = (USHORT) -1;
            if( !nErr )
            {
                nIdx = GetIndex( pImp->aShort );
                if( nIdx != (USHORT) -1 )
                    pImp->aNames[ nIdx ]->aLong = rName;
                else
                {
                    pImp->AddName( pImp->aShort, rName, TRUE );
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
    pImp->nCur = (USHORT) -1;
}


const String& SwTextBlocks::GetFileName() const
{
    return pImp->GetFileName();
}


BOOL SwTextBlocks::IsReadOnly() const
{
    return pImp->bReadOnly;
}

BOOL SwTextBlocks::IsOnlyTextBlock( USHORT nIdx ) const
{
    BOOL bRet = FALSE;
    if( pImp && !pImp->bInPutMuchBlocks )
    {
        SwBlockName* pBlkNm = pImp->aNames[ nIdx ];
        if( !pBlkNm->bIsOnlyTxtFlagInit &&
            !pImp->IsFileChanged() && !pImp->OpenFile( TRUE ) )
        {
            pBlkNm->bIsOnlyTxt = pImp->IsOnlyTextBlock( pBlkNm->aShort );
            pBlkNm->bIsOnlyTxtFlagInit = TRUE;
            pImp->CloseFile();
        }
        bRet = pBlkNm->bIsOnlyTxt;
    }
    return bRet;
}

BOOL SwTextBlocks::IsOnlyTextBlock( const String& rShort ) const
{
    USHORT nIdx = pImp->GetIndex( rShort );
    if( USHRT_MAX != nIdx )
    {
        if( pImp->aNames[ nIdx ]->bIsOnlyTxtFlagInit )
            return pImp->aNames[ nIdx ]->bIsOnlyTxt;
        return IsOnlyTextBlock( nIdx );
    }

    OSL_ENSURE( !this, "ungueltiger Name" );
    return FALSE;
}

BOOL SwTextBlocks::GetMacroTable( USHORT nIdx, SvxMacroTableDtor& rMacroTbl )
{
    BOOL bRet = TRUE;
    if ( pImp && !pImp->bInPutMuchBlocks )
        bRet = ( 0 == pImp->GetMacroTable( nIdx, rMacroTbl ) );
    return bRet;
}

BOOL SwTextBlocks::SetMacroTable( USHORT nIdx,
                                const SvxMacroTableDtor& rMacroTbl )
{
    BOOL bRet = TRUE;
    if ( pImp && !pImp->bInPutMuchBlocks )
        bRet = ( 0 == pImp->SetMacroTable( nIdx, rMacroTbl ) );
    return bRet;
}

BOOL SwTextBlocks::StartPutMuchBlockEntries()
{
    BOOL bRet = FALSE;
    if( !IsOld() && pImp )
        bRet = pImp->PutMuchEntries( TRUE );
    return bRet;
}

void SwTextBlocks::EndPutMuchBlockEntries()
{
    if( pImp )
        pImp->PutMuchEntries( FALSE );
}

String    SwTextBlocks::GetBaseURL() const
{
    String sRet;
    if(pImp)
        sRet = pImp->GetBaseURL();
    return sRet;
}

void SwTextBlocks::SetBaseURL( const String& rURL )
{
    if(pImp)
        pImp->SetBaseURL(rURL);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
