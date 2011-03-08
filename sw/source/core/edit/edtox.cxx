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
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>


#include <tools/urlobj.hxx>
#include <svtools/txtcmp.hxx>
#include <svl/fstathelper.hxx>
#include <sfx2/docfile.hxx>
#include "editeng/unolingu.hxx"
#include <swtypes.hxx>
#include <editsh.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <viewopt.hxx>
#include <ndtxt.hxx>
#include <swundo.hxx>
#include <undobj.hxx>
#include <txttxmrk.hxx>
#include <edimp.hxx>
#include <tox.hxx>
#include <doctxm.hxx>
#include <docary.hxx>
#include <mdiexp.hxx>
#include <statstr.hrc>
#include <bookmrk.hxx>
#include <xmloff/odffields.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

/*--------------------------------------------------------------------
     Beschreibung: Verzeichnismarkierung ins Dokument einfuegen/loeschen
 --------------------------------------------------------------------*/


void SwEditShell::Insert(const SwTOXMark& rMark)
{
    BOOL bInsAtPos = rMark.IsAlternativeText();
    StartAllAction();
    FOREACHPAM_START(this)

        const SwPosition *pStt = PCURCRSR->Start(),
                         *pEnd = PCURCRSR->End();
        if( bInsAtPos )
        {
            SwPaM aTmp( *pStt );
            GetDoc()->InsertPoolItem( aTmp, rMark, 0 );
        }
        else if( *pEnd != *pStt )
        {
            GetDoc()->InsertPoolItem( *PCURCRSR, rMark,
                    nsSetAttrMode::SETATTR_DONTEXPAND );
        }

    FOREACHPAM_END()
    EndAllAction();
}



void SwEditShell::DeleteTOXMark( SwTOXMark* pMark )
{
    SET_CURR_SHELL( this );
    StartAllAction();

    pDoc->DeleteTOXMark( pMark );

    EndAllAction();
}


/*--------------------------------------------------------------------
     Beschreibung: Alle Verzeichnismarkierungen am SPoint zusammensuchen
 --------------------------------------------------------------------*/

USHORT SwEditShell::GetCurTOXMarks(SwTOXMarks& rMarks) const
{
    return GetDoc()->GetCurTOXMark( *GetCrsr()->Start(), rMarks );
}

BOOL SwEditShell::IsTOXBaseReadonly(const SwTOXBase& rTOXBase) const
{
    OSL_ENSURE( rTOXBase.ISA( SwTOXBaseSection ), "no TOXBaseSection!" );
    const SwTOXBaseSection& rTOXSect = (const SwTOXBaseSection&)rTOXBase;
    return  rTOXSect.IsProtect();
}

void SwEditShell::SetTOXBaseReadonly(const SwTOXBase& rTOXBase, BOOL bReadonly)
{
    OSL_ENSURE( rTOXBase.ISA( SwTOXBaseSection ), "no TOXBaseSection!" );
    const SwTOXBaseSection& rTOXSect = (const SwTOXBaseSection&)rTOXBase;
    ((SwTOXBase&)rTOXBase).SetProtected(bReadonly);
    OSL_ENSURE( rTOXSect.SwSection::GetType() == TOX_CONTENT_SECTION, "not a TOXContentSection" );

    SwSectionData aSectionData(rTOXSect);
    aSectionData.SetProtectFlag(bReadonly);
    UpdateSection( GetSectionFmtPos( *rTOXSect.GetFmt()  ), aSectionData, 0 );
}

const SwTOXBase*    SwEditShell::GetDefaultTOXBase( TOXTypes eTyp, BOOL bCreate )
{
    return GetDoc()->GetDefaultTOXBase( eTyp, bCreate );
}

void    SwEditShell::SetDefaultTOXBase(const SwTOXBase& rBase)
{
    GetDoc()->SetDefaultTOXBase(rBase);
}

