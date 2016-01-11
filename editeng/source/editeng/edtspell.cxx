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


#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <impedit.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <edtspell.hxx>
#include <editeng/flditem.hxx>
#include <editeng/fontitem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <editeng/unolingu.hxx>
#include <linguistic/lngprops.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::linguistic2;


EditSpellWrapper::EditSpellWrapper( vcl::Window* _pWin,
        Reference< XSpellChecker1 >  &xChecker,
        bool bIsStart, bool bIsAllRight, EditView* pView ) :
    SvxSpellWrapper( _pWin, xChecker, bIsStart, bIsAllRight )
{
    SAL_WARN_IF( !pView, "editeng", "One view has to be abandoned!" );
    // Keep IgnoreList, delete ReplaceList...
    if (SvxGetChangeAllList().is())
        SvxGetChangeAllList()->clear();
    pEditView = pView;
}

void EditSpellWrapper::SpellStart( SvxSpellArea eArea )
{
    EditEngine* pEE = pEditView->GetEditEngine();
    ImpEditEngine* pImpEE = pEditView->GetImpEditEngine();
    SpellInfo* pSpellInfo = pImpEE->GetSpellInfo();

    if ( eArea == SVX_SPELL_BODY_START )
    {
        // Is called when
        // a) Spell-Forward has arrived at the end and should restart at the top
        // IsEndDone() returns also true, when backward-spelling is started at the end!
        if ( IsEndDone() )
        {
            pSpellInfo->bSpellToEnd = false;
            pSpellInfo->aSpellTo = pSpellInfo->aSpellStart;
            pEditView->GetImpEditView()->SetEditSelection(
                    pEE->GetEditDoc().GetStartPaM() );
        }
        else
        {
            pSpellInfo->bSpellToEnd = true;
            pSpellInfo->aSpellTo = pImpEE->CreateEPaM(
                    pEE->GetEditDoc().GetStartPaM() );
        }
    }
    else if ( eArea == SVX_SPELL_BODY_END )
    {
        // Is called when
        // a) Spell-Forward is launched
        // IsStartDone() return also true, when forward-spelling is started at the beginning!
        if ( !IsStartDone() )
        {
            pSpellInfo->bSpellToEnd = true;
            pSpellInfo->aSpellTo = pImpEE->CreateEPaM(
                    pEE->GetEditDoc().GetEndPaM() );
        }
        else
        {
            pSpellInfo->bSpellToEnd = false;
            pSpellInfo->aSpellTo = pSpellInfo->aSpellStart;
            pEditView->GetImpEditView()->SetEditSelection(
                    pEE->GetEditDoc().GetEndPaM() );
        }
    }
    else if ( eArea == SVX_SPELL_BODY )
    {
        ;   // Is handled by the App through SpellNextDocument
    }
    else
    {
        OSL_FAIL( "SpellStart: Unknown Area!" );
    }
}

void EditSpellWrapper::SpellContinue()
{
    SetLast( pEditView->GetImpEditEngine()->ImpSpell( pEditView ) );
}

void EditSpellWrapper::SpellEnd()
{
    // Base class will show language errors...
    SvxSpellWrapper::SpellEnd();
}

bool EditSpellWrapper::HasOtherCnt()
{
    return false;
}

bool EditSpellWrapper::SpellMore()
{
    EditEngine* pEE = pEditView->GetEditEngine();
    ImpEditEngine* pImpEE = pEditView->GetImpEditEngine();
    SpellInfo* pSpellInfo = pImpEE->GetSpellInfo();
    bool bMore = false;
    if ( pSpellInfo->bMultipleDoc )
    {
        bMore = pEE->SpellNextDocument();
        SetCurTextObj( nullptr );
        if ( bMore )
        {
            // The text has been entered into the engine, when backwords then
            // it must be behind the selection.
            pEditView->GetImpEditView()->SetEditSelection(
                        pEE->GetEditDoc().GetStartPaM() );
        }
    }
    return bMore;
}

void EditSpellWrapper::ReplaceAll( const OUString &rNewText,
            sal_Int16 )
{
    // Is called when the word is in ReplaceList of the spell checker
    pEditView->InsertText( rNewText );
    CheckSpellTo();
}

