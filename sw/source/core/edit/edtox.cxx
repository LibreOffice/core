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
#include <osl/thread.h>
#include <unotools/textsearch.hxx>
#include <unotools/syslocale.hxx>

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

// Add/delete listing markers to a document

void SwEditShell::Insert(const SwTOXMark& rMark)
{
    bool bInsAtPos = rMark.IsAlternativeText();
    StartAllAction();
    for(SwPaM& rPaM : GetCursor()->GetRingContainer())
    {
        const SwPosition *pStt = rPaM.Start(),
                         *pEnd = rPaM.End();
        if( bInsAtPos )
        {
            SwPaM aTmp( *pStt );
            GetDoc()->getIDocumentContentOperations().InsertPoolItem( aTmp, rMark );
        }
        else if( *pEnd != *pStt )
        {
            GetDoc()->getIDocumentContentOperations().InsertPoolItem(
                rPaM, rMark, SetAttrMode::DONTEXPAND );
        }

    }
    EndAllAction();
}

void SwEditShell::DeleteTOXMark( SwTOXMark* pMark )
{
    SET_CURR_SHELL( this );
    StartAllAction();

    mpDoc->DeleteTOXMark( pMark );

    EndAllAction();
}

/// Collect all listing markers
sal_uInt16 SwEditShell::GetCurTOXMarks(SwTOXMarks& rMarks) const
{
    return SwDoc::GetCurTOXMark( *GetCursor()->Start(), rMarks );
}

bool SwEditShell::IsTOXBaseReadonly(const SwTOXBase& rTOXBase)
{
    OSL_ENSURE( dynamic_cast<const SwTOXBaseSection*>( &rTOXBase) !=  nullptr, "no TOXBaseSection!" );
    const SwTOXBaseSection& rTOXSect = static_cast<const SwTOXBaseSection&>(rTOXBase);
    return  rTOXSect.IsProtect();
}

void SwEditShell::SetTOXBaseReadonly(const SwTOXBase& rTOXBase, bool bReadonly)
{
    OSL_ENSURE( dynamic_cast<const SwTOXBaseSection*>( &rTOXBase) !=  nullptr, "no TOXBaseSection!" );
    const SwTOXBaseSection& rTOXSect = static_cast<const SwTOXBaseSection&>(rTOXBase);
    const_cast<SwTOXBase&>(rTOXBase).SetProtected(bReadonly);
    OSL_ENSURE( rTOXSect.SwSection::GetType() == TOX_CONTENT_SECTION, "not a TOXContentSection" );

    SwSectionData aSectionData(rTOXSect);
    aSectionData.SetProtectFlag(bReadonly);
    UpdateSection( GetSectionFormatPos( *rTOXSect.GetFormat()  ), aSectionData );
}

const SwTOXBase*    SwEditShell::GetDefaultTOXBase( TOXTypes eTyp, bool bCreate )
{
    return GetDoc()->GetDefaultTOXBase( eTyp, bCreate );
}

void    SwEditShell::SetDefaultTOXBase(const SwTOXBase& rBase)
{
    GetDoc()->SetDefaultTOXBase(rBase);
}

/// Insert listing and create content
void SwEditShell::InsertTableOf( const SwTOXBase& rTOX, const SfxItemSet* pSet )
{
    SET_CURR_SHELL( this );
    StartAllAction();

    SwDocShell* pDocSh = GetDoc()->GetDocShell();
    ::StartProgress( STR_STATSTR_TOX_INSERT, 0, 0, pDocSh );
    ::SetProgressText( STR_STATSTR_TOX_INSERT, pDocSh );

    // Insert listing
    const SwTOXBaseSection* pTOX = mpDoc->InsertTableOf(
                                        *GetCursor()->GetPoint(), rTOX, pSet, true );
    OSL_ENSURE(pTOX, "No current TOx");

    // start formatting
    CalcLayout();

    // insert page numbering
    const_cast<SwTOXBaseSection*>(pTOX)->UpdatePageNum();

    pTOX->SetPosAtStartEnd( *GetCursor()->GetPoint() );

    // Fix for empty listing
    InvalidateWindows( maVisArea );
    ::EndProgress( pDocSh );
    EndAllAction();
}