/*--------------------------------------------------------------------
     Beschreibung: Verzeichnis einfuegen, und Inhalt erzeugen
 --------------------------------------------------------------------*/

void SwEditShell::InsertTableOf( const SwTOXBase& rTOX, const SfxItemSet* pSet )
{
    SET_CURR_SHELL( this );
    StartAllAction();

    SwDocShell* pDocSh = GetDoc()->GetDocShell();
    ::StartProgress( STR_STATSTR_TOX_INSERT, 0, 0, pDocSh );
    ::SetProgressText( STR_STATSTR_TOX_INSERT, pDocSh );

    // Einfuegen des Verzeichnisses
    const SwTOXBaseSection* pTOX = pDoc->InsertTableOf(
                                        *GetCrsr()->GetPoint(), rTOX, pSet, TRUE );
    OSL_ENSURE(pTOX, "Kein aktuelles Verzeichnis");

    // Formatierung anstossen
    CalcLayout();

    // Seitennummern eintragen
    ((SwTOXBaseSection*)pTOX)->UpdatePageNum();

    pTOX->SetPosAtStartEnd( *GetCrsr()->GetPoint() );

    // Fix fuer leere Verzeichnisse
    InvalidateWindows( aVisArea );
    ::EndProgress( pDocSh );
    EndAllAction();
}

/*--------------------------------------------------------------------
     Beschreibung: Verzeichnisinhalt erneuern
 --------------------------------------------------------------------*/

BOOL SwEditShell::UpdateTableOf( const SwTOXBase& rTOX, const SfxItemSet* pSet )
{
    BOOL bRet = FALSE;

    OSL_ENSURE( rTOX.ISA( SwTOXBaseSection ),  "keine TOXBaseSection!" );
    SwTOXBaseSection* pTOX = (SwTOXBaseSection*)&rTOX;
    OSL_ENSURE(pTOX, "Keine aktuelles Verzeichnis");
    const SwSectionNode* pSectNd;
    if( pTOX && 0 != ( pSectNd = pTOX->GetFmt()->GetSectionNode() ) )
    {
        SwDoc* pMyDoc = GetDoc();
        SwDocShell* pDocSh = pMyDoc->GetDocShell();

        BOOL bInIndex = pTOX == GetCurTOX();
        SET_CURR_SHELL( this );
        StartAllAction();

        ::StartProgress( STR_STATSTR_TOX_UPDATE, 0, 0, pDocSh );
        ::SetProgressText( STR_STATSTR_TOX_UPDATE, pDocSh );

        pMyDoc->StartUndo(UNDO_TOXCHANGE, NULL);

        // Verzeichnisrumpf erzeugen
        pTOX->Update(pSet);

        // Cursor korrigieren
        if( bInIndex )
            pTOX->SetPosAtStartEnd( *GetCrsr()->GetPoint() );

        // Formatierung anstossen
        CalcLayout();

        // Seitennummern eintragen
        pTOX->UpdatePageNum();

        pMyDoc->EndUndo(UNDO_TOXCHANGE, NULL);

        ::EndProgress( pDocSh );
        EndAllAction();
    }
    return bRet;
}

/*--------------------------------------------------------------------
     Beschreibung: Aktuelles Verzeichnis vor oder in dem der Cursor
                                   steht
 --------------------------------------------------------------------*/

const SwTOXBase* SwEditShell::GetCurTOX() const
{
    return GetDoc()->GetCurTOX( *GetCrsr()->GetPoint() );
}

BOOL SwEditShell::DeleteTOX( const SwTOXBase& rTOXBase, BOOL bDelNodes )
{
    return GetDoc()->DeleteTOX( (SwTOXBase&)rTOXBase, bDelNodes );
}

/*--------------------------------------------------------------------
     Beschreibung: Typen der Verzeichnisse verwalten
 --------------------------------------------------------------------*/

const SwTOXType* SwEditShell::GetTOXType(TOXTypes eTyp, USHORT nId) const
{
    return pDoc->GetTOXType(eTyp, nId);
}

