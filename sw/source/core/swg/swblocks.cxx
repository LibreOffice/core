/*************************************************************************
 *
 *  $RCSfile: swblocks.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-09-27 12:20:16 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef SVTOOLS_FSTATHELPER_HXX
#include <svtools/fstathelper.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _SWBLOCKS_HXX
#include <swblocks.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>       // Progress
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>
#endif


using namespace ::com::sun::star;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

#define C2U(cChar) rtl::OUString::createFromAscii(cChar)
SV_IMPL_OP_PTRARR_SORT( SwBlockNames, SwBlockName* );

//////////////////////////////////////////////////////////////////////////

// Hash-Code errechnen (muss nicht eindeutig sein)


USHORT SwImpBlocks::Hash( const String& r )
{
    USHORT n = 0;
    xub_StrLen nLen = r.Len();
    if( nLen > 8 ) nLen = 8;
    const sal_Unicode* p = (const sal_Unicode*) r.GetBuffer();
    while( nLen-- ) n = ( n << 1 ) + *p++;
    return n;
}


SwBlockName::SwBlockName( const String& rShort, const String& rLong, long n )
    : aShort( rShort ), aLong( rLong ), nPos( n ),
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
    if( SvStorage::IsStorageFile( rFile ) )
        return SWBLK_SW3;
    // Kein Storage: Ist es eine SWG-Datei?
    const SfxFilter* pFltr = SwIoSystem::GetFileFilter( rFile, aEmptyStr );
    return( pFltr && pFltr->GetUserData().EqualsAscii(FILTER_SWG) )
                            ? SWBLK_SW2 : SWBLK_NONE;
}


SwImpBlocks::SwImpBlocks( const String& rFile, BOOL bMake )
    : aFile( rFile ), bReadOnly( TRUE ), bInPutMuchBlocks( FALSE ),
    nCur( (USHORT)-1 ), pDoc( 0 )
{
    FStatHelper::GetModifiedDateTimeOfFile( rFile,
                                            &aDateModified, &aTimeModified );
    INetURLObject aObj(rFile);
    aObj.setExtension( aEmptyStr );
    aName = aObj.GetBase();

    aName.Erase(aName.Len() - 4, 4); //remove extension
//  aDEntry.SetExtension( aEmptyStr );
//  aName = aDEntry.GetName();
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

ULONG SwImpBlocks::GetMacroTable( USHORT, SvxMacroTableDtor& )
{
    return 0;
}

BOOL SwImpBlocks::PutMuchEntries( BOOL bOn )
{
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////


SwTextBlocks::SwTextBlocks( const String& rFile )
    : pImp( 0 ), nErr( 0 )
{
    INetURLObject aObj(rFile);
    String sFileName = aObj.GetMainURL();
    switch( SwImpBlocks::GetFileType( rFile ) )
    {
        case SWBLK_SW2:
            pImp = new Sw2TextBlocks( sFileName ); break;
        case SWBLK_SW3:
            pImp = new Sw3TextBlocks( sFileName ); break;
        case SWBLK_NO_FILE:
            pImp = new Sw3TextBlocks( sFileName ); break;
    }
    if( !pImp )
        nErr = ERR_SWG_FILE_FORMAT_ERROR;
}


SwTextBlocks::SwTextBlocks( SvStorage& rStg )
    : nErr( 0 )
{
    pImp = new Sw3TextBlocks( rStg );
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
    return pImp ? pImp->IsOld() : FALSE;
}


ULONG SwTextBlocks::ConvertToNew()
{
    // Wir nehmen die aktuelle Datei, benennen diese in .BAK um
    // und kreieren den neuen Storage
    if( IsOld() )
    {
        // Erst mal muessen wir die Datei freigeben
        pImp->nCur = (USHORT) -1;
        String aName( pImp->aFile );
        delete pImp; pImp = NULL;
        // Jetzt wird umbenannt
        INetURLObject aOldFull( aName );
        INetURLObject aNewFull( aName );
        aOldFull.SetExtension( String::CreateFromAscii("bak") );
        String aOld( aOldFull.GetFull() );
        BOOL bError = FALSE;
        try
        {
            String sMain(aOldFull.GetMainURL());
            sal_Unicode cSlash = '/';
            xub_StrLen nSlashPos = sMain.SearchBackward(cSlash);
            sMain.Erase(nSlashPos);
            ::ucb::Content aNewContent( sMain,
                                        Reference< XCommandEnvironment > ());
            Any aAny;
            TransferInfo aInfo;
            aInfo.NameClash = NameClash::OVERWRITE;
            aInfo.NewTitle = aOldFull.GetName();
            aInfo.SourceURL = aNewFull.GetMainURL();
            aInfo.MoveData  = TRUE;
            aAny <<= aInfo;
            aNewContent.executeCommand( C2U( "transfer" ), aAny);
        }
        catch(...)
        {
            bError = TRUE;
        }

        if( bError )
        {
            pImp = new Sw2TextBlocks( aName );
            return nErr = ERR_SWG_CANNOT_WRITE;
        }

        // Die Datei ist erfolgreich umbenannt. Jetzt wird der Storage
        // aufgesetzt
        Sw2TextBlocks* pOld = new Sw2TextBlocks( aOld );
        Sw3TextBlocks* pNew = new Sw3TextBlocks( aName );
        // Wir kopieren den Doc-Ptr in das alte System
        // den alten SvPersist heben wir uns aber auf,
        // da dieser die ganze Zeit leben bleibt
        SvPersist* pPersist2 = pOld->pDoc->GetPersist();
        delete pOld->pDoc; pOld->pDoc = pNew->pDoc;
        // und lesen die Dateivorlagen erneut ein
        nErr = pOld->LoadDoc();
        if( !nErr && 0 == ( nErr = pNew->OpenFile( FALSE )) )
        {
            nErr = pNew->SetConvertMode( TRUE );
            // jetzt werden die Bausteine einfach umkopiert!
            if( !nErr )
            {
                pOld->StatLineStartPercent();
                for( USHORT i = 0; i < pOld->GetCount(); i++ )
                {
                    pNew->ClearDoc();
                    pNew->pDoc->SetPersist( pPersist2 );
                    nErr = pOld->GetDoc( i );
                    if( nErr )
                        break;
                    String aShort( pOld->GetShortName( i ) );
                    String aLong( pOld->GetLongName( i ) );
                    nErr = pNew->BeginPutDoc( aShort, aLong );
                    if( nErr )
                        break;
                    nErr = pNew->PutDoc();
                    if( nErr )
                        break;
                    pNew->AddName( aShort, aLong );
                    pNew->pDoc->SetPersist( 0 );
                }
                ::EndProgress( pOld->pDoc->GetDocShell() );
            }
            if( !nErr )
                nErr = pNew->SetConvertMode( FALSE );
        }
        // Haben wir es geschafft?
        pOld->pDoc = NULL;
        pNew->ClearDoc();
        if( !nErr )
        {
            delete pOld;
            pImp = pNew;
            try
            {
                ::ucb::Content aOldContent(
                        aOldFull.GetMainURL(),
                        Reference< XCommandEnvironment > ());
                aOldContent.executeCommand( C2U( "delete" ),
                                    makeAny( sal_Bool( sal_True ) ) );
            }
            catch(...){}
            pNew->MakeBlockList();
        }
        else
        {
            delete pOld; delete pNew;
            try
            {
                String sMain(aNewFull.GetMainURL());
                sal_Unicode cSlash = '/';
                xub_StrLen nSlashPos = sMain.SearchBackward(cSlash);
                sMain.Erase(nSlashPos);
                ::ucb::Content aNewContent( sMain, Reference< XCommandEnvironment > ());
                Any aAny;
                TransferInfo aInfo;
                aInfo.NameClash = NameClash::OVERWRITE;
                aInfo.NewTitle = aNewFull.GetName();
                aInfo.SourceURL = aOldFull.GetMainURL();
                aInfo.MoveData  = TRUE;
                aAny <<= aInfo;
                aNewContent.executeCommand( C2U( "transfer" ), aAny);
            }
            catch(...){}
            pImp = new Sw2TextBlocks( aName );
        }
        pNew->CloseFile();

        try
        {
            ::ucb::Content aContent(
                aNewFull.GetMainURL(), Reference< XCommandEnvironment > ());
            aContent.executeCommand( C2U( "delete" ), makeAny( sal_Bool( sal_True ) ) );
        }
        catch(...){}

        FStatHelper::GetModifiedDateTimeOfFile( aNewFull.GetMainURL(),
                                &pImp->aDateModified, &pImp->aTimeModified );
    }
    return nErr;
}


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
            ASSERT( !this, "Kein Kurzname in Rename angegeben" );
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
    if( rSource.IsOld() )
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
                if( !pImp->bInPutMuchBlocks )
                    nErr = pImp->MakeBlockList();
            }
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


const String& SwTextBlocks::GetText( USHORT n )
{
    if( pImp && !pImp->bInPutMuchBlocks )
    {
        if( pImp->IsFileChanged() )
            nErr = ERR_TXTBLOCK_NEWFILE_ERROR;
        else if( 0 == ( nErr = pImp->OpenFile( TRUE )))
        {
            nErr = pImp->GetText( n, pImp->aCur );
            pImp->CloseFile();
            if( !nErr )
                return pImp->aCur;
        }
    }
    return aEmptyStr;
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
                    if( !pImp->bInPutMuchBlocks )
                        nErr = pImp->MakeBlockList();
                }
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

    ASSERT( !this, "ungueltiger Name" );
    return FALSE;
}

BOOL SwTextBlocks::GetMacroTable( USHORT nIdx, SvxMacroTableDtor& rMacroTbl )
{
    return ( pImp && !pImp->bInPutMuchBlocks )
                ? 0 == pImp->GetMacroTable( nIdx, rMacroTbl )
                : TRUE;
}


String SwTextBlocks::GetValidShortCut( const String& rLong,
                                         BOOL bCheckInBlock ) const
{
    String sRet;
    xub_StrLen nLen = rLong.Len();
    if( nLen )
    {
        xub_StrLen nStart = 0;
        while( ' ' == rLong.GetChar( nStart ) )
            if( ++nStart < nLen )
                break;

        if( nStart < nLen )
        {
            sal_Unicode cCurr, cPrev = rLong.GetChar( nStart );
            sRet = cPrev;
            for( ++nStart; nStart < nLen; ++nStart, cPrev = cCurr )
                if( ' ' == cPrev &&
                    ' ' != ( cCurr = rLong.GetChar( nStart )) )
                    sRet += cCurr;
        }
        if( bCheckInBlock )
        {
            USHORT nCurPos = GetIndex( sRet );
            nStart = 0;
            nLen = sRet.Len();
            while( (USHORT)-1 != nCurPos )
            {
                sRet.Erase( nLen ) +=
                    String::CreateFromInt32( ++nStart );// add an Number to it
                nCurPos = GetIndex( sRet );
            }
        }
    }
    return sRet;
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