void EditSpellWrapper::CheckSpellTo()
{
    ImpEditEngine* pImpEE = pEditView->GetImpEditEngine();
    SpellInfo* pSpellInfo = pImpEE->GetSpellInfo();
    EditPaM aPaM( pEditView->GetImpEditView()->GetEditSelection().Max() );
    EPaM aEPaM = pImpEE->CreateEPaM( aPaM );
    if ( aEPaM.nPara == pSpellInfo->aSpellTo.nPara )
    {
        // Check if SpellToEnd still has a valid Index, if replace has been
        // performed in the paragraph.
        if ( pSpellInfo->aSpellTo.nIndex > aPaM.GetNode()->Len() )
            pSpellInfo->aSpellTo.nIndex = aPaM.GetNode()->Len();
    }
}

size_t WrongList::Valid = std::numeric_limits<size_t>::max();

WrongList::WrongList() : mnInvalidStart(0), mnInvalidEnd(Valid) {}

WrongList::WrongList(const WrongList& r) :
    maRanges(r.maRanges),
    mnInvalidStart(r.mnInvalidStart),
    mnInvalidEnd(r.mnInvalidEnd) {}

WrongList::~WrongList() {}


void WrongList::SetRanges( const std::vector<editeng::MisspellRange>& rRanges )
{
    maRanges = rRanges;
}

bool WrongList::IsValid() const
{
    return mnInvalidStart == Valid;
}

void WrongList::SetValid()
{
    mnInvalidStart = Valid;
    mnInvalidEnd = 0;
}

void WrongList::SetInvalidRange( size_t nStart, size_t nEnd )
{
    if (mnInvalidStart == Valid || nStart < mnInvalidStart)
        mnInvalidStart = nStart;

    if (mnInvalidEnd < nEnd)
        mnInvalidEnd = nEnd;
}

void WrongList::ResetInvalidRange( size_t nStart, size_t nEnd )
{
    mnInvalidStart = nStart;
    mnInvalidEnd = nEnd;
}

void WrongList::TextInserted( size_t nPos, size_t nLength, bool bPosIsSep )
{
    if (IsValid())
    {
        mnInvalidStart = nPos;
        mnInvalidEnd = nPos + nLength;
    }
    else
    {
        if ( mnInvalidStart > nPos )
            mnInvalidStart = nPos;
        if ( mnInvalidEnd >= nPos )
            mnInvalidEnd = mnInvalidEnd + nLength;
        else
            mnInvalidEnd = nPos + nLength;
    }

    for (size_t i = 0, n = maRanges.size(); i < n; ++i)
    {
        editeng::MisspellRange& rWrong = maRanges[i];
        bool bRefIsValid = true;
        if (rWrong.mnEnd >= nPos)
        {
            // Move all Wrongs after the insert position...
            if (rWrong.mnStart > nPos)
            {
                rWrong.mnStart += nLength;
                rWrong.mnEnd += nLength;
            }
            // 1: Starts before and goes until nPos...
            else if (rWrong.mnEnd == nPos)
            {
                // Should be halted at a blank!
                if ( !bPosIsSep )
                    rWrong.mnEnd += nLength;
            }
            // 2: Starts before and goes until after nPos...
            else if ((rWrong.mnStart < nPos) && (rWrong.mnEnd > nPos))
            {
                rWrong.mnEnd += nLength;
                // When a separator remove and re-examine the Wrong
                if ( bPosIsSep )
                {
                    // Split Wrong...
                    editeng::MisspellRange aNewWrong(rWrong.mnStart, nPos);
                    rWrong.mnStart = nPos + 1;
                    maRanges.insert(maRanges.begin() + i, aNewWrong);
                    // Reference no longer valid after Insert, the other
                    // was inserted in front of this position
                    bRefIsValid = false;
                    ++i; // Not this again...
                }
            }
            // 3: Attribute starts at position ..
            else if (rWrong.mnStart == nPos)
            {
                rWrong.mnEnd += nLength;
                if ( bPosIsSep )
                    ++(rWrong.mnStart);
            }
        }
        SAL_WARN_IF(bRefIsValid && rWrong.mnStart >= rWrong.mnEnd, "editeng",
                "TextInserted, editeng::MisspellRange: Start >= End?!");
        (void)bRefIsValid;
    }

    SAL_WARN_IF(DbgIsBuggy(), "editeng", "InsertWrong: WrongList broken!");
}