/*--------------------------------------------------------------------
     Beschreibung: Schluessel fuer Stichwortverzeichnisse verwalten
 --------------------------------------------------------------------*/

USHORT SwEditShell::GetTOIKeys( SwTOIKeyType eTyp, SvStringsSort& rArr ) const
{
    return GetDoc()->GetTOIKeys( eTyp, rArr );
}


USHORT SwEditShell::GetTOXCount() const
{
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    USHORT nRet = 0;
    for( USHORT n = rFmts.Count(); n; )
    {
        const SwSection* pSect = rFmts[ --n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode() )
            ++nRet;
    }
    return nRet;
}


const SwTOXBase* SwEditShell::GetTOX( USHORT nPos ) const
{
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    for( USHORT n = 0, nCnt = 0; n < rFmts.Count(); ++n )
    {
        const SwSection* pSect = rFmts[ n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode() &&
            nCnt++ == nPos )
        {
            OSL_ENSURE( pSect->ISA( SwTOXBaseSection ), "keine TOXBaseSection!" );
            return (SwTOXBaseSection*)pSect;
        }
    }
    return 0;
}


    // nach einlesen einer Datei alle Verzeichnisse updaten
void SwEditShell::SetUpdateTOX( BOOL bFlag )
{
    GetDoc()->SetUpdateTOX( bFlag );
}


BOOL SwEditShell::IsUpdateTOX() const
{
    return GetDoc()->IsUpdateTOX();
}

const String&   SwEditShell::GetTOIAutoMarkURL() const
{
    return GetDoc()->GetTOIAutoMarkURL();
}

void SwEditShell::SetTOIAutoMarkURL(const String& rSet)
{
    GetDoc()->SetTOIAutoMarkURL(rSet);
}

