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

SFX_IMPL_DOCKINGWINDOW_WITHID( ScFunctionChildWindow, FID_FUNCTION_BOX )

/*************************************************************************
#*  Member:     ScFunctionChildWindow
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionChildWindow
#*
#*  Funktion:   Konstruktor der Klasse ScFunctionChildWindow
#*              Ableitung vom SfxChildWindow als "Behaelter" fuer
#*              Funktions- Fenster in Clac
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

ScFunctionChildWindow::ScFunctionChildWindow( vcl::Window* pParentP,
                                    sal_uInt16 nId,
                                    SfxBindings* pBindings,
                                    SfxChildWinInfo* pInfo ) :
    SfxChildWindow( pParentP, nId )
{
    VclPtr<ScFunctionDockWin> pWin = VclPtr<ScFunctionDockWin>::Create( pBindings, this,
                                        pParentP, ScResId( FID_FUNCTION_BOX ) );
    SetWindow(pWin);

    SetAlignment(SfxChildAlignment::RIGHT);

    pWin->Initialize( pInfo );
}

/*************************************************************************
#*  Member:     ScFunctionDockWin
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Konstruktor der Klasse ScFunctionDockWin
#*
#*  Input:      Sfx- Verknuepfungen, Fenster, Resource
#*
#*  Output:     ---
#*
#************************************************************************/

ScFunctionDockWin::ScFunctionDockWin( SfxBindings* pBindingsP,
                SfxChildWindow *pCW, vcl::Window* pParent, const ResId& rResId ) :

    SfxDockingWindow( pBindingsP, pCW, pParent, rResId ),
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

    aIdle.SetPriority(SchedulerPriority::LOWER);
    aIdle.SetIdleHdl(LINK( this, ScFunctionDockWin, TimerHdl));

    if (pCW != nullptr)
        eSfxNewAlignment=GetAlignment();
    else
        eSfxNewAlignment=SfxChildAlignment::RIGHT;
    eSfxOldAlignment=eSfxNewAlignment;
    aFiFuncDesc->SetUpdateMode(true);
    pAllFuncList=aFuncList;
    aDDFuncList->Disable();
    aDDFuncList->Hide();
    nArgs=0;
    nDockMode=0;
    bSizeFlag=false;
    aCatBox->SetDropDownLineCount(9);
    vcl::Font aFont=aFiFuncDesc->GetFont();
    aFont.SetColor(Color(COL_BLACK));
    aFiFuncDesc->SetFont(aFont);
    aFiFuncDesc->SetBackground( GetBackground() );       //! never transparent?

    Link<ListBox&,void> aLink=LINK( this, ScFunctionDockWin, SelHdl);
    aCatBox->SetSelectHdl(aLink);
    aFuncList->SetSelectHdl(aLink);
    aDDFuncList->SetSelectHdl(aLink);

    aFuncList->SetDoubleClickHdl(LINK( this, ScFunctionDockWin, SetSelectionHdl));
    aDDFuncList->SetSelectHdl(aLink);
    aInsertButton->SetClickHdl(LINK( this, ScFunctionDockWin, SetSelectionClickHdl));

    Link<ScPrivatSplit&,void> a3Link=LINK( this, ScFunctionDockWin, SetSplitHdl);
    aPrivatSplit->SetCtrModifiedHdl(a3Link);
    StartListening( *pBindingsP, true );

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
    bInit = true;
}