void WrongList::TextDeleted( size_t nPos, size_t nLength )
{
    size_t nEndPos = nPos + nLength;
    if (IsValid())
    {
        const size_t nNewInvalidStart = nPos ? nPos - 1 : 0;
        mnInvalidStart = nNewInvalidStart;
        mnInvalidEnd = nNewInvalidStart + 1;
    }
    else
    {
        if ( mnInvalidStart > nPos )
            mnInvalidStart = nPos;
        if ( mnInvalidEnd > nPos )
        {
            if (mnInvalidEnd > nEndPos)
                mnInvalidEnd = mnInvalidEnd - nLength;
            else
                mnInvalidEnd = nPos+1;
        }
    }

    for (WrongList::iterator i = begin(); i != end(); )
    {
        bool bDelWrong = false;
        if (i->mnEnd >= nPos)
        {
            // Move all Wrongs after the insert position...
            if (i->mnStart >= nEndPos)
            {
                i->mnStart -= nLength;
                i->mnEnd -= nLength;
            }
            // 1. Delete Internal Wrongs ...
            else if (i->mnStart >= nPos && i->mnEnd <= nEndPos)
            {
                bDelWrong = true;
            }
            // 2. Wrong begins before, ends inside or behind it ...
            else if (i->mnStart <= nPos && i->mnEnd > nPos)
            {
                if (i->mnEnd <= nEndPos)   // ends inside
                    i->mnEnd = nPos;
                else
                    i->mnEnd -= nLength; // ends after
            }
            // 3. Wrong begins inside, ending after ...
            else if (i->mnStart >= nPos && i->mnEnd > nEndPos)
            {
                i->mnStart = nEndPos - nLength;
                i->mnEnd -= nLength;
            }
        }
        SAL_WARN_IF(i->mnStart >= i->mnEnd, "editeng",
                "TextDeleted, editeng::MisspellRange: Start >= End?!");
        if ( bDelWrong )
        {
            i = maRanges.erase(i);
        }
        else
        {
            ++i;
        }
    }

    SAL_WARN_IF(DbgIsBuggy(), "editeng", "TextDeleted: WrongList broken!");
}

bool WrongList::NextWrong( size_t& rnStart, size_t& rnEnd ) const
{
    /*
        rnStart get the start position, is possibly adjusted wrt. Wrong start
        rnEnd does not have to be initialized.
    */
    for (WrongList::const_iterator i = begin(); i != end(); ++i)
    {
        if (i->mnEnd > rnStart)
        {
            rnStart = i->mnStart;
            rnEnd = i->mnEnd;
            return true;
        }
    }
    return false;
}

bool WrongList::HasWrong( size_t nStart, size_t nEnd ) const
{
    for (WrongList::const_iterator i = begin(); i != end(); ++i)
    {
        if (i->mnStart == nStart && i->mnEnd == nEnd)
            return true;
        else if (i->mnStart >= nStart)
            break;
    }
    return false;
}

bool WrongList::HasAnyWrong( size_t nStart, size_t nEnd ) const
{
    for (WrongList::const_iterator i = begin(); i != end(); ++i)
    {
        if (i->mnEnd >= nStart && i->mnStart < nEnd)
            return true;
        else if (i->mnStart >= nEnd)
            break;
    }
    return false;
}

void WrongList::ClearWrongs( size_t nStart, size_t nEnd,
            const ContentNode* pNode )
{
    for (WrongList::iterator i = begin(); i != end(); )
    {
        if (i->mnEnd > nStart && i->mnStart < nEnd)
        {
            if (i->mnEnd > nEnd) // Runs out
            {
                i->mnStart = nEnd;
                // Blanks?
                while (i->mnStart < (size_t)pNode->Len() &&
                       (pNode->GetChar(i->mnStart) == ' ' ||
                        pNode->IsFeature(i->mnStart)))
                {
                    ++i->mnStart;
                }
                ++i;
            }
            else
            {
                i = maRanges.erase(i);
                // no increment here
            }
        }
        else
        {
            ++i;
        }
    }

    SAL_WARN_IF(DbgIsBuggy(), "editeng", "ClearWrongs: WrongList broken!");
}

