/*************************************************************************
 *
 *  $RCSfile: impedit5.cxx,v $
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
#include <editeng.hxx>
#include <editdbg.hxx>

#include <svtools/smplhint.hxx>


#ifndef _SVX_LRSPITEM_HXX
#include <lrspitem.hxx>
#endif

void ImpEditEngine::SetStyleSheetPool( SfxStyleSheetPool* pSPool )
{
    if ( pStylePool != pSPool )
    {
//      if ( pStylePool )
//          EndListening( *pStylePool, TRUE );

        pStylePool = pSPool;

//      if ( pStylePool )
//          StartListening( *pStylePool, TRUE );
    }
}

SfxStyleSheet* ImpEditEngine::GetStyleSheet( USHORT nPara ) const
{
    ContentNode* pNode = aEditDoc.SaveGetObject( nPara );
    return pNode ? pNode->GetContentAttribs().GetStyleSheet() : NULL;
}

void ImpEditEngine::SetStyleSheet( EditSelection aSel, SfxStyleSheet* pStyle )
{
    aSel.Adjust( aEditDoc );

    USHORT nStartPara = aEditDoc.GetPos( aSel.Min().GetNode() );
    USHORT nEndPara = aEditDoc.GetPos( aSel.Max().GetNode() );

    BOOL bUpdate = GetUpdateMode();
    SetUpdateMode( FALSE );

    for ( USHORT n = nStartPara; n <= nEndPara; n++ )
        SetStyleSheet( n, pStyle );

    SetUpdateMode( bUpdate, 0 );
}

void ImpEditEngine::SetStyleSheet( USHORT nPara, SfxStyleSheet* pStyle )
{
    DBG_ASSERT( GetStyleSheetPool(), "SetStyleSheet: No StyleSheetPool registered!" );
    ContentNode* pNode = aEditDoc.SaveGetObject( nPara );
    SfxStyleSheet* pCurStyle = pNode->GetStyleSheet();
    if ( pStyle != pCurStyle )
    {
        if ( IsUndoEnabled() && !IsInUndo() && aStatus.DoUndoAttribs() )
        {
            XubString aPrevStyleName;
            if ( pCurStyle )
                aPrevStyleName = pCurStyle->GetName();

            InsertUndo(
                new EditUndoSetStyleSheet( this, aEditDoc.GetPos( pNode ),
                        aPrevStyleName,
                        pCurStyle ? pCurStyle->GetFamily() : SFX_STYLE_FAMILY_PARA,
                        pStyle->GetName(), pStyle->GetFamily(),
                        pNode->GetContentAttribs().GetItems() ) );
        }
        if ( pCurStyle )
            EndListening( *pCurStyle, FALSE );
        pNode->SetStyleSheet( pStyle, aStatus.UseCharAttribs() );
        if ( pStyle )
            StartListening( *pStyle, FALSE );
        ParaAttribsChanged( pNode );
    }
    FormatAndUpdate();
}

void ImpEditEngine::UpdateParagraphsWithStyleSheet( SfxStyleSheet* pStyle )
{
    SvxFont aFontFromStyle;
    CreateFont( aFontFromStyle, pStyle->GetItemSet() );

    BOOL bUsed = FALSE;
    for ( USHORT nNode = 0; nNode < aEditDoc.Count(); nNode++ )
    {
        ContentNode* pNode = aEditDoc.GetObject( nNode );
        if ( pNode->GetStyleSheet() == pStyle )
        {
            bUsed = TRUE;
            if ( aStatus.UseCharAttribs() )
                pNode->SetStyleSheet( pStyle, aFontFromStyle );
            else
                pNode->SetStyleSheet( pStyle, FALSE );

            ParaAttribsChanged( pNode );
        }
    }
    if ( bUsed )
    {
        GetEditEnginePtr()->StyleSheetChanged( pStyle );
        FormatAndUpdate();
    }
}

void ImpEditEngine::RemoveStyleFromParagraphs( SfxStyleSheet* pStyle )
{
    for ( USHORT nNode = 0; nNode < aEditDoc.Count(); nNode++ )
    {
        ContentNode* pNode = aEditDoc.GetObject(nNode);
        if ( pNode->GetStyleSheet() == pStyle )
        {
            pNode->SetStyleSheet( NULL );
            ParaAttribsChanged( pNode );
        }
    }
    FormatAndUpdate();
}

void ImpEditEngine::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    // Damit nicht beim Destruieren unnoetig formatiert wird:
    if ( !bDowning )
    {
        DBG_CHKOBJ( GetEditEnginePtr(), EditEngine, 0 );

        SfxStyleSheet* pStyle = NULL;
        ULONG nId = 0;

        if ( rHint.ISA( SfxStyleSheetHint ) )
        {
            const SfxStyleSheetHint& rH = (const SfxStyleSheetHint&) rHint;
            DBG_ASSERT( rH.GetStyleSheet()->ISA( SfxStyleSheet ), "Kein SfxStyleSheet!" );
            pStyle = (SfxStyleSheet*) rH.GetStyleSheet();
            nId = rH.GetHint();
        }
        else if ( ( rHint.Type() == TYPE(SfxSimpleHint ) ) && ( rBC.ISA( SfxStyleSheet ) ) )
        {
            pStyle = (SfxStyleSheet*)&rBC;
            nId = ((SfxSimpleHint&)rHint).GetId();
        }

        if ( pStyle )
        {
            if ( ( nId == SFX_HINT_DYING ) ||
                 ( nId == SFX_STYLESHEET_INDESTRUCTION ) ||
                 ( nId == SFX_STYLESHEET_ERASED ) )
            {
                RemoveStyleFromParagraphs( pStyle );
            }
            else if ( ( nId == SFX_HINT_DATACHANGED ) ||
                      ( nId == SFX_STYLESHEET_MODIFIED ) )
            {
                UpdateParagraphsWithStyleSheet( pStyle );

                // Alle Absaetze mit EditStyles, die das geaenderte Style
                // irgendwie als Parent haben, muessen formatiert werden.
                // ULONG nStyles = pMyStylePool->GetStyles().Count();
                // for ( ULONG nStyle = 0; nStyle < nStyles; nStyle++ )
                // {
                //  EditStyleSheet* pES = (EditStyleSheet*)pMyStylePool->GetStyles().GetObject( nStyle );
                //  DBG_ASSERT( pES, "NULL-Pointer im StyleSheetPool!" );
                //  if ( pES->IsUsed() && pES->HasStyleAsAnyParent( *pStyle ) )
                //      UpdateParagraphsWithStyleSheet( pES );
                // }
            }
        }
    }
}

EditUndoSetAttribs* ImpEditEngine::CreateAttribUndo( EditSelection aSel, const SfxItemSet& rSet )
{
    DBG_ASSERT( !aSel.DbgIsBuggy( aEditDoc ), "CreateAttribUndo: Fehlerhafte Selektion" );
    aSel.Adjust( aEditDoc );

    ESelection aESel( CreateESel( aSel ) );

    USHORT nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    USHORT nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    DBG_ASSERT( nStartNode <= nEndNode, "CreateAttribUndo: Start > End ?!" );

    EditUndoSetAttribs* pUndo = NULL;
    if ( rSet.GetPool() != &aEditDoc.GetItemPool() )
    {
        SfxItemSet aTmpSet( GetEmptyItemSet() );
        aTmpSet.Put( rSet );
        pUndo = new EditUndoSetAttribs( this, aESel, aTmpSet );
    }
    else
    {
        pUndo = new EditUndoSetAttribs( this, aESel, rSet );
    }

    SfxItemPool* pPool = pUndo->GetNewAttribs().GetPool();

    for ( USHORT nPara = nStartNode; nPara <= nEndNode; nPara++ )
    {
        ContentNode* pNode = aEditDoc.GetObject( nPara );
        DBG_ASSERT( aEditDoc.SaveGetObject( nPara ), "Node nicht gefunden: CreateAttribUndo" );
        ContentAttribsInfo* pInf = new ContentAttribsInfo( pNode->GetContentAttribs().GetItems() );
        pUndo->GetContentInfos().Insert( pInf, pUndo->GetContentInfos().Count() );

        for ( USHORT nAttr = 0; nAttr < pNode->GetCharAttribs().Count(); nAttr++ )
        {
            EditCharAttribPtr pAttr = pNode->GetCharAttribs().GetAttribs()[ nAttr ];
            if ( pAttr->GetLen() )
            {
                EditCharAttribPtr pNew = MakeCharAttrib( *pPool, *pAttr->GetItem(), pAttr->GetStart(), pAttr->GetEnd() );
                pInf->GetPrevCharAttribs().Insert( pNew, pInf->GetPrevCharAttribs().Count() );
            }
        }
    }
    return pUndo;
}

void ImpEditEngine::UndoActionStart( USHORT nId, const ESelection& aSel )
{
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        GetUndoManager().EnterListAction( GetEditEnginePtr()->GetUndoComment( nId ), XubString(), nId );
        DBG_ASSERT( !pUndoMarkSelection, "UndoAction SelectionMarker?" );
        pUndoMarkSelection = new ESelection( aSel );
    }
}

void ImpEditEngine::UndoActionStart( USHORT nId )
{
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        GetUndoManager().EnterListAction( GetEditEnginePtr()->GetUndoComment( nId ), XubString(), nId );
        DBG_ASSERT( !pUndoMarkSelection, "UndoAction SelectionMarker?" );
    }
}

void ImpEditEngine::UndoActionEnd( USHORT nId )
{
    if ( IsUndoEnabled() && !IsInUndo() )
    {
        GetUndoManager().LeaveListAction();
        delete pUndoMarkSelection;
        pUndoMarkSelection = NULL;
    }
}

void ImpEditEngine::InsertUndo( EditUndo* pUndo, BOOL bTryMerge )
{
    DBG_ASSERT( !IsInUndo(), "InsertUndo im Undomodus!" );
    if ( pUndoMarkSelection )
    {
        EditUndoMarkSelection* pU = new EditUndoMarkSelection( this, *pUndoMarkSelection );
        GetUndoManager().AddUndoAction( pU, FALSE );
        delete pUndoMarkSelection;
        pUndoMarkSelection = NULL;
    }
    GetUndoManager().AddUndoAction( pUndo, bTryMerge );
}

void ImpEditEngine::ResetUndoManager()
{
    if ( HasUndoManager() )
        GetUndoManager().Clear();
}

void ImpEditEngine::EnableUndo( BOOL bEnable )
{
    // Beim Umschalten des Modus Liste loeschen:
    if ( bEnable != IsUndoEnabled() )
        ResetUndoManager();

    bUndoEnabled = bEnable;
}

BOOL ImpEditEngine::Undo( EditView* pView )
{
    if ( HasUndoManager() && GetUndoManager().GetUndoActionCount() )
    {
        SetActiveView( pView );
        GetUndoManager().Undo( 1 );
        return TRUE;
    }
    return FALSE;
}

BOOL ImpEditEngine::Redo( EditView* pView )
{
    if ( HasUndoManager() && GetUndoManager().GetRedoActionCount() )
    {
        SetActiveView( pView );
        GetUndoManager().Redo( 0 );
        return TRUE;
    }
    return FALSE;
}

BOOL ImpEditEngine::Repeat( EditView* /* pView */ )
{
    if ( HasUndoManager() && GetUndoManager().GetRepeatActionCount() )
    {
        DBG_WARNING( "Repeat nicht implementiert!" );
        return TRUE;
    }
    return FALSE;
}