/*************************************************************************
#*  Member:     ScFunctionDockWin
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Destruktor der Klasse ScFunctionDockWin
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

ScFunctionDockWin::~ScFunctionDockWin()
{
    disposeOnce();
}

void ScFunctionDockWin::dispose()
{
    EndListening( GetBindings() );
    aPrivatSplit.disposeAndClear();
    aCatBox.disposeAndClear();
    aFuncList.disposeAndClear();
    aDDFuncList.disposeAndClear();
    aInsertButton.disposeAndClear();
    aFiFuncDesc.disposeAndClear();
    pAllFuncList.clear();
    SfxDockingWindow::dispose();
}

/*************************************************************************
#*  Member:     UpdateFunctionList
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Aktualisiert die Liste der Funktionen ab-
#*              haengig von der eingestellten Kategorie.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionDockWin::InitLRUList()
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
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Aktualisiert die Liste der zuletzt verwendeten Funktionen.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionDockWin::UpdateLRUList()
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
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Groesse fuer die einzelnen Controls einzustellen.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionDockWin::SetSize()
{
    sal_Int32 nSelEntry=0;
    SfxChildAlignment  aChildAlign=eSfxOldAlignment;//GetAlignment();
    short nNewDockMode;
    switch(aChildAlign)
    {
        case SfxChildAlignment::HIGHESTTOP:
        case SfxChildAlignment::TOP:
        case SfxChildAlignment::LOWESTTOP:
        case SfxChildAlignment::LOWESTBOTTOM:
        case SfxChildAlignment::BOTTOM:
        case SfxChildAlignment::TOOLBOXTOP:
        case SfxChildAlignment::TOOLBOXBOTTOM:

                        nNewDockMode=1;
                        if(nDockMode!=nNewDockMode)
                        {
                            nDockMode=nNewDockMode;
                            nSelEntry=aFuncList->GetSelectEntryPos();
                            aFuncList->Clear();
                            aFiFuncDesc->SetPosPixel(aFuncList->GetPosPixel());
                            aDDFuncList->Enable();
                            aDDFuncList->Show();
                            aPrivatSplit->Disable();
                            aPrivatSplit->Hide();
                            aFuncList->Disable();
                            aFuncList->Hide();
                            pAllFuncList=aDDFuncList;
                            SelHdl(*aCatBox.get());
                            aDDFuncList->SelectEntryPos(nSelEntry);
                        }
                        break;

        default:        nNewDockMode=0;
                        if(nDockMode!=nNewDockMode)
                        {
                            nDockMode=nNewDockMode;
                            nSelEntry=aDDFuncList->GetSelectEntryPos();
                            aDDFuncList->Clear();
                            aDDFuncList->Disable();
                            aDDFuncList->Hide();
                            aPrivatSplit->Enable();
                            aPrivatSplit->Show();
                            aFuncList->Enable();
                            aFuncList->Show();
                            pAllFuncList=aFuncList;
                            SelHdl(*aCatBox.get());
                            aFuncList->SelectEntryPos(nSelEntry);
                        }
                        break;
    }

    if(nDockMode==0)
    {
        SetLeftRightSize();
    }
    else
    {
        SetTopBottonSize();
    }
}
/*************************************************************************
#*  Member:     SetLeftRightSize
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Groesse fuer die einzelnen Controls einstellen,
#*              wenn Links oder Rechts angedockt wird.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionDockWin::SetLeftRightSize()
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
#*  Member:     SetTopBottonSize
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Groesse fuer die einzelnen Controls einzustellen.
#*              wenn oben oder unten angedockt wird.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionDockWin::SetTopBottonSize()
{
    if(!bSizeFlag)
    {
        bSizeFlag = true;
        Size aDiffSize=GetSizePixel();
        Size aNewSize=GetOutputSizePixel();
        aDiffSize.Width()-=aNewSize.Width();
        aDiffSize.Height()-=aNewSize.Height();

        SetMyWidthToBo(aNewSize);
        SetMyHeightToBo(aNewSize);

        aNewSize.Width()+=aDiffSize.Width();
        aNewSize.Height()+=aDiffSize.Height();
        bSizeFlag=false;
    }
}

/*************************************************************************
#*  Member:     SetMyWidthLeRi
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Breite fuer die einzelnen Controls und
#*              das Fenster einstellen,wenn Li oder Re
#*
#*  Input:      neue Fenstergroesse
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionDockWin::SetMyWidthLeRi(Size &aNewSize)
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
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Hoehe fuer die einzelnen Controls und
#*              das Fenster einstellen bei Li oder Re
#*
#*  Input:      neue Fenstergroesse
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionDockWin::SetMyHeightLeRi(Size &aNewSize)
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
#*  Member:     SetMyWidthToBo
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Breite fuer die einzelnen Controls und
#*              das Fenster einstellen, wenn oben oder
#*              unten angedockt werden soll.
#*
#*  Input:      neue Fenstergroesse
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionDockWin::SetMyWidthToBo(Size &aNewSize)
{
    if((sal_uLong)aNewSize.Width()<nMinWidth)   aNewSize.Width()=nMinWidth;

    Size aCDSize=aCatBox->GetSizePixel();
    Size aDdFLSize=aDDFuncList->GetSizePixel();
    Size aFDSize=aFiFuncDesc->GetSizePixel();

    Point aCDTopLeft=aCatBox->GetPosPixel();
    Point aDdFLTopLeft=aDDFuncList->GetPosPixel();
    Point aFDTopLeft=aFiFuncDesc->GetPosPixel();

    aCDSize.Width()=aDdFLTopLeft.X()-aFDTopLeft.X()-aCDTopLeft.X();
    aDdFLTopLeft.X()=aCDSize.Width()+aCDTopLeft.X()+aFDTopLeft.X();

    aDdFLSize.Width()=aNewSize.Width()-aDdFLTopLeft.X()-aFDTopLeft.X();

    aFDSize.Width()=aNewSize.Width()-2*aFDTopLeft.X();

    aDDFuncList->SetPosPixel(aDdFLTopLeft);
    aDDFuncList->SetSizePixel(aDdFLSize);
    aCatBox->SetSizePixel(aCDSize);
    aFiFuncDesc->SetSizePixel(aFDSize);
}

/*************************************************************************
#*  Member:     SetHeight
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Hoehe fuer die einzelnen Controls und
#*              das Fenster einstellen, wenn oben oder
#*              unten angedockt werden soll.
#*
#*  Input:      neue Fenstergroesse
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionDockWin::SetMyHeightToBo(Size &aNewSize)
{
    if((sal_uLong)aNewSize.Height()<nMinHeight) aNewSize.Height()=nMinHeight;

    Size aFDSize=aFiFuncDesc->GetSizePixel();

    Point aFDTopLeft=aFiFuncDesc->GetPosPixel();
    Point aCBTopLeft=aCatBox->GetPosPixel();
    aFDSize.Height()=aNewSize.Height()-aFDTopLeft.Y()-aCBTopLeft.Y();
    aFiFuncDesc->SetSizePixel(aFDSize);

}

/*************************************************************************
#*  Member:     SetDescription
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Erklaerungstext fuer die Funktion einstellen.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionDockWin::SetDescription()
{
    aFiFuncDesc->SetText( EMPTY_OUSTRING );
    const ScFuncDesc* pDesc =
             static_cast<const ScFuncDesc*>(pAllFuncList->GetEntryData(
                    pAllFuncList->GetSelectEntryPos() ));
    if (pDesc)
    {
        pDesc->initArgumentInfo();      // full argument info is needed

        OUStringBuffer aBuf(pAllFuncList->GetSelectEntry());
        if(nDockMode==0)
        {
            aBuf.append(":\n\n");
        }
        else
        {
            aBuf.append(":   ");
        }

        aBuf.append(pDesc->GetParamList());

        if(nDockMode==0)
        {
            aBuf.append("\n\n");
        }
        else
        {
            aBuf.append("\n");
        }

        aBuf.append(*pDesc->pFuncDesc);

        aFiFuncDesc->SetText(aBuf.makeStringAndClear());
        aFiFuncDesc->StateChanged(StateChangedType::Text);
        aFiFuncDesc->Invalidate();
        aFiFuncDesc->Update();

    }
 }

/// override to set new size of the controls
void ScFunctionDockWin::Resizing( Size& rNewSize )
{
    if((sal_uLong)rNewSize.Width()<nMinWidth) rNewSize.Width()=nMinWidth;
    if((sal_uLong)rNewSize.Height()<nMinHeight) rNewSize.Height()=nMinHeight;

}

/*************************************************************************
#*  Member:     Close
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Schliessen des Fensters
#*
#*  Input:      ---
#*
#*  Output:     TRUE
#*
#************************************************************************/