void WrongList::InsertWrong( size_t nStart, size_t nEnd )
{
    WrongList::iterator nPos = end();
    for (WrongList::iterator i = begin(); i != end(); ++i)
    {
        if (i->mnStart >= nStart)
        {
            nPos = i;
            {
                // It can really only happen that the Wrong starts exactly here
                // and runs along, but not that there are several ranges ...
                // Exactly in the range is no one allowed to be, otherwise this
                // Method can not be called!
                SAL_WARN_IF((i->mnStart != nStart || i->mnEnd <= nEnd) && i->mnStart <= nEnd, "editeng", "InsertWrong: RangeMismatch!");
                if (i->mnStart == nStart && i->mnEnd > nEnd)
                    i->mnStart = nEnd + 1;
            }
            break;
        }
    }

    if (nPos != maRanges.end())
        maRanges.insert(nPos, editeng::MisspellRange(nStart, nEnd));
    else
        maRanges.push_back(editeng::MisspellRange(nStart, nEnd));

    SAL_WARN_IF(DbgIsBuggy(), "editeng", "InsertWrong: WrongList broken!");
}

void WrongList::MarkWrongsInvalid()
{
    if (!maRanges.empty())
        SetInvalidRange(maRanges.front().mnStart, maRanges.back().mnEnd);
}

WrongList* WrongList::Clone() const
{
    return new WrongList(*this);
}

// #i102062#
bool WrongList::operator==(const WrongList& rCompare) const
{
    // check direct members
    if(GetInvalidStart() != rCompare.GetInvalidStart()
        || GetInvalidEnd() != rCompare.GetInvalidEnd()
        || maRanges.size() != rCompare.maRanges.size())
        return false;

    WrongList::const_iterator rCA = maRanges.begin();
    WrongList::const_iterator rCB = rCompare.maRanges.begin();

    for (; rCA != maRanges.end(); ++rCA, ++rCB)
    {
        if(rCA->mnStart != rCB->mnStart || rCA->mnEnd != rCB->mnEnd)
            return false;
    }

    return true;
}

bool WrongList::empty() const
{
    return maRanges.empty();
}

void WrongList::push_back(const editeng::MisspellRange& rRange)
{
    maRanges.push_back(rRange);
}

editeng::MisspellRange& WrongList::back()
{
    return maRanges.back();
}

const editeng::MisspellRange& WrongList::back() const
{
    return maRanges.back();
}

WrongList::iterator WrongList::begin()
{
    return maRanges.begin();
}

WrongList::iterator WrongList::end()
{
    return maRanges.end();
}

WrongList::const_iterator WrongList::begin() const
{
    return maRanges.begin();
}

WrongList::const_iterator WrongList::end() const
{
    return maRanges.end();
}

bool WrongList::DbgIsBuggy() const
{
    // Check if the ranges overlap.
    bool bError = false;
    for (WrongList::const_iterator i = begin(); !bError && (i != end()); ++i)
    {
        for (WrongList::const_iterator j = i + 1; !bError && (j != end()); ++j)
        {
            // 1) Start before, End after the second Start
            if (i->mnStart <= j->mnStart && i->mnEnd >= j->mnStart)
                bError = true;
            // 2) Start after the second Start, but still before the second End
            else if (i->mnStart >= j->mnStart && i->mnStart <= j->mnEnd)
                bError = true;
        }
    }
    return bError;
}


EdtAutoCorrDoc::EdtAutoCorrDoc(
    EditEngine* pE, ContentNode* pN, sal_Int32 nCrsr, sal_Unicode cIns) :
    mpEditEngine(pE),
    pCurNode(pN),
    nCursor(nCrsr),
    bAllowUndoAction(cIns != 0),
    bUndoAction(false) {}