SfxItemSet ImpEditEngine::GetAttribs( EditSelection aSel )
{
    DBG_CHKOBJ( GetEditEnginePtr(), EditEngine, 0 );

    aSel.Adjust( aEditDoc );

    SfxItemSet aCurSet( GetEmptyItemSet() );

    USHORT nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    USHORT nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );


    // ueber die Absaetze iterieren...
    for ( USHORT nNode = nStartNode; nNode <= nEndNode; nNode++ )
    {
        ContentNode* pNode = aEditDoc.GetObject( nNode );
        DBG_ASSERT( aEditDoc.SaveGetObject( nNode ), "Node nicht gefunden: GetAttrib" );

        xub_StrLen nStartPos = 0;
        xub_StrLen nEndPos = pNode->Len();
        if ( nNode == nStartNode )
            nStartPos = aSel.Min().GetIndex();
        if ( nNode == nEndNode ) // kann auch == nStart sein!
            nEndPos = aSel.Max().GetIndex();

        // Problem: Vorlagen....
        // => Andersrum:
        // 1) Harte Zeichenattribute, wie gehabt...
        // 2) Nur wenn OFF, Style and Absatzattr. pruefen...

        // Erst die ganz harte Formatierung...
        aEditDoc.FindAttribs( pNode, nStartPos, nEndPos, aCurSet );

        // Und dann Absatzformatierung und Vorlage...
        // SfxStyleSheet* pStyle = pNode->GetStyleSheet();
        for ( USHORT nWhich = EE_ITEMS_START; nWhich <= EE_CHAR_END; nWhich++)
        {
            if ( aCurSet.GetItemState( nWhich ) == SFX_ITEM_OFF )
            {
                // Nicht nur, wenn gesetzt, sondern auch Defaults
                /*
                if ( pNode->GetContentAttribs().GetItems().GetItemState( nWhich ) == SFX_ITEM_ON )
                    aCurSet.Put( pNode->GetContentAttribs().GetItems().Get( nWhich ) );
                else if ( pStyle && ( pStyle->GetItemSet().GetItemState( nWhich ) == SFX_ITEM_ON ) )
                    aCurSet.Put( pStyle->GetItemSet().Get( nWhich ) );
                */
                // GetItem() hohlt das richtige...
                const SfxPoolItem& rItem = pNode->GetContentAttribs().GetItem( nWhich );
                aCurSet.Put( rItem );
            }
            else if ( aCurSet.GetItemState( nWhich ) == SFX_ITEM_ON )
            {
                // Generell vergleichen, weil bei SFX_ITEM_OFF das Default-Item aktiv ist.
//              if ( ( pNode->GetContentAttribs().GetItems().GetItemState( nWhich ) == SFX_ITEM_ON ) ||
//                   ( pStyle && ( pStyle->GetItemSet().GetItemState( nWhich ) == SFX_ITEM_ON ) ) )
                {
                    // GetItem() hohlt das richtige...
                    const SfxPoolItem& rItem = pNode->GetContentAttribs().GetItem( nWhich );
                    if ( rItem != aCurSet.Get( nWhich ) )
                    {
                        // Problem: Wenn Absatzvorlage mit z.B. Font,
                        // aber Font hart und anders und komplett in Selektion
                        // Falsch, wenn invalidiert....
                        // => Lieber nicht invalidieren, UMSTELLEN!
                        // Besser waere, Absatzweise ein ItemSet zu fuellen
                        // und dieses mit dem gesmten vergleichen.
//                      aCurSet.InvalidateItem( nWhich );
                        if ( nWhich <= EE_PARA_END )
                            aCurSet.InvalidateItem( nWhich );
                    }
                }
            }
        }
    }

    // Leere Slots mit Defaults fuellen...
    for ( USHORT nWhich = EE_ITEMS_START; nWhich <= EE_CHAR_END; nWhich++ )
    {
        if ( aCurSet.GetItemState( nWhich ) == SFX_ITEM_OFF )
        {
            aCurSet.Put( aEditDoc.GetItemPool().GetDefaultItem( nWhich ) );
        }
    }

