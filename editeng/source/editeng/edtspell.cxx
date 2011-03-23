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
#include "precompiled_editeng.hxx"

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

using ::rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::linguistic2;


EditSpellWrapper::EditSpellWrapper( Window* _pWin,
        Reference< XSpellChecker1 >  &xChecker,
        sal_Bool bIsStart, sal_Bool bIsAllRight, EditView* pView ) :
    SvxSpellWrapper( _pWin, xChecker, bIsStart, bIsAllRight )
{
    DBG_ASSERT( pView, "One view has to be abandoned!" );
    // Keep IgnoreList, delete ReplaceList...
    if (SvxGetChangeAllList().is())
        SvxGetChangeAllList()->clear();
    pEditView = pView;
}

void EditSpellWrapper::SpellStart( SvxSpellArea eArea )
{
    ImpEditEngine* pImpEE = pEditView->GetImpEditEngine();
    SpellInfo* pSpellInfo = pImpEE->GetSpellInfo();

    if ( eArea == SVX_SPELL_BODY_START )
    {
        // Is called when
        // a) Spell-Forward has arrived at the end and should restart at the top
        // IsEndDone() returns also sal_True, when backward-spelling is started at the end!
        if ( IsEndDone() )
        {
            pSpellInfo->bSpellToEnd = sal_False;
            pSpellInfo->aSpellTo = pSpellInfo->aSpellStart;
            pEditView->GetImpEditView()->SetEditSelection(
                    pImpEE->GetEditDoc().GetStartPaM() );
        }
        else
        {
            pSpellInfo->bSpellToEnd = sal_True;
            pSpellInfo->aSpellTo = pImpEE->CreateEPaM(
                    pImpEE->GetEditDoc().GetStartPaM() );
        }
    }
    else if ( eArea == SVX_SPELL_BODY_END )
    {
        // Is called when
        // a) Spell-Forward is launched
        // IsStartDone() return also sal_True, when forward-spelling is started at the beginning!
        if ( !IsStartDone() )
        {
            pSpellInfo->bSpellToEnd = sal_True;
            pSpellInfo->aSpellTo = pImpEE->CreateEPaM(
                    pImpEE->GetEditDoc().GetEndPaM() );
        }
        else
        {
            pSpellInfo->bSpellToEnd = sal_False;
            pSpellInfo->aSpellTo = pSpellInfo->aSpellStart;
            pEditView->GetImpEditView()->SetEditSelection(
                    pImpEE->GetEditDoc().GetEndPaM() );
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

sal_Bool EditSpellWrapper::SpellContinue()
{
    SetLast( pEditView->GetImpEditEngine()->ImpSpell( pEditView ) );
    return GetLast().is();
}

void EditSpellWrapper::SpellEnd()
{
    // Base class will show language errors...
    SvxSpellWrapper::SpellEnd();
}

sal_Bool EditSpellWrapper::HasOtherCnt()
{
    return sal_False;
}

sal_Bool EditSpellWrapper::SpellMore()
{
    ImpEditEngine* pImpEE = pEditView->GetImpEditEngine();
    SpellInfo* pSpellInfo = pImpEE->GetSpellInfo();
    sal_Bool bMore = sal_False;
    if ( pSpellInfo->bMultipleDoc )
    {
        bMore = pImpEE->GetEditEnginePtr()->SpellNextDocument();
        if ( bMore )
        {
            // The text has been entered into the engine, when backwords then
            // it must be behind the selection.
            Reference< XPropertySet >  xProp( SvxGetLinguPropertySet() );
            pEditView->GetImpEditView()->SetEditSelection(
                        pImpEE->GetEditDoc().GetStartPaM() );
        }
    }
    return bMore;
}

void EditSpellWrapper::ScrollArea()
{
    // No further action needed ...
    // Except for, that the area is to be scrolled in the center, and not stand
    // still anywhere.
}

void EditSpellWrapper::ReplaceAll( const String &rNewText,
            sal_Int16 )
{
    // Is called when the word is in ReplaceList of the spell checker
    pEditView->InsertText( rNewText );
    CheckSpellTo();
}

void EditSpellWrapper::ChangeWord( const String& rNewWord,
            const sal_uInt16 )
{
    // Will be called when Word Button Change
    // or internally by me ChangeAll

    // If there is a dot Punkt after the word, this dot will be stripped away.
    // If '"' => PreStripped.
    String aNewWord( rNewWord );
    pEditView->InsertText( aNewWord );
    CheckSpellTo();
}

void EditSpellWrapper::ChangeThesWord( const String& rNewWord )
{
    pEditView->InsertText( rNewWord );
    CheckSpellTo();
}

void EditSpellWrapper::AutoCorrect( const String&, const String& )
{
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

//////////////////////////////////////////////////////////////////////

SV_IMPL_VARARR( WrongRanges, WrongRange );

WrongList::WrongList()
{
    nInvalidStart = 0;
    nInvalidEnd = 0xFFFF;
}

WrongList::~WrongList()
{
}

void WrongList::MarkInvalid( sal_uInt16 nS, sal_uInt16 nE )
{
    if ( ( nInvalidStart == NOT_INVALID ) || ( nInvalidStart > nS ) )
        nInvalidStart = nS;
    if ( nInvalidEnd < nE )
        nInvalidEnd = nE;
}

void WrongList::TextInserted( sal_uInt16 nPos, sal_uInt16 nNew, sal_Bool bPosIsSep )
{
    if ( !IsInvalid() )
    {
        nInvalidStart = nPos;
        nInvalidEnd = nPos+nNew;
    }
    else
    {
        if ( nInvalidStart > nPos )
            nInvalidStart = nPos;
        if ( nInvalidEnd >= nPos )
            nInvalidEnd = nInvalidEnd + nNew;
        else
            nInvalidEnd = nPos+nNew;
    }

    for ( sal_uInt16 n = 0; n < Count(); n++ )
    {
        WrongRange& rWrong = GetObject( n );
        sal_Bool bRefIsValid = sal_True;
        if ( rWrong.nEnd >= nPos )
        {
            // Move all Wrongs after the insert position...
            if ( rWrong.nStart > nPos )
            {
                rWrong.nStart = rWrong.nStart + nNew;
                rWrong.nEnd = rWrong.nEnd + nNew;
            }
            // 1: Starts before and goes until nPos...
            else if ( rWrong.nEnd == nPos )
            {
                // Should be halted at a blank!
                if ( !bPosIsSep )
                    rWrong.nEnd = rWrong.nEnd + nNew;
            }
            // 2: Starts before and goes until after nPos...
            else if ( ( rWrong.nStart < nPos ) && ( rWrong.nEnd > nPos ) )
            {
                rWrong.nEnd = rWrong.nEnd + nNew;
                // When a separator remove and re-examine the Wrong
                if ( bPosIsSep )
                {
                    // Split Wrong...
                    WrongRange aNewWrong( rWrong.nStart, nPos );
                    rWrong.nStart = nPos+1;
                    Insert( aNewWrong, n );
                    bRefIsValid = sal_False;    // Reference no longer valid after Insert, the other was inserted in front of this position
                    n++; // This not again ...
                }
            }
            // 3: Attribute starts at position ..
            else if ( rWrong.nStart == nPos )
            {
                rWrong.nEnd = rWrong.nEnd + nNew;
                if ( bPosIsSep )
                    rWrong.nStart++;
            }
        }
        DBG_ASSERT( !bRefIsValid || ( rWrong.nStart < rWrong.nEnd ),
                "TextInserted, WrongRange: Start >= End?!" );
        (void)bRefIsValid;
    }

    DBG_ASSERT( !DbgIsBuggy(), "InsertWrong: WrongList broken!" );
}

void WrongList::TextDeleted( sal_uInt16 nPos, sal_uInt16 nDeleted )
{
    sal_uInt16 nEndChanges = nPos+nDeleted;
    if ( !IsInvalid() )
    {
        sal_uInt16 nNewInvalidStart = nPos ? nPos - 1 : 0;
        nInvalidStart = nNewInvalidStart;
        nInvalidEnd = nNewInvalidStart + 1;
    }
    else
    {
        if ( nInvalidStart > nPos )
            nInvalidStart = nPos;
        if ( nInvalidEnd > nPos )
        {
            if ( nInvalidEnd > nEndChanges )
                nInvalidEnd = nInvalidEnd - nDeleted;
            else
                nInvalidEnd = nPos+1;
        }
    }

    for ( sal_uInt16 n = 0; n < Count(); n++ )
    {
        WrongRange& rWrong = GetObject( n );
        sal_Bool bDelWrong = sal_False;
        if ( rWrong.nEnd >= nPos )
        {
            // Move all Wrongs after the insert position...
            if ( rWrong.nStart >= nEndChanges )
            {
                rWrong.nStart = rWrong.nStart - nDeleted;
                rWrong.nEnd = rWrong.nEnd - nDeleted;
            }
            // 1. Delete Internal Wrongs ...
            else if ( ( rWrong.nStart >= nPos ) && ( rWrong.nEnd <= nEndChanges ) )
            {
                bDelWrong = sal_True;
            }
            // 2. Wrong begins before, ends inside or behind it ...
            else if ( ( rWrong.nStart <= nPos ) && ( rWrong.nEnd > nPos ) )
            {
                if ( rWrong.nEnd <= nEndChanges )   // ends inside
                    rWrong.nEnd = nPos;
                else
                    rWrong.nEnd = rWrong.nEnd - nDeleted; // ends after
            }
            // 3. Wrong begins inside, ending after ...
            else if ( ( rWrong.nStart >= nPos ) && ( rWrong.nEnd > nEndChanges ) )
            {
                rWrong.nStart = nEndChanges;
                rWrong.nStart = rWrong.nStart - nDeleted;
                rWrong.nEnd = rWrong.nEnd - nDeleted;
            }
        }
        DBG_ASSERT( rWrong.nStart < rWrong.nEnd,
                "TextInserted, WrongRange: Start >= End?!" );
        if ( bDelWrong )
        {
            Remove( n, 1 );
            n--;
        }
    }

    DBG_ASSERT( !DbgIsBuggy(), "InsertWrong: WrongList broken!" );
}

sal_Bool WrongList::NextWrong( sal_uInt16& rnStart, sal_uInt16& rnEnd ) const
{
    /*
        rnStart get the start position, is possibly adjusted wrt. Wrong start
        rnEnd does not have to be initialized.
    */
    for ( sal_uInt16 n = 0; n < Count(); n++ )
    {
        WrongRange& rWrong = GetObject( n );
        if ( rWrong.nEnd > rnStart )
        {
            rnStart = rWrong.nStart;
            rnEnd = rWrong.nEnd;
            return sal_True;
        }
    }
    return sal_False;
}

sal_Bool WrongList::HasWrong( sal_uInt16 nStart, sal_uInt16 nEnd ) const
{
    for ( sal_uInt16 n = 0; n < Count(); n++ )
    {
        WrongRange& rWrong = GetObject( n );
        if ( ( rWrong.nStart == nStart ) && ( rWrong.nEnd == nEnd ) )
            return sal_True;
        else if ( rWrong.nStart >= nStart )
            break;
    }
    return sal_False;
}

sal_Bool WrongList::HasAnyWrong( sal_uInt16 nStart, sal_uInt16 nEnd ) const
{
    for ( sal_uInt16 n = 0; n < Count(); n++ )
    {
        WrongRange& rWrong = GetObject( n );
        if ( ( rWrong.nEnd >= nStart ) && ( rWrong.nStart < nEnd ) )
            return sal_True;
        else if ( rWrong.nStart >= nEnd )
            break;
    }
    return sal_False;
}

void WrongList::ClearWrongs( sal_uInt16 nStart, sal_uInt16 nEnd,
            const ContentNode* pNode )
{
    for ( sal_uInt16 n = 0; n < Count(); n++ )
    {
        WrongRange& rWrong = GetObject( n );
        if ( ( rWrong.nEnd > nStart ) && ( rWrong.nStart < nEnd ) )
        {
            if ( rWrong.nEnd > nEnd ) // Runs out
            {
                rWrong.nStart = nEnd;
                // Blanks?
                while ( ( rWrong.nStart < pNode->Len() ) &&
                            ( ( pNode->GetChar( rWrong.nStart ) == ' ' ) ||
                              ( pNode->IsFeature( rWrong.nStart ) ) ) )
                {
                    rWrong.nStart++;
                }
            }
            else
            {
                Remove( n, 1 );
                n--;
            }
        }
    }

    DBG_ASSERT( !DbgIsBuggy(), "InsertWrong: WrongList broken!" );
}

void WrongList::InsertWrong( sal_uInt16 nStart, sal_uInt16 nEnd,
            sal_Bool bClearRange )
{
    sal_uInt16 nPos = Count();
    for ( sal_uInt16 n = 0; n < Count(); n++ )
    {
        WrongRange& rWrong = GetObject( n );
        if ( rWrong.nStart >= nStart )
        {
            nPos = n;
            if ( bClearRange )
            {
                // It can really only happen that the Wrong starts exactly here
                // and runs along, but not that there are several ranges ...
                // Exactly in the range is no one allowed to be, otherwise this
                // Method can not be called!
                DBG_ASSERT( ( ( rWrong.nStart == nStart ) && ( rWrong.nEnd > nEnd ) )
                                || ( rWrong.nStart > nEnd ), "InsertWrong: RangeMismatch!" );
                if ( ( rWrong.nStart == nStart ) && ( rWrong.nEnd > nEnd ) )
                    rWrong.nStart = nEnd+1;
            }
            break;
        }
    }
    Insert( WrongRange( nStart, nEnd ), nPos );

    DBG_ASSERT( !DbgIsBuggy(), "InsertWrong: WrongList broken!" );
}

void WrongList::MarkWrongsInvalid()
{
    if ( Count() )
        MarkInvalid( GetObject( 0 ).nStart, GetObject( Count()-1 ).nEnd );
}

WrongList*  WrongList::Clone() const
{
    WrongList* pNew = new WrongList;
    for ( sal_uInt16 n = 0; n < Count(); n++ )
    {
        WrongRange& rWrong = GetObject( n );
        pNew->Insert( rWrong, pNew->Count() );
    }

    return pNew;
}

// #i102062#
bool WrongList::operator==(const WrongList& rCompare) const
{
    // cleck direct members
    if(GetInvalidStart() != rCompare.GetInvalidStart()
        || GetInvalidEnd() != rCompare.GetInvalidEnd()
        || Count() != rCompare.Count())
    {
        return false;
    }

    for(sal_uInt16 a(0); a < Count(); a++)
    {
        const WrongRange& rCandA(GetObject(a));
        const WrongRange& rCandB(rCompare.GetObject(a));

        if(rCandA.nStart != rCandB.nStart
            || rCandA.nEnd != rCandB.nEnd)
        {
            return false;
        }
    }

    return true;
}

#ifdef DBG_UTIL
sal_Bool WrongList::DbgIsBuggy() const
{
    // Check if the ranges overlap.
    sal_Bool bError = sal_False;
    for ( sal_uInt16 _nA = 0; !bError && ( _nA < Count() ); _nA++ )
    {
        WrongRange& rWrong = GetObject( _nA );
        for ( sal_uInt16 nB = _nA+1; !bError && ( nB < Count() ); nB++ )
        {
            WrongRange& rNextWrong = GetObject( nB );
            // 1) Start before, End after the second Start
            if (   ( rWrong.nStart <= rNextWrong.nStart )
                && ( rWrong.nEnd >= rNextWrong.nStart ) )
                bError = sal_True;
            // 2) Start after the second Start, but still before the second End
            else if (   ( rWrong.nStart >= rNextWrong.nStart)
                     && ( rWrong.nStart <= rNextWrong.nEnd ) )
                bError = sal_True;
        }
    }
    return bError;
}
#endif

//////////////////////////////////////////////////////////////////////

EdtAutoCorrDoc::EdtAutoCorrDoc( ImpEditEngine* pE, ContentNode* pN,
            sal_uInt16 nCrsr, xub_Unicode cIns )
{
    pImpEE = pE;
    pCurNode = pN;
    nCursor = nCrsr;

    bUndoAction = sal_False;
    bAllowUndoAction = cIns ? sal_True : sal_False;
}

EdtAutoCorrDoc::~EdtAutoCorrDoc()
{
    if ( bUndoAction )
        pImpEE->UndoActionEnd( EDITUNDO_INSERT );
}

sal_Bool EdtAutoCorrDoc::Delete( sal_uInt16 nStt, sal_uInt16 nEnd )
{
    EditSelection aSel( EditPaM( pCurNode, nStt ), EditPaM( pCurNode, nEnd ) );
    pImpEE->ImpDeleteSelection( aSel );
    DBG_ASSERT( nCursor >= nEnd, "Cursor in the heart of the action?!" );
    nCursor -= ( nEnd-nStt );
    bAllowUndoAction = sal_False;
    return sal_True;
}

sal_Bool EdtAutoCorrDoc::Insert( sal_uInt16 nPos, const String& rTxt )
{
    EditSelection aSel = EditPaM( pCurNode, nPos );
    pImpEE->ImpInsertText( aSel, rTxt );
    DBG_ASSERT( nCursor >= nPos, "Cursor in the heart of the action?!" );
    nCursor = nCursor + rTxt.Len();

    if ( bAllowUndoAction && ( rTxt.Len() == 1 ) )
        ImplStartUndoAction();
    bAllowUndoAction = sal_False;

    return sal_True;
}

sal_Bool EdtAutoCorrDoc::Replace( sal_uInt16 nPos, const String& rTxt )
{
    return ReplaceRange( nPos, rTxt.Len(), rTxt );
}

sal_Bool EdtAutoCorrDoc::ReplaceRange( xub_StrLen nPos, xub_StrLen nSourceLength, const String& rTxt )
{
    // Actually a Replace introduce => corresponds to UNDO
    sal_uInt16 nEnd = nPos+nSourceLength;
    if ( nEnd > pCurNode->Len() )
        nEnd = pCurNode->Len();

    // #i5925# First insert new text behind to be deleted text, for keeping attributes.
    pImpEE->ImpInsertText( EditSelection( EditPaM( pCurNode, nEnd ) ), rTxt );
    pImpEE->ImpDeleteSelection( EditSelection( EditPaM( pCurNode, nPos ), EditPaM( pCurNode, nEnd ) ) );

    if ( nPos == nCursor )
        nCursor = nCursor + rTxt.Len();

    if ( bAllowUndoAction && ( rTxt.Len() == 1 ) )
        ImplStartUndoAction();

    bAllowUndoAction = sal_False;

    return sal_True;
}

sal_Bool EdtAutoCorrDoc::SetAttr( sal_uInt16 nStt, sal_uInt16 nEnd,
            sal_uInt16 nSlotId, SfxPoolItem& rItem )
{
    SfxItemPool* pPool = &pImpEE->GetEditDoc().GetItemPool();
    while ( pPool->GetSecondaryPool() &&
            !pPool->GetName().EqualsAscii( "EditEngineItemPool" ) )
    {
        pPool = pPool->GetSecondaryPool();

    }
    sal_uInt16 nWhich = pPool->GetWhich( nSlotId );
    if ( nWhich )
    {
        rItem.SetWhich( nWhich );

        SfxItemSet aSet( pImpEE->GetEmptyItemSet() );
        aSet.Put( rItem );

        EditSelection aSel( EditPaM( pCurNode, nStt ), EditPaM( pCurNode, nEnd ) );
        aSel.Max().SetIndex( nEnd );    // ???
        pImpEE->SetAttribs( aSel, aSet, ATTRSPECIAL_EDGE );
        bAllowUndoAction = sal_False;
    }
    return sal_True;
}

sal_Bool EdtAutoCorrDoc::SetINetAttr( sal_uInt16 nStt, sal_uInt16 nEnd,
            const String& rURL )
{
    // Turn the Text into a command field ...
    EditSelection aSel( EditPaM( pCurNode, nStt ), EditPaM( pCurNode, nEnd ) );
    String aText = pImpEE->GetSelected( aSel );
    aSel = pImpEE->ImpDeleteSelection( aSel );
    DBG_ASSERT( nCursor >= nEnd, "Cursor in the heart of the action ?!" );
    nCursor -= ( nEnd-nStt );
    SvxFieldItem aField( SvxURLField( rURL, aText, SVXURLFORMAT_REPR ),
                                      EE_FEATURE_FIELD  );
    pImpEE->InsertField( aSel, aField );
    nCursor++;
    pImpEE->UpdateFields();
    bAllowUndoAction = sal_False;
    return sal_True;
}

sal_Bool EdtAutoCorrDoc::HasSymbolChars( sal_uInt16 nStt, sal_uInt16 nEnd )
{
    sal_uInt16 nScriptType = pImpEE->GetScriptType( EditPaM( pCurNode, nStt ) );
    sal_uInt16 nScriptFontInfoItemId = GetScriptItemId( EE_CHAR_FONTINFO, nScriptType );

    CharAttribArray& rAttribs = pCurNode->GetCharAttribs().GetAttribs();
    sal_uInt16 nAttrs = rAttribs.Count();
    for ( sal_uInt16 n = 0; n < nAttrs; n++ )
    {
        EditCharAttrib* pAttr = rAttribs.GetObject( n );
        if ( pAttr->GetStart() >= nEnd )
            return sal_False;

        if ( ( pAttr->Which() == nScriptFontInfoItemId ) &&
                ( ((SvxFontItem*)pAttr->GetItem())->GetCharSet() == RTL_TEXTENCODING_SYMBOL ) )
        {
            // check if the Attribtuteis within range...
            if ( pAttr->GetEnd() >= nStt )
                return sal_True;
        }
    }
    return sal_False;
}

const String* EdtAutoCorrDoc::GetPrevPara( sal_Bool )
{
    // Return previous paragraph, so that it can be determined,
    // whether the current word is at the beginning of a sentence.

    bAllowUndoAction = sal_False;   // Not anymore ...

    ContentList& rNodes = pImpEE->GetEditDoc();
    sal_uInt16 nPos = rNodes.GetPos( pCurNode );

    // Special case: Bullet => Paragraph start => simply return NULL...
    const SfxBoolItem& rBulletState = (const SfxBoolItem&)
            pImpEE->GetParaAttrib( nPos, EE_PARA_BULLETSTATE );
    sal_Bool bBullet = rBulletState.GetValue() ? sal_True : sal_False;
    if ( !bBullet && ( pImpEE->aStatus.GetControlWord() & EE_CNTRL_OUTLINER ) )
    {
        // The Outliner has still a Bullet at Level 0.
        const SfxInt16Item& rLevel = (const SfxInt16Item&)
                pImpEE->GetParaAttrib( nPos, EE_PARA_OUTLLEVEL );
        if ( rLevel.GetValue() == 0 )
            bBullet = sal_True;
    }
    if ( bBullet )
        return NULL;

    for ( sal_uInt16 n = nPos; n; )
    {
        n--;
        ContentNode* pNode = rNodes[n];
        if ( pNode->Len() )
            return pNode;
    }
    return NULL;

}

sal_Bool EdtAutoCorrDoc::ChgAutoCorrWord( sal_uInt16& rSttPos,
            sal_uInt16 nEndPos, SvxAutoCorrect& rACorrect,
            const String** ppPara )
{
    // Paragraph-start or a blank found, search for the word
    // shortcut in Auto
    bAllowUndoAction = sal_False;   // Not anymore ...

    String aShort( pCurNode->Copy( rSttPos, nEndPos - rSttPos ) );
    sal_Bool bRet = sal_False;

    if( !aShort.Len() )
        return bRet;

    LanguageType eLang = pImpEE->GetLanguage( EditPaM( pCurNode, rSttPos+1 ) );
    const SvxAutocorrWord* pFnd = rACorrect.SearchWordsInList( *pCurNode, rSttPos, nEndPos, *this, eLang );
    if( pFnd && pFnd->IsTextOnly() )
    {
        // then replace
        EditSelection aSel( EditPaM( pCurNode, rSttPos ),
                            EditPaM( pCurNode, nEndPos ) );
        aSel = pImpEE->ImpDeleteSelection( aSel );
        DBG_ASSERT( nCursor >= nEndPos, "Cursor in the heart of the action?!" );
        nCursor -= ( nEndPos-rSttPos );
        pImpEE->ImpInsertText( aSel, pFnd->GetLong() );
        nCursor = nCursor + pFnd->GetLong().Len();
        if( ppPara )
            *ppPara = pCurNode;
        bRet = sal_True;
    }

    return bRet;
}

LanguageType EdtAutoCorrDoc::GetLanguage( sal_uInt16 nPos, sal_Bool ) const
{
    return pImpEE->GetLanguage( EditPaM( pCurNode, nPos+1 ) );
}

void EdtAutoCorrDoc::ImplStartUndoAction()
{
    sal_uInt16 nPara = pImpEE->GetEditDoc().GetPos( pCurNode );
    ESelection aSel( nPara, nCursor, nPara, nCursor );
    pImpEE->UndoActionStart( EDITUNDO_INSERT, aSel );
    bUndoAction = sal_True;
    bAllowUndoAction = sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
