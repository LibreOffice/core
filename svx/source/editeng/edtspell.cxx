/*************************************************************************
 *
 *  $RCSfile: edtspell.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:14 $
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

#include <eeng_pch.hxx>

#pragma hdrstop

#include <impedit.hxx>
#include <editview.hxx>
#include <editeng.hxx>
#include <edtspell.hxx>
#include <flditem.hxx>
#include <fontitem.hxx>
#include <svtools/intitem.hxx>

#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _LINGU_LNGPROPS_HHX_
#include <lingu/lngprops.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::linguistic;


EditSpellWrapper::EditSpellWrapper( Window* pWin,
        Reference< XSpellChecker1 >  &xChecker,
        sal_Bool bIsStart, sal_Bool bIsAllRight, EditView* pView ) :
    SvxSpellWrapper( pWin, xChecker, bIsStart, bIsAllRight )
{
    DBG_ASSERT( pView, "Es muss eine View uebergeben werden!" );
    // IgnoreList behalten, ReplaceList loeschen...
    if (SvxGetChangeAllList().is())
        SvxGetChangeAllList()->clear();
    pEditView = pView;
}

void __EXPORT EditSpellWrapper::SpellStart( SvxSpellArea eArea )
{
    ImpEditEngine* pImpEE = pEditView->GetImpEditEngine();
    SpellInfo* pSpellInfo = pImpEE->GetSpellInfo();
    sal_Bool bForward = !IsSpellReverse();

    if ( eArea == SVX_SPELL_BODY_START )
    {
        // Wird gerufen, wenn
        // a) Spell-Forwad ist am Ende angekomment und soll von vorne beginnen
        // b) Spell-Backward wird gestartet
        // IsEndDone() liefert auch sal_True, wenn Rueckwaerts-Spelling am Ende gestartet wird!
        if ( IsEndDone() && bForward )
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
        // Wird gerufen, wenn
        // a) Spell-Forwad wird gestartet
        // b) Spell-Backward ist am Anfang angekomment und soll von hinten beginnen
        // IsStartDone() liefert auch sal_True, wenn Vorwaerts-Spelling am Anfang gestartet wird!
        if ( !IsStartDone() || bForward )
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
        ;   // Wird ueber SpellNextDocument von App gehandelt

        // pSpellInfo->bSpellToEnd = sal_True;
        // pSpellInfo->aSpellTo = pImpEE->CreateEPaM( pImpEE->GetEditDoc().GetEndPaM() );
    }
    else
    {
        DBG_ERROR( "SpellStart: Unknown Area!" );
    }
}

sal_Bool EditSpellWrapper::SpellContinue()
{
    SetLast( pEditView->GetImpEditEngine()->ImpSpell( pEditView ) );
    return GetLast().is();
}

void __EXPORT EditSpellWrapper::SpellEnd()
{
    // Keine weitere Aktion noetig...
}

sal_Bool __EXPORT EditSpellWrapper::HasOtherCnt()
{
    return sal_False;
}

sal_Bool __EXPORT EditSpellWrapper::SpellMore()
{
    ImpEditEngine* pImpEE = pEditView->GetImpEditEngine();
    SpellInfo* pSpellInfo = pImpEE->GetSpellInfo();
    sal_Bool bMore = sal_False;
    if ( pSpellInfo->bMultipleDoc )
    {
        bMore = pImpEE->GetEditEnginePtr()->SpellNextDocument();
        if ( bMore )
        {
            // Der Text wurde in diese Engine getreten, bei Rueckwaerts
            // muss die Selektion hinten sein.
            Reference< XPropertySet >  xProp( SvxGetLinguPropertySet() );
            sal_Bool bIsWrapReverse = IsSpellReverse();
            if ( bIsWrapReverse )
                pEditView->GetImpEditView()->SetEditSelection(
                        pImpEE->GetEditDoc().GetEndPaM() );
            else
                pEditView->GetImpEditView()->SetEditSelection(
                        pImpEE->GetEditDoc().GetStartPaM() );
        }
    }
    return bMore;
}

void __EXPORT EditSpellWrapper::ScrollArea()
{
    // Keine weitere Aktion noetig...
    // Es sei denn, der Bereich soll in die Mitte gescrollt werden,
    // und nicht irgendwo stehen.
}

void __EXPORT EditSpellWrapper::ReplaceAll( const String &rNewText,
            sal_Int16 nLanguage )
{
    // Wird gerufen, wenn Wort in ReplaceList des SpellCheckers
    pEditView->InsertText( rNewText );
    CheckSpellTo();
}

void __EXPORT EditSpellWrapper::ChangeWord( const String& rNewWord,
            const sal_uInt16 nLang )
{
    // Wird gerufen, wenn Wort Button Change
    // bzw. intern von mir bei ChangeAll

    // Wenn Punkt hinterm Wort, wird dieser nicht mitgegeben.
    // Falls '"' => PreStripped.
    String aNewWord( rNewWord );
    pEditView->InsertText( aNewWord );
    CheckSpellTo();
}

void __EXPORT EditSpellWrapper::ChangeThesWord( const String& rNewWord )
{
    pEditView->InsertText( rNewWord );
    CheckSpellTo();
}

void __EXPORT EditSpellWrapper::AutoCorrect( const String& rOldWord,
            const String& rNewWord )
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
        // prueffen, ob SpellToEnd noch gueltiger Index, falls in dem Absatz
        // ersetzt wurde.
        if ( pSpellInfo->aSpellTo.nIndex > aPaM.GetNode()->Len() )
            pSpellInfo->aSpellTo.nIndex = aPaM.GetNode()->Len();
    }
}

BOOL EditSpellWrapper::IsSpellReverse()
{
    Reference< XPropertySet >  xProp( SvxGetLinguPropertySet() );
    sal_Bool bIsWrapReverse = sal_False;
    if ( xProp.is() )
    {
        Any aVal = xProp->getPropertyValue( OUString(
                RTL_CONSTASCII_USTRINGPARAM( UPN_IS_WRAP_REVERSE ) ) );
        aVal >>= bIsWrapReverse;
    }
    return bIsWrapReverse;
}

SV_IMPL_VARARR( WrongRanges, WrongRange );

WrongList::WrongList()
{
    nInvalidStart = 0;
    nInvalidEnd = 0xFFFF;
}

WrongList::~WrongList()
{
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
            nInvalidEnd += nNew;
        else
            nInvalidEnd = nPos+nNew;
    }

    for ( sal_uInt16 n = 0; n < Count(); n++ )
    {
        WrongRange& rWrong = GetObject( n );
        sal_Bool bRefIsValid = sal_True;
        if ( rWrong.nEnd >= nPos )
        {
            // Alle Wrongs hinter der Einfuegeposition verschieben...
            if ( rWrong.nStart > nPos )
            {
                rWrong.nStart += nNew;
                rWrong.nEnd += nNew;
            }
            // 1: Startet davor, geht bis nPos...
            else if ( rWrong.nEnd == nPos )
            {
                // Sollte bei einem Blank unterbunden werden!
                if ( !bPosIsSep )
                    rWrong.nEnd += nNew;
            }
            // 2: Startet davor, geht hinter Pos...
            else if ( ( rWrong.nStart < nPos ) && ( rWrong.nEnd > nPos ) )
            {
                rWrong.nEnd += nNew;
                // Bei einem Trenner das Wrong entfernen und neu pruefen
                if ( bPosIsSep )
                {
                    // Wrong aufteilen...
                    WrongRange aNewWrong( rWrong.nStart, nPos );
                    rWrong.nStart = nPos+1;
                    Insert( aNewWrong, n );
                    bRefIsValid = sal_False;    // Referenz nach Insert nicht mehr gueltig, der andere wurde davor an dessen Position eingefuegt
                    n++; // Diesen nicht nochmal...
                }
            }
            // 3: Attribut startet auf Pos...
            else if ( rWrong.nStart == nPos )
            {
                rWrong.nEnd += nNew;
                if ( bPosIsSep )
                    rWrong.nStart++;
            }
        }
        DBG_ASSERT( !bRefIsValid || ( rWrong.nStart < rWrong.nEnd ),
                "TextInserted, WrongRange: Start >= End?!" );
    }

    DBG_ASSERT( !DbgIsBuggy(), "InsertWrong: WrongList kaputt!" );
}

void WrongList::TextDeleted( sal_uInt16 nPos, sal_uInt16 nDeleted )
{
    sal_uInt16 nEndChanges = nPos+nDeleted;
    if ( !IsInvalid() )
    {
        nInvalidStart = nPos;
        nInvalidEnd = nPos+1;   // Nicht nDeleted, weil da ja wegfaellt.
    }
    else
    {
        if ( nInvalidStart > nPos )
            nInvalidStart = nPos;
        if ( nInvalidEnd > nPos )
        {
            if ( nInvalidEnd > nEndChanges )
                nInvalidEnd -=nDeleted;
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
            // Alles Wrongs hinter der Einfuegeposition verschieben...
            if ( rWrong.nStart >= nEndChanges )
            {
                rWrong.nStart -= nDeleted;
                rWrong.nEnd -= nDeleted;
            }
            // 1. Innenliegende Wrongs loeschen...
            else if ( ( rWrong.nStart >= nPos ) && ( rWrong.nEnd <= nEndChanges ) )
            {
                bDelWrong = sal_True;
            }
            // 2. Wrong beginnt davor, endet drinnen oder dahinter...
            else if ( ( rWrong.nStart <= nPos ) && ( rWrong.nEnd > nPos ) )
            {
                if ( rWrong.nEnd <= nEndChanges )   // endet drinnen
                    rWrong.nEnd = nPos;
                else
                    rWrong.nEnd -= nDeleted;        // endet dahinter
            }
            // 3. Wrong beginnt drinnen, endet dahinter...
            else if ( ( rWrong.nStart >= nPos ) && ( rWrong.nEnd > nEndChanges ) )
            {
                rWrong.nStart = nEndChanges;
                rWrong.nStart -= nDeleted;
                rWrong.nEnd -= nDeleted;
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

    DBG_ASSERT( !DbgIsBuggy(), "InsertWrong: WrongList kaputt!" );
}

sal_Bool WrongList::NextWrong( sal_uInt16& rnStart, sal_uInt16& rnEnd ) const
{
    /*
        rnStart enthaelt die Startposition, wird ggf. auf Wrong-Start korrigiert
        rnEnd braucht nicht inizialisiert sein.
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
            if ( rWrong.nEnd > nEnd )   // // Laeuft raus
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

    DBG_ASSERT( !DbgIsBuggy(), "InsertWrong: WrongList kaputt!" );
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
                // Es kann eigentlich nur Passieren, dass der Wrong genau
                // hier beginnt und weiter rauslauft, aber nicht, dass hier
                // mehrere im Bereich liegen...
                // Genau im Bereich darf keiner liegen, sonst darf diese Methode
                // garnicht erst gerufen werden!
                DBG_ASSERT( ( ( rWrong.nStart == nStart ) && ( rWrong.nEnd > nEnd ) )
                                || ( rWrong.nStart > nEnd ), "InsertWrong: RangeMismatch!" );
                if ( ( rWrong.nStart == nStart ) && ( rWrong.nEnd > nEnd ) )
                    rWrong.nStart = nEnd+1;
            }
            break;
        }
    }
    Insert( WrongRange( nStart, nEnd ), nPos );

    DBG_ASSERT( !DbgIsBuggy(), "InsertWrong: WrongList kaputt!" );
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


#ifdef DBG_UTIL
sal_Bool WrongList::DbgIsBuggy() const
{
    // Pruefen, ob sich Bereiche ueberlappen
    sal_Bool bError = sal_False;
    for ( sal_uInt16 nA = 0; !bError && ( nA < Count() ); nA++ )
    {
        WrongRange& rWrong = GetObject( nA );
        for ( sal_uInt16 nB = nA+1; !bError && ( nB < Count() ); nB++ )
        {
            WrongRange& rNextWrong = GetObject( nB );
            // 1) Start davor, End hinterm anderen Start
            if (   ( rWrong.nStart <= rNextWrong.nStart )
                && ( rWrong.nEnd >= rNextWrong.nStart ) )
                bError = sal_True;
            // 2) Start hinter anderen Start, aber noch vorm anderen End
            else if (   ( rWrong.nStart >= rNextWrong.nStart)
                     && ( rWrong.nStart <= rNextWrong.nEnd ) )
                bError = sal_True;
        }
    }
    return bError;
}
#endif


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
    DBG_ASSERT( nCursor >= nEnd, "Cursor mitten im Geschehen ?!" );
    nCursor -= ( nEnd-nStt );
    bAllowUndoAction = sal_False;
    return sal_True;
}

sal_Bool EdtAutoCorrDoc::Insert( sal_uInt16 nPos, const String& rTxt )
{
    EditSelection aSel = EditPaM( pCurNode, nPos );
    pImpEE->ImpInsertText( aSel, rTxt );
    DBG_ASSERT( nCursor >= nPos, "Cursor mitten im Geschehen ?!" );
    nCursor += rTxt.Len();

    if ( bAllowUndoAction && ( rTxt.Len() == 1 ) )
        ImplStartUndoAction();
    bAllowUndoAction = sal_False;

    return sal_True;
}

sal_Bool EdtAutoCorrDoc::Replace( sal_uInt16 nPos, const String& rTxt )
{
    // Eigentlich ein Replace einfuehren => Entspr. UNDO
    sal_uInt16 nEnd = nPos+rTxt.Len();
    if ( nEnd > pCurNode->Len() )
        nEnd = pCurNode->Len();
    EditSelection aSel( EditPaM( pCurNode, nPos ), EditPaM( pCurNode, nEnd ) );
    aSel = pImpEE->ImpDeleteSelection( aSel );
    aSel = pImpEE->ImpInsertText( aSel, rTxt );
    if ( nPos == nCursor )
        nCursor += rTxt.Len();

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
            pPool->GetName().EqualsAscii( "EditEngineItemPool" ) )
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
    // Aus dem Text ein Feldbefehl machen...
    EditSelection aSel( EditPaM( pCurNode, nStt ), EditPaM( pCurNode, nEnd ) );
    String aText = pImpEE->GetSelected( aSel );
    aSel = pImpEE->ImpDeleteSelection( aSel );
    DBG_ASSERT( nCursor >= nEnd, "Cursor mitten im Geschehen ?!" );
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
    CharAttribArray& rAttribs = pCurNode->GetCharAttribs().GetAttribs();
    sal_uInt16 nAttrs = rAttribs.Count();
    for ( sal_uInt16 n = 0; n < nAttrs; n++ )
    {
        EditCharAttrib* pAttr = rAttribs.GetObject( n );
        if ( pAttr->GetStart() >= nEnd )
            return sal_False;

        if ( ( pAttr->Which() == EE_CHAR_FONTINFO ) &&
                ( ((SvxFontItem*)pAttr->GetItem())->
                        GetCharSet() == RTL_TEXTENCODING_SYMBOL ) )
        {
            // Pruefen, ob das Attribt im Bereich liegt...
            if ( pAttr->GetEnd() >= nStt )
                return sal_True;
        }
    }
    return sal_False;
}

const String* EdtAutoCorrDoc::GetPrevPara( sal_Bool bAtNormalPos )
{
    // Vorherigen Absatz zurueck geben, damit ermittel werden kann,
    // ob es sich beim aktuellen Wort um einen Satzanfang handelt.

    bAllowUndoAction = sal_False;   // Jetzt nicht mehr...

    ContentList& rNodes = pImpEE->GetEditDoc();
    sal_uInt16 nPos = rNodes.GetPos( pCurNode );

    // Sonderbehandlung: Bullet => Absatzanfang => einfach NULL returnen...
    const SfxUInt16Item& rBulletState = (const SfxUInt16Item&)
            pImpEE->GetParaAttrib( nPos, EE_PARA_BULLETSTATE );
    sal_Bool bBullet = rBulletState.GetValue() ? sal_True : sal_False;
    if ( !bBullet && ( pImpEE->aStatus.GetControlWord() & EE_CNTRL_OUTLINER ) )
    {
        // Der Outliner hat im Gliederungsmodus auf Ebene 0 immer ein Bullet.
        const SfxUInt16Item& rLevel = (const SfxUInt16Item&)
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
    // Absatz-Anfang oder ein Blank gefunden, suche nach dem Wort
    // Kuerzel im Auto

    bAllowUndoAction = sal_False;   // Jetzt nicht mehr...

    String aShort( pCurNode->Copy( rSttPos, nEndPos - rSttPos ) );
    sal_Bool bRet = sal_False;

    if( !aShort.Len() )
        return bRet;

    LanguageType eLang = pImpEE->GetLanguage();
    const SvxAutocorrWord* pFnd = rACorrect.SearchWordsInList( *pCurNode,
                                            rSttPos, nEndPos, *this, eLang );
    if( pFnd && pFnd->IsTextOnly() )
    {
        // dann mal ersetzen
        EditSelection aSel( EditPaM( pCurNode, rSttPos ),
                            EditPaM( pCurNode, nEndPos ) );
        aSel = pImpEE->ImpDeleteSelection( aSel );
        DBG_ASSERT( nCursor >= nEndPos, "Cursor mitten im Geschehen ?!" );
        nCursor -= ( nEndPos-rSttPos );
        pImpEE->ImpInsertText( aSel, pFnd->GetLong() );
        nCursor += pFnd->GetLong().Len();
        if( ppPara )
            *ppPara = pCurNode;
        bRet = sal_True;
    }

    return bRet;
}

LanguageType EdtAutoCorrDoc::GetLanguage( sal_uInt16 nPos, sal_Bool bPrevPara ) const
{
    return pImpEE->GetLanguage();
}

void EdtAutoCorrDoc::ImplStartUndoAction()
{
    sal_uInt16 nPara = pImpEE->GetEditDoc().GetPos( pCurNode );
    ESelection aSel( nPara, nCursor, nPara, nCursor );
    pImpEE->UndoActionStart( EDITUNDO_INSERT, aSel );
    bUndoAction = sal_True;
    bAllowUndoAction = sal_False;
}