#ifdef EDITDEBUG
/*
#ifdef MAC
        FILE* fp = fopen( "debug.log", "a" );
#elif defined UNX
        FILE* fp = fopen( "/tmp/debug.log", "a" );
#else
        FILE* fp = fopen( "d:\\debug.log", "a" );
#endif
        if ( fp )
        {
            fprintf( fp, "\n\n<= Attribute: Absatz %i\n", nNode );
            DbgOutItemSet( fp, aCurSet, TRUE, FALSE );
            fclose( fp );
        }
*/
#endif

    return aCurSet;
}

void ImpEditEngine::SetAttribs( EditSelection aSel, const SfxItemSet& rSet, BYTE nSpecial )
{
    aSel.Adjust( aEditDoc );

    // Wenn keine Selektion => die Attribute aufs Wort anwenden.
    // ( Der RTF-Perser sollte die Methode eigentlich nie ohne Range rufen )
    if ( ( nSpecial == ATTRSPECIAL_WHOLEWORD ) && !aSel.HasRange() )
        aSel = SelectWord( aSel );

    USHORT nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    USHORT nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    if ( IsUndoEnabled() && !IsInUndo() && aStatus.DoUndoAttribs() )
    {
        EditUndoSetAttribs* pUndo = CreateAttribUndo( aSel, rSet );
        pUndo->SetSpecial( nSpecial );
        InsertUndo( pUndo );
    }

    // ueber die Absaetze iterieren...
    for ( USHORT nNode = nStartNode; nNode <= nEndNode; nNode++ )
    {
        BOOL bParaAttribFound = FALSE;
        BOOL bCharAttribFound = FALSE;

        ContentNode* pNode = aEditDoc.GetObject( nNode );
        ParaPortion* pPortion = GetParaPortions().GetObject( nNode );

        DBG_ASSERT( aEditDoc.SaveGetObject( nNode ), "Node nicht gefunden: SetAttribs" );
        DBG_ASSERT( GetParaPortions().GetObject( nNode ), "Portion nicht gefunden: SetAttribs" );

        xub_StrLen nStartPos = 0;
        xub_StrLen nEndPos = pNode->Len();
        if ( nNode == nStartNode )
            nStartPos = aSel.Min().GetIndex();
        if ( nNode == nEndNode ) // kann auch == nStart sein!
            nEndPos = aSel.Max().GetIndex();

        // ueber die Items iterieren...
#ifdef EDITDEBUG
        FILE* fp = fopen( "d:\\debug.log", "a" );
        if ( fp )
        {
            fprintf( fp, "\n\n=> Zeichen-Attribute: Absatz %i, %i-%i\n", nNode, nStartPos, nEndPos );
            DbgOutItemSet( fp, rSet, TRUE, FALSE );
            fclose( fp );
        }
#endif

        for ( USHORT nWhich = EE_ITEMS_START; nWhich <= EE_CHAR_END; nWhich++)
        {
            if ( rSet.GetItemState( nWhich ) == SFX_ITEM_ON )
            {
                const SfxPoolItem& rItem = rSet.Get( nWhich );
                if ( nWhich <= EE_PARA_END )
                {
                    pNode->GetContentAttribs().GetItems().Put( rItem );
                    bParaAttribFound = TRUE;
                }
                else
                {
                    aEditDoc.InsertAttrib( pNode, nStartPos, nEndPos, rItem );
                    bCharAttribFound = TRUE;
                    if ( nSpecial == ATTRSPECIAL_EDGE )
                    {
                        CharAttribArray& rAttribs = pNode->GetCharAttribs().GetAttribs();
                        USHORT nAttrs = rAttribs.Count();
                        for ( USHORT n = 0; n < nAttrs; n++ )
                        {
                            EditCharAttrib* pAttr = rAttribs.GetObject( n );
                            if ( pAttr->GetStart() > nEndPos )
                                break;

                            if ( ( pAttr->GetEnd() == nEndPos ) && ( pAttr->Which() == nWhich ) )
                            {
                                pAttr->SetEdge( TRUE );
                                break;
                            }
                        }
                    }
                }
            }
        }

        if ( bParaAttribFound )
        {
            ParaAttribsChanged( pPortion->GetNode() );
        }
        else if ( bCharAttribFound )
        {
            if ( !pNode->Len() || ( nStartPos != nEndPos  ) )
                pPortion->MarkSelectionInvalid( nStartPos, nEndPos-nStartPos );
        }
    }
}