/// update tables of content
bool SwEditShell::UpdateTableOf( const SwTOXBase& rTOX, const SfxItemSet* pSet )
{
    bool bRet = false;

    OSL_ENSURE( dynamic_cast<const SwTOXBaseSection*>( &rTOX) !=  nullptr,  "no TOXBaseSection!" );
    SwTOXBaseSection* pTOX = const_cast<SwTOXBaseSection*>(static_cast<const SwTOXBaseSection*>(&rTOX));
    OSL_ENSURE(pTOX, "no current listing");
    if( pTOX && nullptr != pTOX->GetFormat()->GetSectionNode() )
    {
        SwDoc* pMyDoc = GetDoc();
        SwDocShell* pDocSh = pMyDoc->GetDocShell();

        bool bInIndex = pTOX == GetCurTOX();
        SET_CURR_SHELL( this );
        StartAllAction();

        ::StartProgress( STR_STATSTR_TOX_UPDATE, 0, 0, pDocSh );
        ::SetProgressText( STR_STATSTR_TOX_UPDATE, pDocSh );

        pMyDoc->GetIDocumentUndoRedo().StartUndo(UNDO_TOXCHANGE, nullptr);

        // create listing stub
        pTOX->Update(pSet);

        // correct Cursor
        if( bInIndex )
            pTOX->SetPosAtStartEnd( *GetCursor()->GetPoint() );

        // start formatting
        CalcLayout();

        // insert page numbering
        pTOX->UpdatePageNum();

        pMyDoc->GetIDocumentUndoRedo().EndUndo(UNDO_TOXCHANGE, nullptr);

        ::EndProgress( pDocSh );
        EndAllAction();
    }
    return bRet;
}

/// Get current listing before or at the Cursor
const SwTOXBase* SwEditShell::GetCurTOX() const
{
    return SwDoc::GetCurTOX( *GetCursor()->GetPoint() );
}

bool SwEditShell::DeleteTOX( const SwTOXBase& rTOXBase, bool bDelNodes )
{
    return GetDoc()->DeleteTOX( (SwTOXBase&)rTOXBase, bDelNodes );
}

// manage types of listings

const SwTOXType* SwEditShell::GetTOXType(TOXTypes eTyp, sal_uInt16 nId) const
{
    return mpDoc->GetTOXType(eTyp, nId);
}

// manage keys for the alphabetical index

sal_uInt16 SwEditShell::GetTOIKeys( SwTOIKeyType eTyp, std::vector<OUString>& rArr ) const
{
    return GetDoc()->GetTOIKeys( eTyp, rArr );
}

sal_uInt16 SwEditShell::GetTOXCount() const
{
    const SwSectionFormats& rFormats = GetDoc()->GetSections();
    sal_uInt16 nRet = 0;
    for( auto n = rFormats.size(); n; )
    {
        const SwSection* pSect = rFormats[ --n ]->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFormat()->GetSectionNode() )
            ++nRet;
    }
    return nRet;
}

const SwTOXBase* SwEditShell::GetTOX( sal_uInt16 nPos ) const
{
    const SwSectionFormats& rFormats = GetDoc()->GetSections();
    sal_uInt16 nCnt {0};
    for( const SwSectionFormat *pFormat : rFormats )
    {
        const SwSection* pSect = pFormat->GetSection();
        if( TOX_CONTENT_SECTION == pSect->GetType() &&
            pSect->GetFormat()->GetSectionNode() &&
            nCnt++ == nPos )
        {
            OSL_ENSURE( dynamic_cast<const SwTOXBaseSection*>( pSect) !=  nullptr, "no TOXBaseSection!" );
            return static_cast<const SwTOXBaseSection*>(pSect);
        }
    }
    return nullptr;
}

/** Update of all listings after reading-in a file */
void SwEditShell::SetUpdateTOX( bool bFlag )
{
    GetDoc()->SetUpdateTOX( bFlag );
}

bool SwEditShell::IsUpdateTOX() const
{
    return GetDoc()->IsUpdateTOX();
}

OUString SwEditShell::GetTOIAutoMarkURL() const
{
    return GetDoc()->GetTOIAutoMarkURL();
}

void SwEditShell::SetTOIAutoMarkURL(const OUString& rSet)
{
    GetDoc()->SetTOIAutoMarkURL(rSet);
}

