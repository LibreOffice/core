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

#include <comphelper/string.hxx>
#include <editeng/editview.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>

#include "sc.hrc"
#include "global.hxx"
#include "scresid.hxx"
#include "reffact.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "tabvwsh.hxx"
#include "appoptio.hxx"
#include "compiler.hxx"

#include "dwfunctr.hrc"
#include "dwfunctr.hxx"

/*************************************************************************
#*  Member:     ScFunctionWin
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionWin
#*
#*  Funktion:   Konstruktor der Klasse ScFunctionWin
#*
#*  Input:      Sfx- Verknuepfungen, Fenster, Resource
#*
#*  Output:     ---
#*
#************************************************************************/

ScFunctionWin::ScFunctionWin( SfxBindings* pBindingsP, vcl::Window* pParent, const ResId& rResId ) :
    vcl::Window(pParent, rResId),
    rBindings   ( *pBindingsP ),
    aIdle       ( "sc formdlg ScFunctionWin" ),
    aPrivatSplit    ( VclPtr<ScPrivatSplit>::Create( this, ResId( FT_SPLIT, *rResId.GetResMgr() ) ) ),
    aCatBox         ( VclPtr<ListBox>::Create( this, ResId( CB_CAT, *rResId.GetResMgr() ) ) ),
    aFuncList       ( VclPtr<ListBox>::Create( this, ResId( LB_FUNC, *rResId.GetResMgr() ) ) ),
    aDDFuncList     ( VclPtr<ListBox>::Create( this, ResId( DDLB_FUNC, *rResId.GetResMgr() ) ) ),
    aInsertButton   ( VclPtr<ImageButton>::Create( this, ResId( IMB_INSERT, *rResId.GetResMgr() ) ) ),
    aFiFuncDesc     ( VclPtr<FixedText>::Create( this, ResId( FI_FUNCDESC, *rResId.GetResMgr() ) ) ),
    aOldSize        (0,0),
    pFuncDesc       (nullptr)
{
    FreeResource();
    InitLRUList();
    SetStyle(GetStyle()|WB_CLIPCHILDREN);

    aIdle.SetIdleHdl(LINK( this, ScFunctionWin, TimerHdl));

    aFiFuncDesc->SetUpdateMode(true);
    pAllFuncList=aFuncList;
    aDDFuncList->Disable();
    aDDFuncList->Hide();
    nArgs=0;
    bSizeFlag=false;
    aCatBox->SetDropDownLineCount(9);
    vcl::Font aFont=aFiFuncDesc->GetFont();
    aFont.SetColor(Color(COL_BLACK));
    aFiFuncDesc->SetFont(aFont);
    aFiFuncDesc->SetBackground( GetBackground() );       //! never transparent?

    Link<ListBox&,void> aLink=LINK( this, ScFunctionWin, SelHdl);
    aCatBox->SetSelectHdl(aLink);
    aFuncList->SetSelectHdl(aLink);
    aDDFuncList->SetSelectHdl(aLink);

    aFuncList->SetDoubleClickHdl(LINK( this, ScFunctionWin, SetSelectionHdl));
    aDDFuncList->SetSelectHdl(aLink);
    aInsertButton->SetClickHdl(LINK( this, ScFunctionWin, SetSelectionClickHdl));

    Link<ScPrivatSplit&,void> a3Link=LINK( this, ScFunctionWin, SetSplitHdl);
    aPrivatSplit->SetCtrModifiedHdl(a3Link);
    StartListening( rBindings, true );

    Point aTopLeft=aCatBox->GetPosPixel();
    OUString aString("ww");
    Size aTxtSize( aFiFuncDesc->GetTextWidth(aString), aFiFuncDesc->GetTextHeight() );
    nMinWidth=aTxtSize.Width()+aTopLeft.X()
            +2*aFuncList->GetPosPixel().X();
    nMinHeight=19*aTxtSize.Height();
    aCatBox->SelectEntryPos(0);

    Range aYRange(3*aTxtSize.Height()+aFuncList->GetPosPixel().Y(),
                GetOutputSizePixel().Height()-2*aTxtSize.Height());
    aPrivatSplit->SetYRange(aYRange);
    SelHdl(*aCatBox.get());
}