void ImpEditEngine::RemoveCharAttribs( EditSelection aSel, BOOL bRemoveParaAttribs, USHORT nWhich )
{
    aSel.Adjust( aEditDoc );

    USHORT nStartNode = aEditDoc.GetPos( aSel.Min().GetNode() );
    USHORT nEndNode = aEditDoc.GetPos( aSel.Max().GetNode() );

    const SfxItemSet* pEmptyItemSet = bRemoveParaAttribs ? &GetEmptyItemSet() : 0;

    if ( IsUndoEnabled() && !IsInUndo() && aStatus.DoUndoAttribs() )
    {
        // Eventuel spezielles Undo, oder ItemSet*
        EditUndoSetAttribs* pUndo = CreateAttribUndo( aSel, GetEmptyItemSet() );
        pUndo->SetRemoveAttribs( TRUE );
        pUndo->SetRemoveParaAttribs( bRemoveParaAttribs );
        pUndo->SetRemoveWhich( nWhich );
        InsertUndo( pUndo );
    }

    // ueber die Absaetze iterieren...
    for ( USHORT nNode = nStartNode; nNode <= nEndNode; nNode++ )
    {
        ContentNode* pNode = aEditDoc.GetObject( nNode );
        ParaPortion* pPortion = GetParaPortions().GetObject( nNode );

        DBG_ASSERT( aEditDoc.SaveGetObject( nNode ), "Node nicht gefunden: SetAttribs" );
        DBG_ASSERT( GetParaPortions().SaveGetObject( nNode ), "Portion nicht gefunden: SetAttribs" );

        xub_StrLen nStartPos = 0;
        xub_StrLen nEndPos = pNode->Len();
        if ( nNode == nStartNode )
            nStartPos = aSel.Min().GetIndex();
        if ( nNode == nEndNode ) // kann auch == nStart sein!
            nEndPos = aSel.Max().GetIndex();

        // Optimieren: Wenn ganzer Absatz, dann RemoveCharAttribs( nPara )?!
        BOOL bChanged = aEditDoc.RemoveAttribs( pNode, nStartPos, nEndPos, nWhich );
        if ( bRemoveParaAttribs )
            SetParaAttribs( nNode, *pEmptyItemSet );    // Invalidiert
        else
        {
            // Bei 'Format-Standard' sollen auch die Zeichenattribute verschwinden,
            // die von der DrawingEngine als Absatzattribute eingestellt wurden.
            // Diese koennen sowieso nicht vom Anwender eingestellt worden sein.
            SfxItemSet aAttribs( GetParaAttribs( nNode ) );
            for ( USHORT nW = EE_CHAR_START; nW <= EE_CHAR_END; nW++ )
                aAttribs.ClearItem( nW );
            SetParaAttribs( nNode, aAttribs );
        }

        if ( bChanged && !bRemoveParaAttribs )
            pPortion->MarkSelectionInvalid( nStartPos, nEndPos-nStartPos );
    }
}