void SwEditShell::ApplyAutoMark()
{
    StartAllAction();
    bool bDoesUndo = DoesUndo();
    DoUndo(false);
    //1. remove all automatic generated index entries if AutoMarkURL has a
    //   length and the file exists
    //2. load file
    //3. select all occurrences of the searched words
    //4. apply index entries

    OUString sAutoMarkURL(GetDoc()->GetTOIAutoMarkURL());
    if( !sAutoMarkURL.isEmpty() && FStatHelper::IsDocument( sAutoMarkURL ))
    {
        //1.
        const SwTOXType* pTOXType = GetTOXType(TOX_INDEX, 0);

        SwTOXMarks aMarks;
        SwTOXMark::InsertTOXMarks( aMarks, *pTOXType );
        for( SwTOXMark* pMark : aMarks )
        {
            if(pMark->IsAutoGenerated() && pMark->GetTextTOXMark())
                // mba: test iteration; objects are deleted in iteration
                DeleteTOXMark(pMark);
        }

        //2.
        SfxMedium aMedium( sAutoMarkURL, STREAM_STD_READ );
        SvStream& rStrm = *aMedium.GetInStream();
        Push();
        rtl_TextEncoding eChrSet = ::osl_getThreadTextEncoding();

        // SearchOptions to be used in loop below
        sal_Int32 nLEV_Other    = 2;    //  -> changedChars;
        sal_Int32 nLEV_Longer   = 3;    //! -> deletedChars;
        sal_Int32 nLEV_Shorter  = 1;    //! -> insertedChars;
        sal_Int32 nTransliterationFlags = 0;

        sal_Int32 nSrchFlags = SearchFlags::LEV_RELAXED;

        SearchOptions aSearchOpt(
                            SearchAlgorithms_ABSOLUTE, nSrchFlags,
                            "", "",
                            SvtSysLocale().GetLanguageTag().getLocale(),
                            nLEV_Other, nLEV_Longer, nLEV_Shorter,
                            nTransliterationFlags );

        while( !rStrm.GetError() && !rStrm.IsEof() )
        {
            OString aRdLine;
            rStrm.ReadLine( aRdLine );

            // # -> comment
            // ; -> delimiter between entries ->
            // Format: TextToSearchFor;AlternativeString;PrimaryKey;SecondaryKey;CaseSensitive;WordOnly
            // Leading and trailing blanks are ignored
            if( !aRdLine.isEmpty() && '#' != aRdLine[0] )
            {
                OUString sLine(OStringToOUString(aRdLine, eChrSet));

                sal_Int32 nTokenPos = 0;
                OUString sToSelect( sLine.getToken(0, ';', nTokenPos ) );
                if( !sToSelect.isEmpty() )
                {
                    OUString sAlternative = sLine.getToken(0, ';', nTokenPos);
                    OUString sPrimary     = sLine.getToken(0, ';', nTokenPos);
                    OUString sSecondary   = sLine.getToken(0, ';', nTokenPos);
                    OUString sCase        = sLine.getToken(0, ';', nTokenPos);
                    OUString sWordOnly    = sLine.getToken(0, ';', nTokenPos);

                    //3.
                    bool bCaseSensitive = !sCase.isEmpty() && sCase != "0";
                    bool bWordOnly = !sWordOnly.isEmpty() && sWordOnly != "0";

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

                    aSearchOpt.searchString = sToSelect;

                    KillPams();
                    bool bCancel;

                    // todo/mba: assuming that notes shouldn't be searched
                    bool bSearchInNotes = false;
                    sal_uLong nRet = Find( aSearchOpt,  bSearchInNotes, DOCPOS_START, DOCPOS_END, bCancel,
                                    (FindRanges)(FND_IN_SELALL) );

                    if(nRet)
                    {
                        SwTOXMark* pTmpMark = new SwTOXMark(pTOXType);
                        if( !sPrimary.isEmpty() )
                        {
                            pTmpMark->SetPrimaryKey( sPrimary );
                            if( !sSecondary.isEmpty() )
                                pTmpMark->SetSecondaryKey( sSecondary );
                        }
                        if( !sAlternative.isEmpty() )
                            pTmpMark->SetAlternativeText(sAlternative);
                        pTmpMark->SetMainEntry(false);
                        pTmpMark->SetAutoGenerated(true);
                        //4.
                        SwEditShell::Insert(*pTmpMark);
                    }
                }
            }
        }
        KillPams();
        Pop(false);
    }
    DoUndo(bDoesUndo);
    EndAllAction();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