void SwEditShell::ApplyAutoMark()
{
    StartAllAction();
    BOOL bDoesUndo = DoesUndo();
    DoUndo(FALSE);
    //1. remove all automatic generated index entries if AutoMarkURL has a
    //   length and the file exists
    //2. load file
    //3. select all occurrences of the searched words
    //4. apply index entries

    String sAutoMarkURL(GetDoc()->GetTOIAutoMarkURL());
    if( sAutoMarkURL.Len() && FStatHelper::IsDocument( sAutoMarkURL ))
    {
        //1.
        const SwTOXType* pTOXType = GetTOXType(TOX_INDEX, 0);
        SwClientIter aIter(*(SwTOXType*)pTOXType);
        SwTOXMark* pMark = (SwTOXMark*)aIter.First(TYPE(SwTOXMark));
        while( pMark )
        {
            if(pMark->IsAutoGenerated() && pMark->GetTxtTOXMark())
                DeleteTOXMark(pMark);
            pMark = (SwTOXMark*)aIter.Next();
        }

        //2.
        SfxMedium aMedium( sAutoMarkURL, STREAM_STD_READ, TRUE );
        SvStream& rStrm = *aMedium.GetInStream();
        const String sZero('0');
        Push();
        rtl_TextEncoding eChrSet = ::gsl_getSystemTextEncoding();

        //
        // SearchOptions to be used in loop below
        //
        BOOL bCaseSensitive = TRUE;
        BOOL bWordOnly      = FALSE;
        BOOL bSrchInSel     = FALSE;
        BOOL bLEV_Relaxed   = TRUE;
        INT32 nLEV_Other    = 2;    //  -> changedChars;
        INT32 nLEV_Longer   = 3;    //! -> deletedChars;
        INT32 nLEV_Shorter  = 1;    //! -> insertedChars;
        INT32 nTransliterationFlags = 0;
        //
        INT32 nSrchFlags = 0;
        if (!bCaseSensitive)
        {
            nSrchFlags |= SearchFlags::ALL_IGNORE_CASE;
            nTransliterationFlags |= TransliterationModules_IGNORE_CASE;
        }
        if ( bWordOnly)
            nSrchFlags |= SearchFlags::NORM_WORD_ONLY;
        if ( bLEV_Relaxed)
            nSrchFlags |= SearchFlags::LEV_RELAXED;
        if ( bSrchInSel)
            nSrchFlags |= (SearchFlags::REG_NOT_BEGINOFLINE |
                            SearchFlags::REG_NOT_ENDOFLINE );
        //
        rtl::OUString sEmpty;
        SearchOptions aSearchOpt(
                            SearchAlgorithms_ABSOLUTE, nSrchFlags,
                            sEmpty, sEmpty,
                            SvxCreateLocale( LANGUAGE_SYSTEM ),
                            nLEV_Other, nLEV_Longer, nLEV_Shorter,
                            nTransliterationFlags );

        while( !rStrm.GetError() && !rStrm.IsEof() )
        {
            ByteString aRdLine;
            rStrm.ReadLine( aRdLine );

            // # -> comment
            // ; -> delimiter between entries ->
            // Format: TextToSearchFor;AlternativeString;PrimaryKey;SecondaryKey;CaseSensitive;WordOnly
            // Leading and trailing blanks are ignored
            if( aRdLine.Len() && '#' != aRdLine.GetChar(0) )
            {
                String sLine( aRdLine, eChrSet );

                xub_StrLen nTokenPos = 0;
                String sToSelect( sLine.GetToken(0, ';', nTokenPos ) );
                if( sToSelect.Len() )
                {
                    String sAlternative = sLine.GetToken(0, ';', nTokenPos);
                    String sPrimary     = sLine.GetToken(0, ';', nTokenPos);
                    String sSecondary   = sLine.GetToken(0, ';', nTokenPos);
                    String sCase        = sLine.GetToken(0, ';', nTokenPos);
                    String sWordOnly    = sLine.GetToken(0, ';', nTokenPos);

                    //3.
                    bCaseSensitive  = sCase.Len() && sCase != sZero;
                    bWordOnly       = sWordOnly.Len() && sWordOnly != sZero;
                    //
                    if (!bCaseSensitive)
                    {
                        aSearchOpt.transliterateFlags |=
                                     TransliterationModules_IGNORE_CASE;
                    }
                    else
                    {
                        aSearchOpt.transliterateFlags &=
                                    ~TransliterationModules_IGNORE_CASE;
                    }
                    if ( bWordOnly)
                        aSearchOpt.searchFlag |=  SearchFlags::NORM_WORD_ONLY;
                    else
                        aSearchOpt.searchFlag &= ~SearchFlags::NORM_WORD_ONLY;
                    //
                    aSearchOpt.searchString = sToSelect;

                    KillPams();
                    BOOL bCancel;

                    // todo/mba: assuming that notes shouldn't be searched
                    BOOL bSearchInNotes = FALSE;
                    ULONG nRet = Find( aSearchOpt,  bSearchInNotes, DOCPOS_START, DOCPOS_END, bCancel,
                                    (FindRanges)(FND_IN_SELALL|FND_IN_BODYONLY),
                                    FALSE );

                    if(nRet)
                    {
                        SwTOXMark* pTmpMark = new SwTOXMark(pTOXType);
                        if( sPrimary.Len() )
                        {
                            pTmpMark->SetPrimaryKey( sPrimary );
                            if( sSecondary.Len() )
                                pTmpMark->SetSecondaryKey( sSecondary );
                        }
                        if(sAlternative.Len())
                            pTmpMark->SetAlternativeText(sAlternative);
                        pTmpMark->SetMainEntry(FALSE);
                        pTmpMark->SetAutoGenerated(TRUE);
                        //4.
                        SwEditShell::Insert(*pTmpMark);
                    }
                }
            }
        }
        KillPams();
        Pop(FALSE);
    }
    DoUndo(bDoesUndo);
    EndAllAction();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