typedef EditCharAttrib* EditCharAttribPtr;

void ImpEditEngine::RemoveCharAttribs( USHORT nPara, USHORT nWhich, BOOL bRemoveFeatures )
{
    ContentNode* pNode = aEditDoc.SaveGetObject( nPara );
    ParaPortion* pPortion = GetParaPortions().SaveGetObject( nPara );

    DBG_ASSERT( pNode, "Node nicht gefunden: RemoveCharAttribs" );
    DBG_ASSERT( pPortion, "Portion nicht gefunden: RemoveCharAttribs" );

    if ( !pNode )
        return;

    USHORT nAttr = 0;
    EditCharAttribPtr pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    while ( pAttr )
    {
        if ( ( !pAttr->IsFeature() || bRemoveFeatures ) &&
             ( !nWhich || ( pAttr->GetItem()->Which() == nWhich ) ) )
        {
            pNode->GetCharAttribs().GetAttribs().Remove( nAttr );
            delete pAttr;
            nAttr--;
        }
        nAttr++;
        pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    }

    pPortion->MarkSelectionInvalid( 0, pNode->Len() );
}

void ImpEditEngine::SetParaAttribs( USHORT nPara, const SfxItemSet& rSet )
{
    ContentNode* pNode = aEditDoc.SaveGetObject( nPara );

    if ( !pNode )
        return;

#ifdef EDITDEBUG
        FILE* fp = fopen( "d:\\debug.log", "a" );
        if ( fp )
        {
            fprintf( fp, "\n\n=> Absatz-Attribute: Absatz %i\n", nPara );
            DbgOutItemSet( fp, rSet, TRUE, FALSE );
            fclose( fp );
        }
#endif

    if ( !( pNode->GetContentAttribs().GetItems() == rSet ) )
    {
        if ( IsUndoEnabled() && !IsInUndo() && aStatus.DoUndoAttribs() )
        {
            if ( rSet.GetPool() != &aEditDoc.GetItemPool() )
            {
                SfxItemSet aTmpSet( GetEmptyItemSet() );
                aTmpSet.Put( rSet );
                InsertUndo( new EditUndoSetParaAttribs( this, nPara, pNode->GetContentAttribs().GetItems(), aTmpSet ) );
            }
            else
            {
                InsertUndo( new EditUndoSetParaAttribs( this, nPara, pNode->GetContentAttribs().GetItems(), rSet ) );
            }
        }
        pNode->GetContentAttribs().GetItems().Set( rSet );
        if ( aStatus.UseCharAttribs() )
            pNode->CreateDefFont();

        ParaAttribsChanged( pNode );
    }
}

