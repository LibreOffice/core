/*************************************************************************
 *
 *  $RCSfile: viewfun4.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-14 15:34:07 $
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

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif

#define ITEMID_FIELD EE_FEATURE_FIELD
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>
#include <svx/editview.hxx>
#include <svx/flditem.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/langitem.hxx>
#include <svx/langtab.hxx>
#include <svx/svxerr.hxx>
#include <svx/impgrf.hxx>
#include <svx/unolingu.hxx>
#include <so3/ipobj.hxx>
#include <so3/svstor.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <svtools/filter.hxx>
#include <svtools/stritem.hxx>
#include <svtools/urlbmk.hxx>
#include <vcl/drag.hxx>
#include <vcl/system.hxx>
#include <vcl/msgbox.hxx>

#include "viewfunc.hxx"
#include "docsh.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "undoblk.hxx"
#include "undocell.hxx"
#include "cell.hxx"
#include "scmod.hxx"
#include "spelleng.hxx"
#include "patattr.hxx"
#include "sc.hrc"
#include "tabvwsh.hxx"
#include "impex.hxx"
#include "editutil.hxx"

// STATIC DATA -----------------------------------------------------------

BOOL bPasteIsDrop = FALSE;

//==================================================================

void ScViewFunc::PasteRTF( USHORT nStartCol, USHORT nStartRow, SvDataObject* pObject )
{
    if ( pObject->HasFormat( EditEngine::RegisterClipboardFormatName() ) )
    {   // EditEngine eigen wg. Drag&Drop und PasteSpecial
        HideAllCursors();

        ScDocument* pUndoDoc = NULL;

        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        ScDocument* pDoc = pDocSh->GetDocument();
        USHORT nTab = GetViewData()->GetTabNo();
        const BOOL bRecord (pDoc->IsUndoEnabled());

        const ScPatternAttr* pPattern = pDoc->GetPattern( nStartCol, nStartRow, nTab );
        ScTabEditEngine* pEngine = new ScTabEditEngine( *pPattern, pDoc->GetEnginePool() );
        pEngine->EnableUndo( FALSE );

        Window* pActWin = GetActiveWin();
        if (pActWin)
        {
            pEngine->SetPaperSize(Size(100000,100000));
            Window aWin( pActWin );
            EditView aEditView( pEngine, &aWin );
            aEditView.SetOutputArea(Rectangle(0,0,100000,100000));

            if (bPasteIsDrop)
            {
                DropEvent aDropEvt;
                if (aEditView.QueryDrop( aDropEvt ))
                    aEditView.Drop( aDropEvt );
            }
            else
                aEditView.PasteSpecial();
        }

        ULONG nParCnt = pEngine->GetParagraphCount();
        if (nParCnt)
        {
            USHORT nEndRow = nStartRow + (USHORT) nParCnt - 1;
            if (nEndRow > MAXROW)
                nEndRow = MAXROW;

            if (bRecord)
            {
                pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
                pUndoDoc->InitUndo( pDoc, nTab, nTab );
                pDoc->CopyToDocument( nStartCol,nStartRow,nTab, nStartCol,nEndRow,nTab, IDF_ALL, FALSE, pUndoDoc );
            }

            USHORT nRow = nStartRow;
            for( USHORT n = 0; n < nParCnt; n++ )
            {
                EditTextObject* pObject = pEngine->CreateTextObject( n );
                EnterData( nStartCol, nRow, nTab, pObject, FALSE, TRUE );
                            // kein Undo, auf einfache Strings testen
                delete pObject;
                if( ++nRow > MAXROW )
                    break;
            }

            if (bRecord)
            {
                ScDocument* pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
                pRedoDoc->InitUndo( pDoc, nTab, nTab );
                pDoc->CopyToDocument( nStartCol,nStartRow,nTab, nStartCol,nEndRow,nTab, IDF_ALL, FALSE, pRedoDoc );

                ScMarkData aDestMark;
                aDestMark.SelectOneTable( nTab );
                pDocSh->GetUndoManager()->AddUndoAction(
                    new ScUndoPaste( pDocSh, nStartCol,nStartRow,nTab, nStartCol,nEndRow,nTab,
                                        aDestMark,
                                        pUndoDoc, pRedoDoc, IDF_ALL, NULL,NULL,NULL,NULL ) );
            }
        }

        delete pEngine;

        ShowAllCursors();
    }
    else
    {
        SvData aData( FORMAT_RTF );
        if (pObject->GetData( &aData ))
        {
            HideAllCursors();
            ScDocShell* pDocSh = GetViewData()->GetDocShell();
            ScImportExport aImpEx( pDocSh->GetDocument(),
                ScAddress( nStartCol, nStartRow, GetViewData()->GetTabNo() ) );
            aImpEx.ImportData( aData );
            AdjustRowHeight( nStartRow, aImpEx.GetRange().aEnd.Row() );
            pDocSh->UpdateOle(GetViewData());
            ShowAllCursors();
        }
    }
}

//  Thesaurus - Undo ok
void ScViewFunc::DoThesaurus( BOOL bRecord )
{
    USHORT nCol, nRow, nTab;
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    ScSplitPos eWhich = GetViewData()->GetActivePart();
    LanguageType eLnge;
    CellType eCellType;
    EESpellState eState;
    String sOldText, sNewString;
    EditTextObject* pOldTObj = NULL;
    const EditTextObject* pTObject = NULL;
    ScBaseCell* pCell = NULL;
    EditView* pEditView = NULL;
    ESelection* pEditSel = NULL;
    ScEditEngineDefaulter* pThesaurusEngine;
    BOOL bIsEditMode = GetViewData()->HasEditView(eWhich);
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
    if (bIsEditMode)                                            // Edit-Mode aktiv
    {
        GetViewData()->GetEditView(eWhich, pEditView, nCol, nRow);
        pEditSel = new ESelection(pEditView->GetSelection());
        SC_MOD()->InputEnterHandler();
        GetViewData()->GetBindings().Update();          // sonst kommt der Sfx durcheinander...
    }
    else
    {
        nCol = GetViewData()->GetCurX();
        nRow = GetViewData()->GetCurY();
    }
    nTab = GetViewData()->GetTabNo();

    if (!pDoc->IsSelectedBlockEditable( nCol, nRow, nCol, nRow, rMark ))
    {
        ErrorMessage(STR_PROTECTIONERR);
        delete pEditSel;
        return;
    }
    pDoc->GetCellType(nCol, nRow, nTab, eCellType);
    if (eCellType != CELLTYPE_STRING && eCellType != CELLTYPE_EDIT)
    {
        ErrorMessage(STR_THESAURUS_NO_STRING);
        return;
    }

    com::sun::star::uno::Reference<com::sun::star::linguistic2::XSpellChecker1>
                                        xSpeller = LinguMgr::GetSpellChecker();
    //! if (...)  // thesaurus not available
    //! {
    //!     ErrorMessage(STR_EXPORT_ASCII_WARNING);
    //!     delete pEditSel;
    //!     return;
    //! }

    eLnge = ScViewUtil::GetEffLanguage( pDoc, ScAddress( nCol, nRow, nTab ) );

    pThesaurusEngine = new ScEditEngineDefaulter( pDoc->GetEnginePool() );
    pThesaurusEngine->SetEditTextObjectPool( pDoc->GetEditPool() );
    pThesaurusEngine->SetRefDevice(GetViewData()->GetActiveWin());
    pThesaurusEngine->SetSpeller(xSpeller);
    MakeEditView(pThesaurusEngine, nCol, nRow );
    const ScPatternAttr* pPattern = NULL;
    SfxItemSet* pEditDefaults = new SfxItemSet(pThesaurusEngine->GetEmptyItemSet());
    pPattern = pDoc->GetPattern(nCol, nRow, nTab);
    if (pPattern )
    {
        pPattern->FillEditItemSet( pEditDefaults );
        pThesaurusEngine->SetDefaults( *pEditDefaults );
    }

    if (eCellType == CELLTYPE_STRING)
    {
        pDoc->GetString(nCol, nRow, nTab, sOldText);
        pThesaurusEngine->SetText(sOldText);
    }
    else if (eCellType == CELLTYPE_EDIT)
    {
        pDoc->GetCell(nCol, nRow, nTab, pCell);
        if (pCell)
        {
            ((ScEditCell*) pCell)->GetData(pTObject);
            pOldTObj = pTObject->Clone();
            if (pTObject)
                pThesaurusEngine->SetText(*pTObject);
        }
    }
    else
    {
        DBG_ERROR("DoThesaurus: Keine String oder Editzelle");
    }
    pEditView = GetViewData()->GetEditView(GetViewData()->GetActivePart());;
    if (pEditSel)
        pEditView->SetSelection(*pEditSel);
    else
        pEditView->SetSelection(ESelection(0,0,0,0));

    pThesaurusEngine->ClearModifyFlag();

    eState = pEditView->StartThesaurus( eLnge );
    DBG_ASSERT(eState != EE_SPELL_NOSPELLER, "No SpellChecker");

    if (eState == EE_SPELL_ERRORFOUND)              // sollte spaeter durch Wrapper geschehen!
    {
        SvxLanguageTable aLangTab;
        String aErr = aLangTab.GetString(eLnge);
        aErr += ScGlobal::GetRscString( STR_SPELLING_NO_LANG );
        InfoBox aBox( GetViewData()->GetDialogParent(), aErr );
        aBox.Execute();
    }
    if (pThesaurusEngine->IsModified())
    {
        EditTextObject* pNewTObj = NULL;
        if (pCell && pTObject)
        {
            pNewTObj = pThesaurusEngine->CreateTextObject();
            pCell = new ScEditCell( pNewTObj, pDoc,
                pThesaurusEngine->GetEditTextObjectPool() );
            pDoc->PutCell( nCol, nRow, nTab, pCell );
        }
        else
        {
            sNewString = pThesaurusEngine->GetText();
            pDoc->SetString(nCol, nRow, nTab, sNewString);
        }
// erack! it's broadcasted
//      pDoc->SetDirty();
        pDocSh->SetDocumentModified();
        if (bRecord)
        {
            GetViewData()->GetDocShell()->GetUndoManager()->AddUndoAction(
                new ScUndoThesaurus( GetViewData()->GetDocShell(),
                                     nCol, nRow, nTab,
                                     sOldText, pOldTObj, sNewString, pNewTObj));
        }
        delete pNewTObj;
    }
    KillEditView(TRUE);
    delete pEditDefaults;
    delete pThesaurusEngine;
    delete pOldTObj;
    delete pEditSel;
    pDocSh->PostPaintGridAll();
}

//  Spelling Checker - Undo ok
void ScViewFunc::DoSpellingChecker( BOOL bRecord )
{
    USHORT nCol, nRow, nTab;
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    ScSplitPos eWhich = GetViewData()->GetActivePart();
    EESpellState eState;
    EditView* pEditView = NULL;
    ESelection* pEditSel = NULL;
    ScSpellingEngine* pSpellingEngine = NULL;
    BOOL bIsEditMode = GetViewData()->HasEditView(eWhich);
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
    if (bIsEditMode)                                            // Edit-Mode aktiv
    {
        GetViewData()->GetEditView(eWhich, pEditView, nCol, nRow);
        pEditSel = new ESelection(pEditView->GetSelection());
        SC_MOD()->InputEnterHandler();
    }
    else
    {
        nCol = GetViewData()->GetCurX();
        nRow = GetViewData()->GetCurY();

        AlignToCursor( nCol, nRow, SC_FOLLOW_JUMP);
    }
    nTab = GetViewData()->GetTabNo();

    rMark.MarkToMulti();
    BOOL bMarked = rMark.IsMultiMarked();
    if (bMarked && !pDoc->IsSelectionEditable(rMark))
    {
        ErrorMessage(STR_PROTECTIONERR);
        delete pEditSel;
        return;
    }

    ScDocument* pUndoDoc = NULL;
    ScDocument* pRedoDoc = NULL;
    if (bRecord)
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nTab, nTab );
        pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
        pRedoDoc->InitUndo( pDoc, nTab, nTab );

        if ( rMark.GetSelectCount() > 1 )
        {
            USHORT nTabCount = pDoc->GetTableCount();
            for (USHORT i=0; i<nTabCount; i++)
                if ( rMark.GetTableSelect(i) && i != nTab )
                {
                    pUndoDoc->AddUndoTab( i, i );
                    pRedoDoc->AddUndoTab( i, i );
                }
        }
    }

    //! no way to set a spelling error handler
    com::sun::star::uno::Reference<com::sun::star::linguistic2::XSpellChecker1>
                                        xSpeller = LinguMgr::GetSpellChecker();

    //  ab hier kein return mehr

    BOOL bOldDis = pDoc->IsIdleDisabled();
    pDoc->DisableIdle(TRUE);    // nicht mit Online-Spelling durcheinanderkommen (#42726#)

    pSpellingEngine = new ScSpellingEngine( pDoc->GetEnginePool(),
                                           GetViewData(), pUndoDoc, pRedoDoc,
                                           nCol, nRow, nTab,
                                           bMarked, LANGUAGE_ENGLISH_US, pEditSel);
    pSpellingEngine->SetSpeller(xSpeller);
    MakeEditView(pSpellingEngine, nCol, nRow );
    pSpellingEngine->SetRefDevice(GetViewData()->GetActiveWin());
                                        // dummy Zelle simulieren:
    pEditView = GetViewData()->GetEditView(GetViewData()->GetActivePart());
    GetViewData()->SetSpellingView(pEditView);
    Rectangle aRect(Point(0,0), Point(0,0));
    pEditView->SetOutputArea(aRect);
    pSpellingEngine->SetControlWord( EE_CNTRL_USECHARATTRIBS );
    pSpellingEngine->EnableUndo( FALSE );
    pSpellingEngine->SetPaperSize( aRect.GetSize() );
    pSpellingEngine->SetText(EMPTY_STRING);

    pSpellingEngine->ClearModifyFlag();
    BOOL bFound = pSpellingEngine->SpellNextDocument();     // erste Zelle holen

    if (bFound)
        eState = pEditView->StartSpeller( pSpellingEngine->GetLanguage(), TRUE );
    else
        eState = EE_SPELL_OK;                               // Bereich war leer

    DBG_ASSERT(eState != EE_SPELL_NOSPELLER, "No SpellChecker");
    if (eState == EE_SPELL_NOLANGUAGE)
    {
        ErrorMessage(STR_NOLANGERR);
    }
    if (pSpellingEngine->IsModifiedAtAll())
    {
        if (bRecord)
        {
            USHORT nNewCol = GetViewData()->GetCurX();
            USHORT nNewRow = GetViewData()->GetCurY();
            GetViewData()->GetDocShell()->GetUndoManager()->AddUndoAction(
                new ScUndoSpelling( GetViewData()->GetDocShell(),
                            rMark,
                            nCol, nRow, nTab,
                            pUndoDoc,
                            nNewCol, nNewRow, nTab,
                            pRedoDoc ) );
        }
        pDoc->SetDirty();
        pDocSh->SetDocumentModified();
    }
    else
    {
        delete pUndoDoc;
        delete pRedoDoc;
    }
    GetViewData()->SetSpellingView( NULL );
    KillEditView(TRUE);
    delete pSpellingEngine;
    delete pEditSel;
    pDocSh->PostPaintGridAll();
    GetViewData()->GetViewShell()->UpdateInputHandler();
    pDoc->DisableIdle(bOldDis);
}

IMPL_LINK_INLINE_START( ScViewFunc, SpellError, void *, nLang )
{
    SvxLanguageTable aLangTab;
    String aErr = aLangTab.GetString((LanguageType) (ULONG) nLang);
    ErrorHandler::HandleError(*new StringErrorInfo(
                                ERRCODE_SVX_LINGU_LANGUAGENOTEXISTS, aErr) );

    return 0;
}
IMPL_LINK_INLINE_END( ScViewFunc, SpellError, void *, nLang )

// Pasten von FORMAT_FILE-Items
//  wird nicht direkt aus Drop aufgerufen, sondern asynchron -> Dialoge sind erlaubt

BOOL ScViewFunc::PasteFile( const Point& rPos, const String& rFile, BOOL bLink )
{
    if (!bLink)     // bei bLink nur Grafik oder URL
    {
        // 1. Kann ich die Datei oeffnen?
        const SfxFilter* pFlt = NULL;

        // nur nach eigenen Filtern suchen, ohne Auswahlbox (wie in ScDocumentLoader)
        SfxFilterMatcher aMatcher( ScDocShell::Factory().GetFilterContainer() );
        SfxMedium aSfxMedium( rFile, (STREAM_READ | STREAM_SHARE_DENYNONE), FALSE );
        ErrCode nErr = aMatcher.GuessFilter( aSfxMedium, &pFlt );

        if ( pFlt && !nErr )
        {
            // Code aus dem SFX geklaut!
            SfxDispatcher &rDispatcher = GetViewData()->GetDispatcher();
            SfxStringItem aFileNameItem( SID_FILE_NAME, rFile );
            SfxStringItem aFilterItem( SID_FILTER_NAME, pFlt->GetName() );

            // Asynchron oeffnen, kann naemlich auch aus D&D heraus passieren
            // und das bekommt dem MAC nicht so gut ...
            return BOOL( 0 != rDispatcher.Execute( SID_OPENDOC,
                                    SFX_CALLMODE_ASYNCHRON, &aFileNameItem, &aFilterItem, 0L) );
        }
    }

    // 2. Kann die Datei ueber die Grafik-Filter eingefuegt werden?
    // (als Link, weil Gallery das so anbietet)

    USHORT nFilterFormat;
    Graphic aGraphic;
    GraphicFilter* pGraphicFilter = ::GetGrfFilter();

//      GraphicProgress aGraphicProgress(&aGraphicFilter);

    INetURLObject aURL;
    aURL.SetSmartURL( rFile );
    if (!pGraphicFilter->ImportGraphic(aGraphic, aURL,
            GRFILTER_FORMAT_DONTKNOW, &nFilterFormat ))
    {
        String aFltName = pGraphicFilter->GetImportFormatName(nFilterFormat);
        return PasteGraphic( rPos, aGraphic, rFile, aFltName );
    }

    if (bLink)                      // bei bLink alles, was nicht Grafik ist, als URL
    {
        Rectangle aRect( rPos, Size(0,0) );
        ScRange aRange = GetViewData()->GetDocument()->
                            GetRange( GetViewData()->GetTabNo(), aRect );
        USHORT nPosX = aRange.aStart.Col();
        USHORT nPosY = aRange.aStart.Row();

        InsertBookmark( rFile, rFile, nPosX, nPosY );
        return TRUE;
    }
    else
    {
        // 3. Kann die Datei als OLE eingefuegt werden?
        // auch nicht-Storages, z.B. Sounds (#38282#)

//      BOOL bIsStg = SvStorage::IsStorageFile( rFile );
        SvStorageRef refStor = new SvStorage( EMPTY_STRING );
#ifndef SO3
        SvObjectRef refOleObj =
            SvInPlaceObject::ClassFactory()->CreateAndInit( rFile, refStor, bLink );
#else
        SvObjectRef refOleObj =
            ((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit( rFile, refStor, bLink );
#endif
        SvInPlaceObjectRef refObj( &refOleObj );
        if( refObj.Is() )
            return PasteObject( rPos, refObj );
    }

    // Ich geb auf! Soll hier noch ne MsgBox hin?
    return FALSE;
}

BOOL ScViewFunc::PasteBookmark( SvDataObject* pObject, USHORT nPosX, USHORT nPosY )
{
    ULONG nFormat = INetBookmark::HasFormat(*pObject);
    INetBookmark aBookmark;
    BOOL bFound = aBookmark.Paste(*pObject,nFormat);
    if ( !bFound )
    {
        USHORT nCount = DragServer::GetItemCount();
        for( USHORT i = 0; i < nCount && !bFound; i++ )
            if ( aBookmark.PasteDragServer(i) )
                bFound = TRUE;
    }
    if ( !bFound )
        return FALSE;       // gibt kein Bookmark (keine Assertion -
                            //  bei Datenauszuegen vom Desktop wird FALSE zurueckgegeben)

    InsertBookmark( aBookmark.GetDescription(), aBookmark.GetURL(), nPosX, nPosY );
    return TRUE;

    //! aus DragServer ggf. noch weitere Items einfuegen !!!!
}

void ScViewFunc::InsertBookmark( const String& rDescription, const String& rURL,
                                    USHORT nPosX, USHORT nPosY, const String* pTarget )
{
    ScViewData* pViewData = GetViewData();
    if ( pViewData->HasEditView( pViewData->GetActivePart() ) &&
            nPosX >= pViewData->GetEditViewCol() && nPosX <= pViewData->GetEditEndCol() &&
            nPosY >= pViewData->GetEditViewRow() && nPosY <= pViewData->GetEditEndRow() )
    {
        //  in die gerade editierte Zelle einfuegen

        String aTargetFrame;
        if (pTarget)
            aTargetFrame = *pTarget;
        pViewData->GetViewShell()->InsertURLField( rDescription, rURL, aTargetFrame );
        return;
    }

    //  in nicht editierte Zelle einfuegen

    ScDocument* pDoc = GetViewData()->GetDocument();
    USHORT nTab = GetViewData()->GetTabNo();
    ScAddress aCellPos( nPosX, nPosY, nTab );
    ScBaseCell* pCell = pDoc->GetCell( aCellPos );
    EditEngine aEngine( pDoc->GetEnginePool() );
    if (pCell)
    {
        if (pCell->GetCellType() == CELLTYPE_EDIT)
        {
            const EditTextObject* pOld = ((ScEditCell*)pCell)->GetData();
            if (pOld)
                aEngine.SetText(*pOld);
        }
        else
        {
            String aOld;
            pDoc->GetInputString( nPosX, nPosY, nTab, aOld );
            if (aOld.Len())
                aEngine.SetText(aOld);
        }
    }

    USHORT nPara = aEngine.GetParagraphCount();
    if (nPara)
        --nPara;
    xub_StrLen nTxtLen = aEngine.GetTextLen(nPara);
    ESelection aInsSel( nPara, nTxtLen, nPara, nTxtLen );

    SvxURLField aField( rURL, rDescription, SVXURLFORMAT_APPDEFAULT );
    if (pTarget)
        aField.SetTargetFrame(*pTarget);
    aEngine.QuickInsertField( SvxFieldItem( aField ), aInsSel );

    EditTextObject* pData = aEngine.CreateTextObject();
    EnterData( nPosX, nPosY, nTab, pData );
    delete pData;
}

BOOL ScViewFunc::HasBookmarkAtCursor( SvxHyperlinkItem* pContent )
{
    ScAddress aPos( GetViewData()->GetCurX(), GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
    ScDocument* pDoc = GetViewData()->GetDocShell()->GetDocument();

    ScBaseCell* pCell = pDoc->GetCell( aPos );
    if ( pCell && pCell->GetCellType() == CELLTYPE_EDIT )
    {
        const EditTextObject* pData = ((ScEditCell*)pCell)->GetData();
        if (pData)
        {
            BOOL bField = pData->IsFieldObject();
            if (bField)
            {
                const SvxFieldItem* pFieldItem = pData->GetField();
                if (pFieldItem)
                {
                    const SvxFieldData* pField = pFieldItem->GetField();
                    if ( pField && pField->ISA(SvxURLField) )
                    {
                        if (pContent)
                        {
                            const SvxURLField* pURLField = (const SvxURLField*)pField;
                            pContent->SetName( pURLField->GetRepresentation() );
                            pContent->SetURL( pURLField->GetURL() );
                            pContent->SetTargetFrame( pURLField->GetTargetFrame() );
                        }
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}




