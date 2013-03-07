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


#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <comphelper/string.hxx>
#include <svl/fstathelper.hxx>

#include <svtools/txtcmp.hxx>

#include <sfx2/docfile.hxx>

#include <xmloff/odffields.hxx>

#include <editeng/unolingu.hxx>

#include <swtypes.hxx>
#include <editsh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <pam.hxx>
#include <viewopt.hxx>
#include <ndtxt.hxx>
#include <swundo.hxx>
#include <txttxmrk.hxx>
#include <edimp.hxx>
#include <tox.hxx>
#include <doctxm.hxx>
#include <docary.hxx>
#include <mdiexp.hxx>
#include <statstr.hrc>
#include <bookmrk.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

/*--------------------------------------------------------------------
     Beschreibung: Verzeichnismarkierung ins Dokument einfuegen/loeschen
 --------------------------------------------------------------------*/


void SwEditShell::Insert(const SwTOXMark& rMark)
{
    sal_Bool bInsAtPos = rMark.IsAlternativeText();
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

    mpDoc->DeleteTOXMark( pMark );

    EndAllAction();
}


/*--------------------------------------------------------------------
     Beschreibung: Alle Verzeichnismarkierungen am SPoint zusammensuchen
 --------------------------------------------------------------------*/

sal_uInt16 SwEditShell::GetCurTOXMarks(SwTOXMarks& rMarks) const
{
    return GetDoc()->GetCurTOXMark( *GetCrsr()->Start(), rMarks );
}

sal_Bool SwEditShell::IsTOXBaseReadonly(const SwTOXBase& rTOXBase) const
{
    OSL_ENSURE( rTOXBase.ISA( SwTOXBaseSection ), "no TOXBaseSection!" );
    const SwTOXBaseSection& rTOXSect = (const SwTOXBaseSection&)rTOXBase;
    return  rTOXSect.IsProtect();
}

void SwEditShell::SetTOXBaseReadonly(const SwTOXBase& rTOXBase, sal_Bool bReadonly)
{
    OSL_ENSURE( rTOXBase.ISA( SwTOXBaseSection ), "no TOXBaseSection!" );
    const SwTOXBaseSection& rTOXSect = (const SwTOXBaseSection&)rTOXBase;
    ((SwTOXBase&)rTOXBase).SetProtected(bReadonly);
    OSL_ENSURE( rTOXSect.SwSection::GetType() == TOX_CONTENT_SECTION, "not a TOXContentSection" );

    SwSectionData aSectionData(rTOXSect);
    aSectionData.SetProtectFlag(bReadonly);
    UpdateSection( GetSectionFmtPos( *rTOXSect.GetFmt()  ), aSectionData, 0 );
}

const SwTOXBase*    SwEditShell::GetDefaultTOXBase( TOXTypes eTyp, bool bCreate )
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
    const SwTOXBaseSection* pTOX = mpDoc->InsertTableOf(
                                        *GetCrsr()->GetPoint(), rTOX, pSet, true );
    OSL_ENSURE(pTOX, "Kein aktuelles Verzeichnis");

    // Formatierung anstossen
    CalcLayout();

    // Seitennummern eintragen
    ((SwTOXBaseSection*)pTOX)->UpdatePageNum();

    pTOX->SetPosAtStartEnd( *GetCrsr()->GetPoint() );

    // Fix fuer leere Verzeichnisse
    InvalidateWindows( maVisArea );
    ::EndProgress( pDocSh );
    EndAllAction();
}

/*--------------------------------------------------------------------
     Beschreibung: Verzeichnisinhalt erneuern
 --------------------------------------------------------------------*/