const SfxItemSet& ImpEditEngine::GetParaAttribs( USHORT nPara ) const
{
    ContentNode* pNode = aEditDoc.GetObject( nPara );
    DBG_ASSERT( pNode, "Node nicht gefunden: GetParaAttribs" );
    return pNode->GetContentAttribs().GetItems();
}

BOOL ImpEditEngine::HasParaAttrib( USHORT nPara, USHORT nWhich ) const
{
    ContentNode* pNode = aEditDoc.GetObject( nPara );
    DBG_ASSERT( pNode, "Node nicht gefunden: HasParaAttrib" );

    // LRSPACE kann ggf. auch OUTLLRSPACE sein!

    BOOL bHasItem = pNode->GetContentAttribs().HasItem( nWhich );
    if ( nWhich == EE_PARA_LRSPACE )
    {
        if ( aStatus.IsOutliner() ||
             ( aStatus.IsOutliner2() && !pNode->GetContentAttribs().HasItem( EE_PARA_LRSPACE ) ) )
        {
            bHasItem = pNode->GetContentAttribs().HasItem( EE_PARA_OUTLLRSPACE );
        }
    }
    return bHasItem;
}

const SfxPoolItem& ImpEditEngine::GetParaAttrib( USHORT nPara, USHORT nWhich )
{
    ContentNode* pNode = aEditDoc.GetObject( nPara );
    DBG_ASSERT( pNode, "Node nicht gefunden: GetParaAttrib" );

    // LRSPACE kann ggf. auch OUTLLRSPACE sein!
    if ( nWhich != EE_PARA_LRSPACE )
        return pNode->GetContentAttribs().GetItem( nWhich );
    else
        return GetLRSpaceItem( pNode );
}

