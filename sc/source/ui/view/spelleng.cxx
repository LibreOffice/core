/*************************************************************************
 *
 *  $RCSfile: spelleng.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:09 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/langitem.hxx>
#include <svx/editobj.hxx>
#include <svx/editview.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/system.hxx>

#ifndef ONE_LINGU
#include <hm2/splchk.hxx>
#endif

#include "spelleng.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "cell.hxx"
#include "patattr.hxx"
#include "waitoff.hxx"
#include "globstr.hrc"


// -----------------------------------------------------------------------

BOOL lcl_HasString( ScDocument* pDoc, USHORT nCol, USHORT nRow, USHORT nTab,
                    const String& rString )
{
    String aCompStr;
    pDoc->GetString( nCol, nRow, nTab, aCompStr );
    return ( aCompStr == rString );     //! case-insensitive?
}

BOOL __EXPORT ScSpellingEngine::SpellNextDocument()
{
    ScDocShell* pDocSh = pViewData->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScMarkData& rMark = pViewData->GetMarkData();
    ScTabViewShell* pViewShell = pViewData->GetViewShell();
    ScSplitPos eWhich = pViewData->GetActivePart();
    CellType eCellType;
    ScBaseCell* pCell = NULL;
    LanguageType eLnge;
    EditTextObject* pETObject = NULL;
    const SfxPoolItem* pItem = NULL;
    SvxLanguageItem* pLangIt = NULL;
    const ScPatternAttr* pPattern = NULL;
    const ScPatternAttr* pLastPattern = NULL;
    SfxItemSet* pEditDefaults = new SfxItemSet( GetEmptyItemSet() );

    if (IsModified())
    {
        bIsModifiedAtAll = TRUE;

        String aNewStr = GetText();

        BOOL bMultiTab = ( rMark.GetSelectCount() > 1 );
        String aVisibleStr;
        if (bMultiTab)
            pDoc->GetString( nOldCol, nOldRow, nOrgTab, aVisibleStr );

        USHORT nTabCount = pDoc->GetTableCount();
        for ( USHORT nTab=0; nTab<nTabCount; nTab++ )
        {
            //  #69965# always change the cell on the visible tab,
            //  on the other selected tabs only if they contain the same text

            if ( nTab == nOrgTab || ( bMultiTab && rMark.GetTableSelect(nTab) &&
                                    lcl_HasString( pDoc, nOldCol, nOldRow, nTab, aVisibleStr ) ) )
            {
                pDoc->GetCellType(nOldCol, nOldRow, nTab, eCellType);
                pDoc->GetCell(nOldCol, nOldRow, nTab, pCell);
                if (pUndoDoc && pCell)
                {
                    ScBaseCell* pUndoCell = pCell->Clone(pUndoDoc);
                    pUndoDoc->PutCell(nOldCol, nOldRow, nTab, pUndoCell);
                }
                if (eCellType == CELLTYPE_EDIT)
                {
                    if (pCell)
                    {
                        pETObject = CreateTextObject();
                        ((ScEditCell*) pCell)->SetData( pETObject, GetEditTextObjectPool() );
                        delete pETObject;
                    }
                }
                else
                {
                    pDoc->SetString(nOldCol, nOldRow, nTab, aNewStr);
                    pDoc->GetCell(nOldCol, nOldRow, nTab, pCell);
                }
                if (pRedoDoc && pCell)
                {
                    ScBaseCell* pRedoCell = pCell->Clone(pRedoDoc);
                    pRedoDoc->PutCell(nOldCol, nOldRow, nTab, pRedoCell);
                }
                pDocSh->PostPaintCell(nOldCol, nOldRow, nTab);
            }
        }
    }
    pCell = NULL;
    USHORT nCol, nRow;
    BOOL bStop = FALSE;
    BOOL bNext;
    nCol = nOldCol;
    if (bFirstTime)
    {
        bFirstTime = FALSE;
        if (nOldCol == nOrgCol && nOldRow == nOrgRow)   // das erste Mal
            nRow = nOldRow-1;
        else
            nRow = nOldRow;
    }
    else
        nRow = nOldRow;
    while (!bStop)
    {
        bNext = pDoc->GetNextSpellingCell(nCol, nRow, nOrgTab, bInSel, rMark);
        if (bNext)
        {
            if (!bFirstTable &&
                 (nCol > nOrgCol || (nCol == nOrgCol && nRow >= nOrgRow)))
            {
                pDoc->GetCellType(nOldCol, nOldRow, nOrgTab, eCellType);
                if (eCellType == CELLTYPE_STRING)
                {
                    String sOldText;
                    pDoc->GetString(nOldCol, nOldRow, nOrgTab, sOldText);
                    SetText(sOldText);
                }
                else if (eCellType == CELLTYPE_EDIT)
                {
                    pDoc->GetCell(nOldCol, nOldRow, nOrgTab, pCell);
                    if (pCell)
                    {
                        const EditTextObject* pNewTObj = NULL;
                        ((ScEditCell*) pCell)->GetData(pNewTObj);
                        if (pNewTObj)
                            SetText(*pNewTObj);
                    }
                }
                else
                    SetText(EMPTY_STRING);

                {   // own scope for WaitCursorOff
                    ScWaitCursorOff aWaitOff( pDocSh->GetDialogParent() );
                    InfoBox aBox( pViewData->GetDialogParent(),
                        ScGlobal::GetRscString(STR_SPELLING_STOP_OK));
                    aBox.Execute();
                }
                return FALSE;
            }
            else if ( nCol == MAXCOL+1 )
            {
                if ( nOrgCol == 0 && nOrgRow == 0 )
                {
                    pDoc->GetCellType(nOldCol, nOldRow, nOrgTab, eCellType);
                    if (eCellType == CELLTYPE_STRING)
                    {
                        String sOldText;
                        pDoc->GetString(nOldCol, nOldRow, nOrgTab, sOldText);
                        SetText(sOldText);
                    }
                    else if (eCellType == CELLTYPE_EDIT)
                    {
                        pDoc->GetCell(nOldCol, nOldRow, nOrgTab, pCell);
                        if (pCell)
                        {
                            const EditTextObject* pNewTObj = NULL;
                            ((ScEditCell*) pCell)->GetData(pNewTObj);
                            if (pNewTObj)
                                SetText(*pNewTObj);
                        }
                    }
                    else
                        SetText(EMPTY_STRING);

                    {   // own scope for WaitCursorOff
                        ScWaitCursorOff aWaitOff( pDocSh->GetDialogParent() );
                        InfoBox aBox( pViewData->GetDialogParent(),
                            ScGlobal::GetRscString(STR_SPELLING_STOP_OK));
                        aBox.Execute();
                    }

                    return FALSE;
                }
                else
                {
                                        // zuerst auf letzte Zelle zuruecksetzen
                                        // fuer Paint
                    pDoc->GetCellType(nOldCol, nOldRow, nOrgTab, eCellType);
                    if (eCellType == CELLTYPE_STRING)
                    {
                        String sOldText;
                        pDoc->GetString(nOldCol, nOldRow, nOrgTab, sOldText);
                        SetText(sOldText);
                    }
                    else if (eCellType == CELLTYPE_EDIT)
                    {
                        pDoc->GetCell(nOldCol, nOldRow, nOrgTab, pCell);
                        if (pCell)
                        {
                            const EditTextObject* pNewTObj = NULL;
                            ((ScEditCell*) pCell)->GetData(pNewTObj);
                            if (pNewTObj)
                                SetText(*pNewTObj);
                        }
                    }
                    else
                        SetText(EMPTY_STRING);

                    short nRet;
                    {   // own scope for WaitCursorOff
                        ScWaitCursorOff aWaitOff( pDocSh->GetDialogParent() );
                        nRet = MessBox( pViewData->GetDialogParent(),
                            WinBits(WB_YES_NO | WB_DEF_YES),
                            ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),       // "StarCalc"
                            ScGlobal::GetRscString( STR_SPELLING_BEGIN_TAB) )       // Fortsetzen?
                                       .Execute();
                    }
                    if (nRet == RET_YES)
                    {
                        nRow = MAXROW+2;
                        bFirstTable = FALSE;
                    }
                    else
                        return FALSE;
                }
            }
            else                    // Stringzelle mit Inhalt
            {
                pPattern = pDoc->GetPattern(nCol, nRow, nOrgTab);
                if (pPattern && pPattern != pLastPattern)
                {
                    pPattern->FillEditItemSet( pEditDefaults );
                    SetDefaults( *pEditDefaults );
                    pLastPattern = pPattern;
                }
                pItem = pDoc->GetAttr(nCol, nRow, nOrgTab, ATTR_FONT_LANGUAGE);
                pLangIt = PTR_CAST( SvxLanguageItem, pItem );
                if (pLangIt)
                {
                    eLnge = (LanguageType) pLangIt->GetValue();
                    if ( eLnge == LANGUAGE_SYSTEM )
                        eLnge = System::GetLanguage();          // Spelling nie mit SYSTEM
                    if (eLnge != eOldLnge)
                    {
                        eOldLnge = eLnge;
#ifdef ONE_LINGU
                        SetDefaultLanguage( eLnge );
#else
                        SpellCheck* pSpCheck = GetSpeller();
                        if (pSpCheck)
                            pSpCheck->SetActualLanguage(eLnge);
                        else
                        {
                            DBG_ERROR(
                            "ScSpellingEngine::SpellNextDoc: Kein Spell Checker");
                        }
#endif
                    }
                }
                pDoc->GetCellType(nCol, nRow, nOrgTab, eCellType);
                if (eCellType == CELLTYPE_STRING)
                {
                    String sOldText;
                    pDoc->GetString(nCol, nRow, nOrgTab, sOldText);
                    SetText(sOldText);
                }
                else if (eCellType == CELLTYPE_EDIT)
                {
                    pDoc->GetCell(nCol, nRow, nOrgTab, pCell);
                    if (pCell)
                    {
                        const EditTextObject* pNewTObj = NULL;
                        ((ScEditCell*) pCell)->GetData(pNewTObj);
                        if (pNewTObj)
                            SetText(*pNewTObj);
                    }
                }
                else
                    SetText(EMPTY_STRING);
                if (HasSpellErrors(eLnge))
                    bStop = TRUE;
            }
        }
        else
            return FALSE;
    }
    pViewShell->AlignToCursor( nCol, nRow, SC_FOLLOW_JUMP );
    pViewShell->SetCursor( nCol, nRow, TRUE );
    pViewData->GetView()->MakeEditView(this, nCol, nRow );
    EditView* pEditView = pViewData->GetSpellingView();
    if (pEditSel)                                   // hoechstens beim ersten Mal
    {
        pEditView->SetSelection(*pEditSel);
        pEditSel = NULL;
    }
    else
        pEditView->SetSelection(ESelection(0,0,0,0));
    ClearModifyFlag();
    nOldCol = nCol;
    nOldRow = nRow;
    delete pEditDefaults;
    return TRUE;
}