/*************************************************************************
#*  Member:     ScFunctionWin
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionWin
#*
#*  Funktion:   Destruktor der Klasse ScFunctionWin
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

ScFunctionWin::~ScFunctionWin()
{
    disposeOnce();
}

void ScFunctionWin::dispose()
{
    EndListening( rBindings );
    aPrivatSplit.disposeAndClear();
    aCatBox.disposeAndClear();
    aFuncList.disposeAndClear();
    aDDFuncList.disposeAndClear();
    aInsertButton.disposeAndClear();
    aFiFuncDesc.disposeAndClear();
    pAllFuncList.clear();
    vcl::Window::dispose();
}

/*************************************************************************
#*  Member:     UpdateFunctionList
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionWin
#*
#*  Funktion:   Aktualisiert die Liste der Funktionen ab-
#*              haengig von der eingestellten Kategorie.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionWin::InitLRUList()
{
    ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();
    pFuncMgr->fillLastRecentlyUsedFunctions(aLRUList);

    sal_Int32  nSelPos   = aCatBox->GetSelectEntryPos();

    if(nSelPos == 0)
        UpdateFunctionList();
}

/*************************************************************************
#*  Member:     UpdateFunctionList
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionWin
#*
#*  Funktion:   Aktualisiert die Liste der zuletzt verwendeten Funktionen.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionWin::UpdateLRUList()
{
    if (pFuncDesc && pFuncDesc->nFIndex!=0)
    {
        ScModule* pScMod = SC_MOD();
        pScMod->InsertEntryToLRUList(pFuncDesc->nFIndex);
    }
}

/*************************************************************************
#*  Member:     SetSize
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionWin
#*
#*  Funktion:   Groesse fuer die einzelnen Controls einzustellen.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionWin::SetSize()
{
    SetLeftRightSize();
}

/*************************************************************************
#*  Member:     SetLeftRightSize
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionWin
#*
#*  Funktion:   Groesse fuer die einzelnen Controls einstellen,
#*              wenn Links oder Rechts angedockt wird.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionWin::SetLeftRightSize()
{
    if(!bSizeFlag)
    {
        bSizeFlag = true;

        Size aDiffSize=GetSizePixel();
        Size aNewSize=GetOutputSizePixel();
        aDiffSize.Width()-=aNewSize.Width();
        aDiffSize.Height()-=aNewSize.Height();

        OUString aString("ww");
        Size aTxtSize( aFuncList->GetTextWidth(aString), aFuncList->GetTextHeight() );

        Range aYRange(3*aTxtSize.Height()+aFuncList->GetPosPixel().Y(),
                    GetOutputSizePixel().Height()-2*aTxtSize.Height());
        aPrivatSplit->SetYRange(aYRange);

        if(aOldSize.Width()!=aNewSize.Width())
            SetMyWidthLeRi(aNewSize);

        if(aOldSize.Height()!=aNewSize.Height())
            SetMyHeightLeRi(aNewSize);

        aOldSize=aNewSize;
        aNewSize.Width()+=aDiffSize.Width();
        aNewSize.Height()+=aDiffSize.Height();
        bSizeFlag=false;
    }

}

/*************************************************************************
#*  Member:     SetMyWidthLeRi
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionWin
#*
#*  Funktion:   Breite fuer die einzelnen Controls und
#*              das Fenster einstellen,wenn Li oder Re
#*
#*  Input:      neue Fenstergroesse
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionWin::SetMyWidthLeRi(Size &aNewSize)
{
    if((sal_uLong)aNewSize.Width()<nMinWidth)   aNewSize.Width()=nMinWidth;

    Size aCDSize=aCatBox->GetSizePixel();
    Size aFLSize=aFuncList->GetSizePixel();
    Size aSplitterSize=aPrivatSplit->GetSizePixel();
    Size aFDSize=aFiFuncDesc->GetSizePixel();

    Point aCDTopLeft=aCatBox->GetPosPixel();
    Point aFLTopLeft=aFuncList->GetPosPixel();

    aCDSize.Width()=aNewSize.Width()-aCDTopLeft.X()-aFLTopLeft.X();
    aFLSize.Width()=aNewSize.Width()-2*aFLTopLeft.X();
    aFDSize.Width()=aFLSize.Width();
    aSplitterSize.Width()=aFLSize.Width();

    aCatBox->SetSizePixel(aCDSize);
    aFuncList->SetSizePixel(aFLSize);
    aPrivatSplit->SetSizePixel(aSplitterSize);
    aFiFuncDesc->SetSizePixel(aFDSize);
}

/*************************************************************************
#*  Member:     SetHeight
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionWin
#*
#*  Funktion:   Hoehe fuer die einzelnen Controls und
#*              das Fenster einstellen bei Li oder Re
#*
#*  Input:      neue Fenstergroesse
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionWin::SetMyHeightLeRi(Size &aNewSize)
{
    if((sal_uLong)aNewSize.Height()<nMinHeight) aNewSize.Height()=nMinHeight;

    Size aFLSize=aFuncList->GetSizePixel();
    Size aSplitterSize=aPrivatSplit->GetSizePixel();
    Size aFDSize=aFiFuncDesc->GetSizePixel();

    Point aFLTopLeft=aFuncList->GetPosPixel();
    Point aSplitterTopLeft=aPrivatSplit->GetPosPixel();
    Point aFDTopLeft=aFiFuncDesc->GetPosPixel();

    long nTxtHeight = aFuncList->GetTextHeight();

    short nY=(short)(3*nTxtHeight+
        aFuncList->GetPosPixel().Y()+aSplitterSize.Height());

    aFDTopLeft.Y()=aNewSize.Height()-aFDSize.Height()-4;
    if(nY>aFDTopLeft.Y())
    {
        aFDSize.Height()-=nY-aFDTopLeft.Y();
        aFDTopLeft.Y()=nY;
    }
    aSplitterTopLeft.Y()=aFDTopLeft.Y()-aSplitterSize.Height()-1;
    aFLSize.Height()=aSplitterTopLeft.Y()-aFLTopLeft.Y()-1;

    aFuncList->SetSizePixel(aFLSize);
    aPrivatSplit->SetPosPixel(aSplitterTopLeft);
    aFiFuncDesc->SetPosPixel(aFDTopLeft);
    aFiFuncDesc->SetSizePixel(aFDSize);

}

/*************************************************************************
#*  Member:     SetDescription
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionWin
#*
#*  Funktion:   Erklaerungstext fuer die Funktion einstellen.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionWin::SetDescription()
{
    aFiFuncDesc->SetText( EMPTY_OUSTRING );
    const ScFuncDesc* pDesc =
             static_cast<const ScFuncDesc*>(pAllFuncList->GetEntryData(
                    pAllFuncList->GetSelectEntryPos() ));
    if (pDesc)
    {
        pDesc->initArgumentInfo();      // full argument info is needed

        OUStringBuffer aBuf(pAllFuncList->GetSelectEntry());
        aBuf.append(":\n\n");
        aBuf.append(pDesc->GetParamList());
        aBuf.append("\n\n");
        aBuf.append(*pDesc->pFuncDesc);

        aFiFuncDesc->SetText(aBuf.makeStringAndClear());
        aFiFuncDesc->StateChanged(StateChangedType::Text);
        aFiFuncDesc->Invalidate();
        aFiFuncDesc->Update();

    }
 }

/*************************************************************************
#*  Member:     Close
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionWin
#*
#*  Funktion:   Aenderungen erkennen
#*
#*  Input:      ---
#*
#*  Output:     TRUE
#*
#************************************************************************/
void ScFunctionWin::Notify( SfxBroadcaster&, const SfxHint& /* rHint */ )
{
}