void ImpEditEngine::GetCharAttribs( USHORT nPara, EECharAttribArray& rLst ) const
{
    rLst.Remove( 0, rLst.Count() );
    ContentNode* pNode = aEditDoc.GetObject( nPara );
    if ( pNode )
    {
        for ( USHORT nAttr = 0; nAttr < pNode->GetCharAttribs().Count(); nAttr++ )
        {
            EditCharAttribPtr pAttr = pNode->GetCharAttribs().GetAttribs()[ nAttr ];
            EECharAttrib aEEAttr;
            aEEAttr.pAttr = pAttr->GetItem();
            aEEAttr.nPara = nPara;
            aEEAttr.nStart = pAttr->GetStart();
            aEEAttr.nEnd = pAttr->GetEnd();
            rLst.Insert( aEEAttr, rLst.Count() );
        }
    }
}

void ImpEditEngine::ParaAttribsToCharAttribs( ContentNode* pNode )
{
    pNode->GetCharAttribs().DeleteEmptyAttribs( GetEditDoc().GetItemPool() );
    xub_StrLen nEndPos = pNode->Len();
    for ( USHORT nWhich = EE_CHAR_START; nWhich <= EE_CHAR_END; nWhich++ )
    {
        if ( pNode->GetContentAttribs().HasItem( nWhich ) )
        {
            const SfxPoolItem& rItem = pNode->GetContentAttribs().GetItem( nWhich );
            // Die Luecken auffuellen:
            USHORT nLastEnd = 0;
            EditCharAttrib* pAttr = pNode->GetCharAttribs().FindNextAttrib( nWhich, nLastEnd );
            while ( pAttr )
            {
                if ( pAttr->GetStart() > nLastEnd )
                    aEditDoc.InsertAttrib( pNode, nLastEnd, pAttr->GetStart(), rItem );
                nLastEnd = pAttr->GetEnd();
                pAttr = pNode->GetCharAttribs().FindNextAttrib( nWhich, nLastEnd );
            }

            // Und den Rest:
            if ( nLastEnd < nEndPos )
                aEditDoc.InsertAttrib( pNode, nLastEnd, nEndPos, rItem );
        }
    }
    // Portion braucht hier nicht invalidiert werden, geschieht woanders.
}

IdleFormattter::IdleFormattter()
{
    pView = 0;
    nRestarts = 0;
}

IdleFormattter::~IdleFormattter()
{
    pView = 0;
}

void IdleFormattter::DoIdleFormat( EditView* pV )
{
    pView = pV;

    if ( IsActive() )
        nRestarts++;

    if ( nRestarts > 4 )
        ForceTimeout();
    else
        Start();
}

void IdleFormattter::ForceTimeout()
{
    if ( IsActive() )
    {
        Stop();
        ((Link&)GetTimeoutHdl()).Call( this );
    }
}

ImplIMEInfos::ImplIMEInfos( const EditPaM& rPos )
{
    aPos = rPos;
    nLen = 0;
    bCursor = TRUE;
    pAttribs = NULL;
}

ImplIMEInfos::~ImplIMEInfos()
{
    delete pAttribs;
}

void ImplIMEInfos::CopyAttribs( const USHORT* pA, USHORT nL )
{
    nLen = nL;
    delete pAttribs;
    pAttribs = new USHORT[ nL ];
    memcpy( pAttribs, pA, nL*sizeof(USHORT) );
}

void ImplIMEInfos::DestroyAttribs()
{
    delete pAttribs;
    pAttribs = NULL;
}