EdtAutoCorrDoc::~EdtAutoCorrDoc()
{
    if ( bUndoAction )
        mpEditEngine->UndoActionEnd( EDITUNDO_INSERT );
}

bool EdtAutoCorrDoc::Delete(sal_Int32 nStt, sal_Int32 nEnd)
{
    EditSelection aSel( EditPaM( pCurNode, nStt ), EditPaM( pCurNode, nEnd ) );
    mpEditEngine->DeleteSelection(aSel);
    SAL_WARN_IF(nCursor < nEnd, "editeng",
            "Cursor in the heart of the action?!");
    nCursor -= ( nEnd-nStt );
    bAllowUndoAction = false;
    return true;
}

bool EdtAutoCorrDoc::Insert(sal_Int32 nPos, const OUString& rTxt)
{
    EditSelection aSel = EditPaM( pCurNode, nPos );
    mpEditEngine->InsertText(aSel, rTxt);
    SAL_WARN_IF(nCursor < nPos, "editeng",
            "Cursor in the heart of the action?!");
    nCursor = nCursor + rTxt.getLength();

    if ( bAllowUndoAction && ( rTxt.getLength() == 1 ) )
        ImplStartUndoAction();
    bAllowUndoAction = false;

    return true;
}

bool EdtAutoCorrDoc::Replace(sal_Int32 nPos, const OUString& rTxt)
{
    return ReplaceRange( nPos, rTxt.getLength(), rTxt );
}

bool EdtAutoCorrDoc::ReplaceRange(sal_Int32 nPos, sal_Int32 nSourceLength, const OUString& rTxt)
{
    // Actually a Replace introduce => corresponds to UNDO
    sal_Int32 nEnd = nPos+nSourceLength;
    if ( nEnd > pCurNode->Len() )
        nEnd = pCurNode->Len();

    // #i5925# First insert new text behind to be deleted text, for keeping attributes.
    mpEditEngine->InsertText(EditSelection(EditPaM(pCurNode, nEnd)), rTxt);
    mpEditEngine->DeleteSelection(
        EditSelection(EditPaM(pCurNode, nPos), EditPaM(pCurNode, nEnd)));

    if ( nPos == nCursor )
        nCursor = nCursor + rTxt.getLength();

    if ( bAllowUndoAction && ( rTxt.getLength() == 1 ) )
        ImplStartUndoAction();

    bAllowUndoAction = false;

    return true;
}

void EdtAutoCorrDoc::SetAttr(sal_Int32 nStt, sal_Int32 nEnd,
            sal_uInt16 nSlotId, SfxPoolItem& rItem)
{
    SfxItemPool* pPool = &mpEditEngine->GetEditDoc().GetItemPool();
    while ( pPool->GetSecondaryPool() &&
            pPool->GetName() != "EditEngineItemPool" )
    {
        pPool = pPool->GetSecondaryPool();

    }
    sal_uInt16 nWhich = pPool->GetWhich( nSlotId );
    if ( nWhich )
    {
        rItem.SetWhich( nWhich );

        SfxItemSet aSet = mpEditEngine->GetEmptyItemSet();
        aSet.Put( rItem );

        EditSelection aSel( EditPaM( pCurNode, nStt ), EditPaM( pCurNode, nEnd ) );
        aSel.Max().SetIndex( nEnd );    // ???
        mpEditEngine->SetAttribs( aSel, aSet, ATTRSPECIAL_EDGE );
        bAllowUndoAction = false;
    }
}

bool EdtAutoCorrDoc::SetINetAttr(sal_Int32 nStt, sal_Int32 nEnd,
            const OUString& rURL)
{
    // Turn the Text into a command field ...
    EditSelection aSel( EditPaM( pCurNode, nStt ), EditPaM( pCurNode, nEnd ) );
    OUString aText = mpEditEngine->GetSelected(aSel);
    aSel = mpEditEngine->DeleteSelection(aSel);
    SAL_WARN_IF(nCursor < nEnd, "editeng",
            "Cursor in the heart of the action?!");
    nCursor -= ( nEnd-nStt );
    SvxFieldItem aField( SvxURLField( rURL, aText, SVXURLFORMAT_REPR ),
                                      EE_FEATURE_FIELD  );
    mpEditEngine->InsertField(aSel, aField);
    nCursor++;
    mpEditEngine->UpdateFieldsOnly();
    bAllowUndoAction = false;
    return true;
}