sal_Bool SwEditShell::UpdateTableOf( const SwTOXBase& rTOX, const SfxItemSet* pSet )
{
    sal_Bool bRet = sal_False;

    OSL_ENSURE( rTOX.ISA( SwTOXBaseSection ),  "keine TOXBaseSection!" );
    SwTOXBaseSection* pTOX = (SwTOXBaseSection*)&rTOX;
    OSL_ENSURE(pTOX, "Keine aktuelles Verzeichnis");
    const SwSectionNode* pSectNd;
    if( pTOX && 0 != ( pSectNd = pTOX->GetFmt()->GetSectionNode() ) )
    {
        SwDoc* pMyDoc = GetDoc();
        SwDocShell* pDocSh = pMyDoc->GetDocShell();

        bool bInIndex = pTOX == GetCurTOX();
        SET_CURR_SHELL( this );
        StartAllAction();

        ::StartProgress( STR_STATSTR_TOX_UPDATE, 0, 0, pDocSh );
        ::SetProgressText( STR_STATSTR_TOX_UPDATE, pDocSh );

        pMyDoc->GetIDocumentUndoRedo().StartUndo(UNDO_TOXCHANGE, NULL);

        // Verzeichnisrumpf erzeugen
        pTOX->Update(pSet);

        // Cursor korrigieren
        if( bInIndex )
            pTOX->SetPosAtStartEnd( *GetCrsr()->GetPoint() );

        // Formatierung anstossen
        CalcLayout();

        // Seitennummern eintragen
        pTOX->UpdatePageNum();

        pMyDoc->GetIDocumentUndoRedo().EndUndo(UNDO_TOXCHANGE, NULL);

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

bool SwEditShell::DeleteTOX( const SwTOXBase& rTOXBase, bool bDelNodes )
{
    return GetDoc()->DeleteTOX( (SwTOXBase&)rTOXBase, bDelNodes );
}

/*--------------------------------------------------------------------
     Beschreibung: Typen der Verzeichnisse verwalten
 --------------------------------------------------------------------*/

const SwTOXType* SwEditShell::GetTOXType(TOXTypes eTyp, sal_uInt16 nId) const
{
    return mpDoc->GetTOXType(eTyp, nId);
}

/*--------------------------------------------------------------------
     Beschreibung: Schluessel fuer Stichwortverzeichnisse verwalten
 --------------------------------------------------------------------*/

sal_uInt16 SwEditShell::GetTOIKeys( SwTOIKeyType eTyp, std::vector<String>& rArr ) const
{
    return GetDoc()->GetTOIKeys( eTyp, rArr );
}


sal_uInt16 SwEditShell::GetTOXCount() const
{
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    sal_uInt16 nRet = 0;
    for( sal_uInt16 n = rFmts.size(); n; )
    {
        const SwSection* pSect = rFmts[ --n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFmt()->GetSectionNode() )
            ++nRet;
    }
    return nRet;
}


const SwTOXBase* SwEditShell::GetTOX( sal_uInt16 nPos ) const
{
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    for( sal_uInt16 n = 0, nCnt = 0; n < rFmts.size(); ++n )
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
void SwEditShell::SetUpdateTOX( sal_Bool bFlag )
{
    GetDoc()->SetUpdateTOX( bFlag );
}


sal_Bool SwEditShell::IsUpdateTOX() const
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
    sal_Bool bDoesUndo = DoesUndo();
    DoUndo(sal_False);
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

        SwTOXMarks aMarks;
        SwTOXMark::InsertTOXMarks( aMarks, *pTOXType );
        for( sal_uInt16 nMark=0; nMark<aMarks.size(); nMark++ )
        {
            SwTOXMark* pMark = aMarks[nMark];
            if(pMark->IsAutoGenerated() && pMark->GetTxtTOXMark())
                // mba: test iteration; objects are deleted in iteration
                DeleteTOXMark(pMark);
        }

        //2.
        SfxMedium aMedium( sAutoMarkURL, STREAM_STD_READ );
        SvStream& rStrm = *aMedium.GetInStream();
        const sal_Unicode cZero('0');
        Push();
        rtl_TextEncoding eChrSet = ::osl_getThreadTextEncoding();

        //
        // SearchOptions to be used in loop below
        //
        bool bCaseSensitive = true;
        bool bWordOnly      = false;
        bool bSrchInSel     = false;
        bool bLEV_Relaxed   = true;
        sal_Int32 nLEV_Other    = 2;    //  -> changedChars;
        sal_Int32 nLEV_Longer   = 3;    //! -> deletedChars;
        sal_Int32 nLEV_Shorter  = 1;    //! -> insertedChars;
        sal_Int32 nTransliterationFlags = 0;
        //
        sal_Int32 nSrchFlags = 0;
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
                            SvtSysLocale().GetLanguageTag().getLocale(),
                            nLEV_Other, nLEV_Longer, nLEV_Shorter,
                            nTransliterationFlags );

        while( !rStrm.GetError() && !rStrm.IsEof() )
        {
            rtl::OString aRdLine;
            rStrm.ReadLine( aRdLine );

            // # -> comment
            // ; -> delimiter between entries ->
            // Format: TextToSearchFor;AlternativeString;PrimaryKey;SecondaryKey;CaseSensitive;WordOnly
            // Leading and trailing blanks are ignored
            if( !aRdLine.isEmpty() && '#' != aRdLine[0] )
            {
                String sLine(rtl::OStringToOUString(aRdLine, eChrSet));

                sal_Int32 nTokenPos = 0;
                String sToSelect( sLine.GetToken(0, ';', nTokenPos ) );
                if( sToSelect.Len() )
                {
                    String sAlternative = sLine.GetToken(0, ';', nTokenPos);
                    String sPrimary     = sLine.GetToken(0, ';', nTokenPos);
                    String sSecondary   = sLine.GetToken(0, ';', nTokenPos);
                    String sCase        = sLine.GetToken(0, ';', nTokenPos);
                    String sWordOnly    = sLine.GetToken(0, ';', nTokenPos);

                    //3.
                    bCaseSensitive  = sCase.Len() && !comphelper::string::equals(sCase, cZero);
                    bWordOnly       = sWordOnly.Len() && !comphelper::string::equals(sWordOnly, cZero);
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
                    sal_Bool bCancel;

                    // todo/mba: assuming that notes shouldn't be searched
                    sal_Bool bSearchInNotes = sal_False;
                    sal_uLong nRet = Find( aSearchOpt,  bSearchInNotes, DOCPOS_START, DOCPOS_END, bCancel,
                                    (FindRanges)(FND_IN_SELALL|FND_IN_BODYONLY|FND_IN_OTHER),
                                    sal_False );

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
                        pTmpMark->SetMainEntry(sal_False);
                        pTmpMark->SetAutoGenerated(sal_True);
                        //4.
                        SwEditShell::Insert(*pTmpMark);
                    }
                }
            }
        }
        KillPams();
        Pop(sal_False);
    }
    DoUndo(bDoesUndo);
    EndAllAction();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