bool ScFunctionDockWin::Close()
{
    SfxBoolItem aItem( FID_FUNCTION_BOX, false );

    GetBindings().GetDispatcher()->Execute( FID_FUNCTION_BOX,
                                SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                                &aItem, 0L );

    SfxDockingWindow::Close();

    return true;
}

/*************************************************************************
#*  Member:     CheckAlignment
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Ueberprueft den Andockmodus und stellt die
#*              Groessen dementsprechend ein.
#*
#*  Input:      Das neue Alignment
#*
#*  Output:     Das uebergebene Alignment
#*
#************************************************************************/
SfxChildAlignment ScFunctionDockWin::CheckAlignment(SfxChildAlignment /* abla */,
                                SfxChildAlignment aChildAlign)
{
    OUString aString("ww");
    Size aTxtSize( aFiFuncDesc->GetTextWidth(aString), aFiFuncDesc->GetTextHeight() );
    if(!bInit)
    {
        eSfxOldAlignment=eSfxNewAlignment;
        eSfxNewAlignment=aChildAlign;
    }
    else
    {
        bInit=false;
        eSfxOldAlignment=aChildAlign;
        eSfxNewAlignment=aChildAlign;
    }

    switch(eSfxOldAlignment)
    {
        case SfxChildAlignment::HIGHESTTOP:
        case SfxChildAlignment::TOP:
        case SfxChildAlignment::LOWESTTOP:
        case SfxChildAlignment::LOWESTBOTTOM:
        case SfxChildAlignment::BOTTOM:
        case SfxChildAlignment::TOOLBOXTOP:
        case SfxChildAlignment::TOOLBOXBOTTOM:

                        nMinWidth= 0;
                        nMinHeight=0;

                        break;

        case SfxChildAlignment::NOALIGNMENT:

                        aString = aCatBox->GetEntry(0);
                        aString += "www";
                        aTxtSize = Size( aFiFuncDesc->GetTextWidth(aString),
                                            aFiFuncDesc->GetTextHeight() );
                        // fall-through
        default:        Point aTopLeft=aCatBox->GetPosPixel();
                        nMinWidth=aTxtSize.Width()+aTopLeft.X()
                                +2*aFuncList->GetPosPixel().X();
                        nMinHeight=19*aTxtSize.Height();

                        break;
    }

    return aChildAlign;
}
/*************************************************************************
#*  Member:     Close
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Aenderungen erkennen
#*
#*  Input:      ---
#*
#*  Output:     TRUE
#*
#************************************************************************/
void ScFunctionDockWin::Notify( SfxBroadcaster&, const SfxHint& /* rHint */ )
{
}