/// override to set new size of the controls
void ScFunctionWin::Resize()
{
    SetSize();
    vcl::Window::Resize();
}

/*************************************************************************
#*  Member:     UpdateFunctionList
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionWin
#*
#*  Funktion:   Aktualisiert die Liste der Funktionen ab-
#*              haengig von der eingestellten Kategorie.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionWin::UpdateFunctionList()
{
    sal_Int32  nSelPos   = aCatBox->GetSelectEntryPos();
    sal_Int32  nCategory = ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
                            ? (nSelPos-1) : 0;

    pAllFuncList->Clear();
    pAllFuncList->SetUpdateMode( false );

    if ( nSelPos > 0 )
    {
        ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();

        const ScFuncDesc* pDesc = pFuncMgr->First( nCategory );
        while ( pDesc )
        {
            pAllFuncList->SetEntryData(
                pAllFuncList->InsertEntry( *(pDesc->pFuncName) ),
                const_cast<ScFuncDesc *>(pDesc) );
            pDesc = pFuncMgr->Next();
        }
    }
    else // LRU-Liste
    {
        for (::std::vector<const formula::IFunctionDescription*>::iterator iter=aLRUList.begin();
                iter != aLRUList.end(); ++iter)
        {
            const formula::IFunctionDescription* pDesc = *iter;
            if (pDesc)
                pAllFuncList->SetEntryData( pAllFuncList->InsertEntry( pDesc->getFunctionName()), const_cast<formula::IFunctionDescription *>(pDesc));
        }
    }

    pAllFuncList->SetUpdateMode( true );

    if ( pAllFuncList->GetEntryCount() > 0 )
    {
        pAllFuncList->Enable();
        pAllFuncList->SelectEntryPos( 0 );
    }
    else
    {
        pAllFuncList->Disable();
    }
}

/*************************************************************************
#*  Member:     DoEnter
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionWin
#*
#*  Funktion:   Eingabe ins Dokument uebernehmen. Wird aufgerufen
#*              nach betaetigen der Uebernehmen- Schaltflaeche
#*              oder einem Doppelklick in die Funktionsliste.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionWin::DoEnter()
{
    OUString aFirstArgStr;
    OUString aArgStr;
    OUString aString=pAllFuncList->GetSelectEntry();
    SfxViewShell* pCurSh = SfxViewShell::Current();
    nArgs=0;

    if(!aString.isEmpty())
    {

        ScModule* pScMod = SC_MOD();
        ScTabViewShell* pViewSh = dynamic_cast<ScTabViewShell*>( pCurSh );
        ScInputHandler* pHdl = pScMod->GetInputHdl( pViewSh );
        if(!pScMod->IsEditMode())
        {
            pScMod->SetInputMode(SC_INPUT_TABLE);
            // the above call can result in us being disposed
            if (OutputDevice::isDisposed())
                return;
            aString = "=";
            aString += pAllFuncList->GetSelectEntry();
            if (pHdl)
                pHdl->ClearText();
        }
        const ScFuncDesc* pDesc =
             static_cast<const ScFuncDesc*>(pAllFuncList->GetEntryData(
                    pAllFuncList->GetSelectEntryPos() ));
        if (pDesc)
        {
            pFuncDesc=pDesc;
            UpdateLRUList();
            nArgs = pDesc->nArgCount;
            if(nArgs>0)
            {
                // NOTE: Theoretically the first parameter could have the
                // suppress flag as well, but practically it doesn't.
                aFirstArgStr = pDesc->maDefArgNames[0];
                aFirstArgStr = comphelper::string::strip(aFirstArgStr, ' ');
                aFirstArgStr = aFirstArgStr.replaceAll(" ", "_");
                aArgStr = aFirstArgStr;
                if ( nArgs != VAR_ARGS && nArgs != PAIRED_VAR_ARGS )
                {   // no VarArgs or Fix plus VarArgs, but not VarArgs only
                    OUString aArgSep("; ");
                    sal_uInt16 nFix;
                    if (nArgs >= PAIRED_VAR_ARGS)
                        nFix = nArgs - PAIRED_VAR_ARGS + 2;
                    else if (nArgs >= VAR_ARGS)
                        nFix = nArgs - VAR_ARGS + 1;
                    else
                        nFix = nArgs;
                    for ( sal_uInt16 nArg = 1;
                            nArg < nFix && !pDesc->pDefArgFlags[nArg].bOptional; nArg++ )
                    {
                        if (!pDesc->pDefArgFlags[nArg].bSuppress)
                        {
                            aArgStr += aArgSep;
                            OUString sTmp = pDesc->maDefArgNames[nArg];
                            sTmp = comphelper::string::strip(sTmp, ' ');
                            sTmp = sTmp.replaceAll(" ", "_");
                            aArgStr += sTmp;
                        }
                    }
                }
            }
        }
        if (pHdl)
        {
            if (pHdl->GetEditString().isEmpty())
            {
                aString = "=";
                aString += pAllFuncList->GetSelectEntry();
            }
            EditView *pEdView=pHdl->GetActiveView();
            if(pEdView!=nullptr) // @ Wegen Absturz bei Namen festlegen
            {
                if(nArgs>0)
                {
                    pHdl->InsertFunction(aString);
                    pEdView->InsertText(aArgStr,true);
                    ESelection  aESel=pEdView->GetSelection();
                    aESel.nEndPos = aESel.nStartPos + aFirstArgStr.getLength();
                    pEdView->SetSelection(aESel);
                    pHdl->DataChanged();
                }
                else
                {
                    aString += "()";
                    pEdView->InsertText(aString);
                    pHdl->DataChanged();
                }
            }
        }
        InitLRUList();
    }
    if ( pCurSh )
    {
        vcl::Window* pShellWnd = pCurSh->GetWindow();

        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }

}

/*************************************************************************
#*  Handle:     SelHdl
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionWin
#*
#*  Funktion:   Bei einer Aenderung der Kategorie wird die
#*              die Liste der Funktionen aktualisiert.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK_TYPED( ScFunctionWin, SelHdl, ListBox&, rLb, void )
{
    if ( &rLb == aCatBox.get() )
    {
        UpdateFunctionList();
        SetDescription();
    }

    if ( &rLb == aFuncList.get() || &rLb == aDDFuncList.get() )
    {
        SetDescription();
    }
}

/*************************************************************************
#*  Handle:     SelHdl
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionWin
#*
#*  Funktion:   Bei einer Aenderung der Kategorie wird die
#*              die Liste der Funktionen aktualisiert.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK_NOARG_TYPED( ScFunctionWin, SetSelectionClickHdl, Button*, void )
{
    DoEnter();          // Uebernimmt die Eingabe
}
IMPL_LINK_NOARG_TYPED( ScFunctionWin, SetSelectionHdl, ListBox&, void )
{
    DoEnter();          // Uebernimmt die Eingabe
}

/*************************************************************************
#*  Handle:     SetSplitHdl
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionWin
#*
#*  Funktion:   Bei einer Aenderung des Split- Controls werden die
#*              einzelnen Controls an die neue Groesse angepasst.
#*
#*  Input:      Zeiger auf Control
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK_TYPED( ScFunctionWin, SetSplitHdl, ScPrivatSplit&, rCtrl, void )
{
    if (&rCtrl == aPrivatSplit.get())
    {
        short nDeltaY=aPrivatSplit->GetDeltaY();
        Size aFLSize=aFuncList->GetSizePixel();
        Size aFDSize=aFiFuncDesc->GetSizePixel();
        Point aFDTopLeft=aFiFuncDesc->GetPosPixel();

        aFLSize.Height()+=nDeltaY;
        aFDSize.Height()-=nDeltaY;
        aFDTopLeft.Y()+=nDeltaY;
        aFuncList->SetSizePixel(aFLSize);
        aFiFuncDesc->SetPosPixel(aFDTopLeft);
        aFiFuncDesc->SetSizePixel(aFDSize);
    }
}

IMPL_LINK_NOARG_TYPED(ScFunctionWin, TimerHdl, Idle *, void)
{
    OUString aString("ww");
    Size aTxtSize( aFiFuncDesc->GetTextWidth(aString), aFiFuncDesc->GetTextHeight() );
    Point aTopLeft=aCatBox->GetPosPixel();
    nMinWidth=aTxtSize.Width()+aTopLeft.X() +2*aFuncList->GetPosPixel().X();
    nMinHeight=19*aTxtSize.Height();
    SetSize();
}

void ScFunctionWin::UseSplitterInitPos()
{
    if ( IsVisible() && aPrivatSplit->IsEnabled() && aSplitterInitPos != Point() )
    {
        aPrivatSplit->MoveSplitTo(aSplitterInitPos);
        aSplitterInitPos = Point();     // use only once
    }
}

void ScFunctionWin::StateChanged( StateChangedType nStateChange )
{
    vcl::Window::StateChanged( nStateChange );

    if (nStateChange == StateChangedType::InitShow)
    {
        UseSplitterInitPos();           //  set initial splitter position if necessary
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