OUString const* EdtAutoCorrDoc::GetPrevPara(bool const)
{
    // Return previous paragraph, so that it can be determined,
    // whether the current word is at the beginning of a sentence.

    bAllowUndoAction = false;   // Not anymore ...

    EditDoc& rNodes = mpEditEngine->GetEditDoc();
    sal_Int32 nPos = rNodes.GetPos( pCurNode );

    // Special case: Bullet => Paragraph start => simply return NULL...
    const SfxBoolItem& rBulletState = static_cast<const SfxBoolItem&>(
            mpEditEngine->GetParaAttrib( nPos, EE_PARA_BULLETSTATE ));
    bool bBullet = rBulletState.GetValue();
    if ( !bBullet && (mpEditEngine->GetControlWord() & EEControlBits::OUTLINER) )
    {
        // The Outliner has still a Bullet at Level 0.
        const SfxInt16Item& rLevel = static_cast<const SfxInt16Item&>(
                mpEditEngine->GetParaAttrib( nPos, EE_PARA_OUTLLEVEL ));
        if ( rLevel.GetValue() == 0 )
            bBullet = true;
    }
    if ( bBullet )
        return nullptr;

    for ( sal_Int32 n = nPos; n; )
    {
        n--;
        ContentNode* pNode = rNodes[n];
        if ( pNode->Len() )
            return & pNode->GetString();
    }
    return nullptr;

}

bool EdtAutoCorrDoc::ChgAutoCorrWord( sal_Int32& rSttPos,
            sal_Int32 nEndPos, SvxAutoCorrect& rACorrect,
            OUString* pPara )
{
    // Paragraph-start or a blank found, search for the word
    // shortcut in Auto
    bAllowUndoAction = false;   // Not anymore ...

    OUString aShort( pCurNode->Copy( rSttPos, nEndPos - rSttPos ) );
    bool bRet = false;

    if( aShort.isEmpty() )
        return bRet;

    LanguageTag aLanguageTag( mpEditEngine->GetLanguage( EditPaM( pCurNode, rSttPos+1 ) ));
    const SvxAutocorrWord* pFnd = rACorrect.SearchWordsInList(
            pCurNode->GetString(), rSttPos, nEndPos, *this, aLanguageTag);
    if( pFnd && pFnd->IsTextOnly() )
    {

        // replace also last colon of keywords surrounded by colons (for example, ":name:")
        bool replaceLastChar = pFnd->GetShort()[0] == ':' && pFnd->GetShort().endsWith(":");

        // then replace
        EditSelection aSel( EditPaM( pCurNode, rSttPos ),
                            EditPaM( pCurNode, nEndPos + (replaceLastChar ? 1 : 0) ));
        aSel = mpEditEngine->DeleteSelection(aSel);
        SAL_WARN_IF(nCursor < nEndPos, "editeng",
                "Cursor in the heart of the action?!");
        nCursor -= ( nEndPos-rSttPos );
        mpEditEngine->InsertText(aSel, pFnd->GetLong());
        nCursor = nCursor + pFnd->GetLong().getLength();
        if( pPara )
            *pPara = pCurNode->GetString();
        bRet = true;
    }

    return bRet;
}

LanguageType EdtAutoCorrDoc::GetLanguage( sal_Int32 nPos, bool ) const
{
    return mpEditEngine->GetLanguage( EditPaM( pCurNode, nPos+1 ) );
}

void EdtAutoCorrDoc::ImplStartUndoAction()
{
    sal_Int32 nPara = mpEditEngine->GetEditDoc().GetPos( pCurNode );
    ESelection aSel( nPara, nCursor, nPara, nCursor );
    mpEditEngine->UndoActionStart( EDITUNDO_INSERT, aSel );
    bUndoAction = true;
    bAllowUndoAction = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