/// override to set new size of the controls
void ScFunctionDockWin::Resize()
{
    if ( !IsFloatingMode() ||
         !GetFloatingWindow()->IsRollUp() )
    {
        Size aQSize=GetOutputSizePixel();
        Resizing( aQSize);
        SetSize();
    }
    SfxDockingWindow::Resize();
}

/*************************************************************************
#*  Member:     UpdateFunctionList
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Aktualisiert die Liste der Funktionen ab-
#*              haengig von der eingestellten Kategorie.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

void ScFunctionDockWin::UpdateFunctionList()
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
#*  Klasse:     ScFunctionDockWin
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

void ScFunctionDockWin::DoEnter()
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
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Bei einer Aenderung der Kategorie wird die
#*              die Liste der Funktionen aktualisiert.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK_TYPED( ScFunctionDockWin, SelHdl, ListBox&, rLb, void )
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
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Bei einer Aenderung der Kategorie wird die
#*              die Liste der Funktionen aktualisiert.
#*
#*  Input:      ---
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK_NOARG_TYPED( ScFunctionDockWin, SetSelectionClickHdl, Button*, void )
{
    DoEnter();          // Uebernimmt die Eingabe
}
IMPL_LINK_NOARG_TYPED( ScFunctionDockWin, SetSelectionHdl, ListBox&, void )
{
    DoEnter();          // Uebernimmt die Eingabe
}

/*************************************************************************
#*  Handle:     SetSplitHdl
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Bei einer Aenderung des Split- Controls werden die
#*              einzelnen Controls an die neue Groesse angepasst.
#*
#*  Input:      Zeiger auf Control
#*
#*  Output:     ---
#*
#************************************************************************/

IMPL_LINK_TYPED( ScFunctionDockWin, SetSplitHdl, ScPrivatSplit&, rCtrl, void )
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

void ScFunctionDockWin::ToggleFloatingMode()
{
    aSplitterInitPos = Point();
    SfxDockingWindow::ToggleFloatingMode();

    eSfxNewAlignment=GetAlignment();
    eSfxOldAlignment=eSfxNewAlignment;

    aOldSize.Height()=0;
    aOldSize.Width()=0;
    aIdle.Start();
}

IMPL_LINK_NOARG_TYPED(ScFunctionDockWin, TimerHdl, Idle *, void)
{
    CheckAlignment(eSfxOldAlignment,eSfxNewAlignment);
    SetSize();
}

void ScFunctionDockWin::Initialize(SfxChildWinInfo *pInfo)
{
    OUString aStr;
    if(pInfo!=nullptr)
    {
        if ( !pInfo->aExtraString.isEmpty() )
        {
            sal_Int32 nPos = pInfo->aExtraString.indexOf( "ScFuncList:" );

            // Versuche, den Alignment-String "ALIGN:(...)" einzulesen; wenn
            // er nicht vorhanden ist, liegt eine "altere Version vor
            if ( nPos != -1 )
            {
                sal_Int32 n1 = pInfo->aExtraString.indexOf('(', nPos);
                if ( n1 != -1 )
                {
                    sal_Int32 n2 = pInfo->aExtraString.indexOf(')', n1);
                    if ( n2 != -1 )
                    {
                        // Alignment-String herausschneiden
                        aStr = pInfo->aExtraString.copy(nPos, n2 - nPos + 1);
                        pInfo->aExtraString = pInfo->aExtraString.replaceAt(nPos, n2 - nPos + 1, "");
                        aStr = aStr.copy( n1-nPos+1 );
                    }
                }
            }
        }
    }
    SfxDockingWindow::Initialize(pInfo);

    if ( !aStr.isEmpty())
    {
        aSplitterInitPos = aPrivatSplit->GetPosPixel();
        aSplitterInitPos.Y() = (sal_uInt16) aStr.toInt32();
        sal_Int32 n1 = aStr.indexOf(';');
        aStr = aStr.copy( n1+1 );
        sal_Int32 nSelPos = aStr.toInt32();
        aCatBox->SelectEntryPos(nSelPos);
        SelHdl(*aCatBox.get());

        //  if the window has already been shown (from SfxDockingWindow::Initialize if docked),
        //  set the splitter position now, otherwise it is set in StateChanged with type INITSHOW

        UseSplitterInitPos();
    }
}

void ScFunctionDockWin::FillInfo(SfxChildWinInfo& rInfo) const
{
    SfxDockingWindow::FillInfo(rInfo);
    Point aPoint=aPrivatSplit->GetPosPixel();
    rInfo.aExtraString += "ScFuncList:(" +
                          OUString::number(aPoint.Y()) + ";" +
                          OUString::number(aCatBox->GetSelectEntryPos()) + ")";
}

void ScFunctionDockWin::UseSplitterInitPos()
{
    if ( IsVisible() && aPrivatSplit->IsEnabled() && aSplitterInitPos != Point() )
    {
        aPrivatSplit->MoveSplitTo(aSplitterInitPos);
        aSplitterInitPos = Point();     // use only once
    }
}

void ScFunctionDockWin::StateChanged( StateChangedType nStateChange )
{
    SfxDockingWindow::StateChanged( nStateChange );

    if (nStateChange == StateChangedType::InitShow)
    {
        UseSplitterInitPos();           //  set initial splitter position if necessary
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
