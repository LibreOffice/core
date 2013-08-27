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
#include "global.hxx"       // ScAddress
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

// -----------------------------------------------------------------------

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

ScFunctionChildWindow::ScFunctionChildWindow( Window* pParentP,
                                    sal_uInt16 nId,
                                    SfxBindings* pBindings,
                                    SfxChildWinInfo* pInfo ) :
    SfxChildWindow( pParentP, nId )
{
    ScFunctionDockWin* pWin = new ScFunctionDockWin( pBindings, this,
                                        pParentP, ScResId( FID_FUNCTION_BOX ) );
    pWindow = pWin;

    eChildAlignment = SFX_ALIGN_RIGHT;

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
                SfxChildWindow *pCW, Window* pParent, const ResId& rResId ) :

    SfxDockingWindow( pBindingsP, pCW, pParent, rResId ),
    aPrivatSplit    ( this, ResId( FT_SPLIT, *rResId.GetResMgr()  ),SC_SPLIT_VERT),
    aCatBox         ( this, ResId( CB_CAT, *rResId.GetResMgr()  ) ),
    aFuncList       ( this, ResId( LB_FUNC, *rResId.GetResMgr()  ) ),
    aDDFuncList     ( this, ResId( DDLB_FUNC, *rResId.GetResMgr()  ) ),
    aInsertButton   ( this, ResId( IMB_INSERT, *rResId.GetResMgr()  ) ),
    aFiFuncDesc     ( this, ResId( FI_FUNCDESC, *rResId.GetResMgr()  ) ),
    aOldSize        (0,0)
{
    FreeResource();
    InitLRUList();
    SetStyle(GetStyle()|WB_CLIPCHILDREN);

    aTimer.SetTimeout(200);
    aTimer.SetTimeoutHdl(LINK( this, ScFunctionDockWin, TimerHdl));

    if (pCW != NULL)
        eSfxNewAlignment=GetAlignment();
    else
        eSfxNewAlignment=SFX_ALIGN_RIGHT;
    eSfxOldAlignment=eSfxNewAlignment;
    aFiFuncDesc.SetUpdateMode(sal_True);
    pAllFuncList=&aFuncList;
    aDDFuncList.Disable();
    aDDFuncList.Hide();
    nArgs=0;
    nDockMode=0;
    bSizeFlag=false;
    aCatBox.SetDropDownLineCount(9);
    Font aFont=aFiFuncDesc.GetFont();
    aFont.SetColor(Color(COL_BLACK));
    aFiFuncDesc.SetFont(aFont);
    aFiFuncDesc.SetBackground( GetBackground() );       //! never transparent?

    Link aLink=LINK( this, ScFunctionDockWin, SelHdl);
    aCatBox.SetSelectHdl(aLink);
    aFuncList.SetSelectHdl(aLink);
    aDDFuncList.SetSelectHdl(aLink);

    Link a2Link=LINK( this, ScFunctionDockWin, SetSelectionHdl);
    aFuncList.SetDoubleClickHdl(a2Link);
    aDDFuncList.SetSelectHdl(aLink);
    aInsertButton.SetClickHdl(a2Link);

    Link a3Link=LINK( this, ScFunctionDockWin, SetSplitHdl);
    aPrivatSplit.SetCtrModifiedHdl(a3Link);
    StartListening( *pBindingsP, sal_True );

    Point aTopLeft=aCatBox.GetPosPixel();
    OUString aString("ww");
    Size aTxtSize( aFiFuncDesc.GetTextWidth(aString), aFiFuncDesc.GetTextHeight() );
    nMinWidth=aTxtSize.Width()+aTopLeft.X()
            +2*aFuncList.GetPosPixel().X();
    nMinHeight=19*aTxtSize.Height();
    aCatBox.SelectEntryPos(0);

    Range aYRange(3*aTxtSize.Height()+aFuncList.GetPosPixel().Y(),
                GetOutputSizePixel().Height()-2*aTxtSize.Height());
    aPrivatSplit.SetYRange(aYRange);
    SelHdl(&aCatBox);
    bInit=sal_True;
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
    EndListening( GetBindings() );
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


    sal_uInt16  nSelPos   = aCatBox.GetSelectEntryPos();

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
    sal_uInt16 nSelEntry=0;
    SfxChildAlignment  aChildAlign=eSfxOldAlignment;//GetAlignment();
    short nNewDockMode;
    switch(aChildAlign)
    {
        case SFX_ALIGN_HIGHESTTOP:
        case SFX_ALIGN_TOP:
        case SFX_ALIGN_LOWESTTOP:
        case SFX_ALIGN_LOWESTBOTTOM:
        case SFX_ALIGN_BOTTOM:
        case SFX_ALIGN_TOOLBOXTOP:
        case SFX_ALIGN_TOOLBOXBOTTOM:


                        nNewDockMode=1;
                        if(nDockMode!=nNewDockMode)
                        {
                            nDockMode=nNewDockMode;
                            nSelEntry=aFuncList.GetSelectEntryPos();
                            aFuncList.Clear();
                            aFiFuncDesc.SetPosPixel(aFuncList.GetPosPixel());
                            aDDFuncList.Enable();
                            aDDFuncList.Show();
                            aPrivatSplit.Disable();
                            aPrivatSplit.Hide();
                            aFuncList.Disable();
                            aFuncList.Hide();
                            pAllFuncList=&aDDFuncList;
                            SelHdl(&aCatBox);
                            aDDFuncList.SelectEntryPos(nSelEntry);
                        }
                        break;

        default:        nNewDockMode=0;
                        if(nDockMode!=nNewDockMode)
                        {
                            nDockMode=nNewDockMode;
                            nSelEntry=aDDFuncList.GetSelectEntryPos();
                            aDDFuncList.Clear();
                            aDDFuncList.Disable();
                            aDDFuncList.Hide();
                            aPrivatSplit.Enable();
                            aPrivatSplit.Show();
                            aFuncList.Enable();
                            aFuncList.Show();
                            pAllFuncList=&aFuncList;
                            SelHdl(&aCatBox);
                            aFuncList.SelectEntryPos(nSelEntry);
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
    if(bSizeFlag==false)
    {
        bSizeFlag=sal_True;

        Size aDiffSize=GetSizePixel();
        Size aNewSize=GetOutputSizePixel();
        aDiffSize.Width()-=aNewSize.Width();
        aDiffSize.Height()-=aNewSize.Height();

        OUString aString("ww");
        Size aTxtSize( aFuncList.GetTextWidth(aString), aFuncList.GetTextHeight() );

        Range aYRange(3*aTxtSize.Height()+aFuncList.GetPosPixel().Y(),
                    GetOutputSizePixel().Height()-2*aTxtSize.Height());
        aPrivatSplit.SetYRange(aYRange);


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
    if(bSizeFlag==false)
    {
        bSizeFlag=sal_True;
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

    Size aCDSize=aCatBox.GetSizePixel();
    Size aFLSize=aFuncList.GetSizePixel();
    Size aSplitterSize=aPrivatSplit.GetSizePixel();
    Size aFDSize=aFiFuncDesc.GetSizePixel();

    Point aCDTopLeft=aCatBox.GetPosPixel();
    Point aFLTopLeft=aFuncList.GetPosPixel();

    aCDSize.Width()=aNewSize.Width()-aCDTopLeft.X()-aFLTopLeft.X();
    aFLSize.Width()=aNewSize.Width()-2*aFLTopLeft.X();
    aFDSize.Width()=aFLSize.Width();
    aSplitterSize.Width()=aFLSize.Width();

    aCatBox.SetSizePixel(aCDSize);
    aFuncList.SetSizePixel(aFLSize);
    aPrivatSplit.SetSizePixel(aSplitterSize);
    aFiFuncDesc.SetSizePixel(aFDSize);
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

    Size aFLSize=aFuncList.GetSizePixel();
    Size aSplitterSize=aPrivatSplit.GetSizePixel();
    Size aFDSize=aFiFuncDesc.GetSizePixel();

    Point aFLTopLeft=aFuncList.GetPosPixel();
    Point aSplitterTopLeft=aPrivatSplit.GetPosPixel();
    Point aFDTopLeft=aFiFuncDesc.GetPosPixel();

    long nTxtHeight = aFuncList.GetTextHeight();

    short nY=(short)(3*nTxtHeight+
        aFuncList.GetPosPixel().Y()+aSplitterSize.Height());

    aFDTopLeft.Y()=aNewSize.Height()-aFDSize.Height()-4;
    if(nY>aFDTopLeft.Y())
    {
        aFDSize.Height()-=nY-aFDTopLeft.Y();
        aFDTopLeft.Y()=nY;
    }
    aSplitterTopLeft.Y()=aFDTopLeft.Y()-aSplitterSize.Height()-1;
    aFLSize.Height()=aSplitterTopLeft.Y()-aFLTopLeft.Y()-1;

    aFuncList.SetSizePixel(aFLSize);
    aPrivatSplit.SetPosPixel(aSplitterTopLeft);
    aFiFuncDesc.SetPosPixel(aFDTopLeft);
    aFiFuncDesc.SetSizePixel(aFDSize);

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

    Size aCDSize=aCatBox.GetSizePixel();
    Size aDdFLSize=aDDFuncList.GetSizePixel();
    Size aFDSize=aFiFuncDesc.GetSizePixel();

    Point aCDTopLeft=aCatBox.GetPosPixel();
    Point aDdFLTopLeft=aDDFuncList.GetPosPixel();
    Point aFDTopLeft=aFiFuncDesc.GetPosPixel();

    aCDSize.Width()=aDdFLTopLeft.X()-aFDTopLeft.X()-aCDTopLeft.X();
    aDdFLTopLeft.X()=aCDSize.Width()+aCDTopLeft.X()+aFDTopLeft.X();

    aDdFLSize.Width()=aNewSize.Width()-aDdFLTopLeft.X()-aFDTopLeft.X();

    aFDSize.Width()=aNewSize.Width()-2*aFDTopLeft.X();

    aDDFuncList.SetPosPixel(aDdFLTopLeft);
    aDDFuncList.SetSizePixel(aDdFLSize);
    aCatBox.SetSizePixel(aCDSize);
    aFiFuncDesc.SetSizePixel(aFDSize);
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

    Size aFDSize=aFiFuncDesc.GetSizePixel();

    Point aFDTopLeft=aFiFuncDesc.GetPosPixel();
    Point aCBTopLeft=aCatBox.GetPosPixel();
    aFDSize.Height()=aNewSize.Height()-aFDTopLeft.Y()-aCBTopLeft.Y();
    aFiFuncDesc.SetSizePixel(aFDSize);

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
    aFiFuncDesc.SetText( EMPTY_STRING );
    const ScFuncDesc* pDesc =
             (const ScFuncDesc*)pAllFuncList->GetEntryData(
                    pAllFuncList->GetSelectEntryPos() );
    if (pDesc)
    {
        pDesc->initArgumentInfo();      // full argument info is needed

        OUStringBuffer aBuf(pAllFuncList->GetSelectEntry());
        if(nDockMode==0)
        {
            aBuf.appendAscii(":\n\n");
        }
        else
        {
            aBuf.appendAscii(":   ");
        }

        aBuf.append(pDesc->GetParamList());

        if(nDockMode==0)
        {
            aBuf.appendAscii("\n\n");
        }
        else
        {
            aBuf.appendAscii("\n");
        }

        aBuf.append(*pDesc->pFuncDesc);

        aFiFuncDesc.SetText(aBuf.makeStringAndClear());
        aFiFuncDesc.StateChanged(STATE_CHANGE_TEXT);
        aFiFuncDesc.Invalidate();
        aFiFuncDesc.Update();

    }
 }

/*************************************************************************
#*  Member:     Resizing
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Ueberladene Funktion um die Groesse der
#*              einzelnen Controls einzustellen.
#*
#*  Input:      neue Groesse
#*
#*  Output:     ---
#*
#************************************************************************/

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

sal_Bool ScFunctionDockWin::Close()
{
    SfxBoolItem aItem( FID_FUNCTION_BOX, false );

    GetBindings().GetDispatcher()->Execute( FID_FUNCTION_BOX,
                                SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                &aItem, 0L );

    SfxDockingWindow::Close();

    return( sal_True );
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
    String aString = OUString("ww");
    Size aTxtSize( aFiFuncDesc.GetTextWidth(aString), aFiFuncDesc.GetTextHeight() );
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
        case SFX_ALIGN_HIGHESTTOP:
        case SFX_ALIGN_TOP:
        case SFX_ALIGN_LOWESTTOP:
        case SFX_ALIGN_LOWESTBOTTOM:
        case SFX_ALIGN_BOTTOM:
        case SFX_ALIGN_TOOLBOXTOP:
        case SFX_ALIGN_TOOLBOXBOTTOM:

                        nMinWidth= 0;
                        nMinHeight=0;

                        break;

        case SFX_ALIGN_NOALIGNMENT:

                        aString = aCatBox.GetEntry(0);
                        aString.AppendAscii(RTL_CONSTASCII_STRINGPARAM("www"));
                        aTxtSize = Size( aFiFuncDesc.GetTextWidth(aString),
                                            aFiFuncDesc.GetTextHeight() );

        default:        Point aTopLeft=aCatBox.GetPosPixel();
                        nMinWidth=aTxtSize.Width()+aTopLeft.X()
                                +2*aFuncList.GetPosPixel().X();
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


/*************************************************************************
#*  Member:     Resize
#*------------------------------------------------------------------------
#*
#*  Klasse:     ScFunctionDockWin
#*
#*  Funktion:   Ueberladene Funktion um die Groesse der
#*              einzelnen Controls einzustellen.
#*
#*  Input:      neue Groesse
#*
#*  Output:     ---
#*
#************************************************************************/

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
    sal_uInt16  nSelPos   = aCatBox.GetSelectEntryPos();
    sal_uInt16  nCategory = ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
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
                (void*)pDesc );
            pDesc = pFuncMgr->Next();
        }
    }
    else // LRU-Liste
    {
        for(::std::vector<const formula::IFunctionDescription*>::iterator iter=aLRUList.begin();iter!=aLRUList.end();++iter)
        {
            const formula::IFunctionDescription* pDesc = *iter;
            pAllFuncList->SetEntryData(
                    pAllFuncList->InsertEntry(pDesc->getFunctionName()),
                    (void*)pDesc );
        }
    }


    //------------------------------------------------------
    pAllFuncList->SetUpdateMode( sal_True );

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

void ScFunctionDockWin::DoEnter(sal_Bool /* bOk */) //@@ ???
{
    String aFirstArgStr;
    String aArgStr;
    String aString=pAllFuncList->GetSelectEntry();
    SfxViewShell* pCurSh = SfxViewShell::Current();
    nArgs=0;

    if(aString.Len()>0)
    {

        ScModule* pScMod = SC_MOD();
        ScTabViewShell* pViewSh = PTR_CAST( ScTabViewShell, pCurSh);
        ScInputHandler* pHdl = pScMod->GetInputHdl( pViewSh );
        if(!pScMod->IsEditMode())
        {
            pScMod->SetInputMode(SC_INPUT_TABLE);
            aString = '=';
            aString += pAllFuncList->GetSelectEntry();
            if (pHdl)
                pHdl->ClearText();
        }
        const ScFuncDesc* pDesc =
             (const ScFuncDesc*)pAllFuncList->GetEntryData(
                    pAllFuncList->GetSelectEntryPos() );
        if (pDesc)
        {
            pFuncDesc=pDesc;
            UpdateLRUList();
            nArgs = pDesc->nArgCount;
            if(nArgs>0)
            {
                // NOTE: Theoretically the first parameter could have the
                // suppress flag as well, but practically it doesn't.
                aFirstArgStr = *(pDesc->ppDefArgNames[0]);
                aFirstArgStr = comphelper::string::strip(aFirstArgStr, ' ');
                aFirstArgStr.SearchAndReplaceAll(' ', '_');
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
                            String sTmp(*(pDesc->ppDefArgNames[nArg]));
                            sTmp = comphelper::string::strip(sTmp, ' ');
                            sTmp.SearchAndReplaceAll(' ', '_');
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
                aString = '=';
                aString += pAllFuncList->GetSelectEntry();
            }
            EditView *pEdView=pHdl->GetActiveView();
            if(pEdView!=NULL) // @ Wegen Absturz bei Namen festlegen
            {
                if(nArgs>0)
                {
                    pHdl->InsertFunction(aString);
                    pEdView->InsertText(aArgStr,sal_True);
                    ESelection  aESel=pEdView->GetSelection();
                    aESel.nEndPos=aESel.nStartPos+aFirstArgStr.Len();
                    pEdView->SetSelection(aESel);
                    pHdl->DataChanged();
                }
                else
                {
                    aString.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "()" ));
                    pEdView->InsertText(aString,false);
                    pHdl->DataChanged();
                }
            }
        }
        InitLRUList();
    }
    if ( pCurSh )
    {
        Window* pShellWnd = pCurSh->GetWindow();

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

IMPL_LINK( ScFunctionDockWin, SelHdl, ListBox*, pLb )
{
    if ( pLb == &aCatBox)
    {
        UpdateFunctionList();
        SetDescription();
    }

    if ( pLb == &aFuncList||pLb == &aDDFuncList)
    {
        SetDescription();
    }


    //SetSize();
    return 0;
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

IMPL_LINK( ScFunctionDockWin, SetSelectionHdl, void*, pCtrl )
{
    if ((ImageButton *)pCtrl == &aInsertButton ||
        (ListBox *)pCtrl == &aFuncList)
    {
        DoEnter(sal_True);          // Uebernimmt die Eingabe
    }
    //...

    return 0;
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

IMPL_LINK( ScFunctionDockWin, SetSplitHdl, ScPrivatSplit*, pCtrl )
{
    if (pCtrl == &aPrivatSplit)
    {
        short nDeltaY=aPrivatSplit.GetDeltaY();
        Size aFLSize=aFuncList.GetSizePixel();
        Size aFDSize=aFiFuncDesc.GetSizePixel();
        Point aFDTopLeft=aFiFuncDesc.GetPosPixel();

        aFLSize.Height()+=nDeltaY;
        aFDSize.Height()-=nDeltaY;
        aFDTopLeft.Y()+=nDeltaY;
        aFuncList.SetSizePixel(aFLSize);
        aFiFuncDesc.SetPosPixel(aFDTopLeft);
        aFiFuncDesc.SetSizePixel(aFDSize);
    }
    //...

    return 0;
}

void ScFunctionDockWin::ToggleFloatingMode()
{
    aSplitterInitPos = Point();
    SfxDockingWindow::ToggleFloatingMode();

    eSfxNewAlignment=GetAlignment();
    eSfxOldAlignment=eSfxNewAlignment;

    aOldSize.Height()=0;
    aOldSize.Width()=0;
    aTimer.Start();
}

IMPL_LINK_NOARG(ScFunctionDockWin, TimerHdl)
{
    CheckAlignment(eSfxOldAlignment,eSfxNewAlignment);
    SetSize();
    return 0;
}

void ScFunctionDockWin::Initialize(SfxChildWinInfo *pInfo)
{
    OUString aStr;
    if(pInfo!=NULL)
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
        aSplitterInitPos = aPrivatSplit.GetPosPixel();
        aSplitterInitPos.Y() = (sal_uInt16) aStr.toInt32();
        sal_Int32 n1 = aStr.indexOf(';');
        aStr = aStr.copy( n1+1 );
        sal_uInt16 nSelPos = sal::static_int_cast<sal_uInt16>( aStr.toInt32() );
        aCatBox.SelectEntryPos(nSelPos);
        SelHdl(&aCatBox);

        //  if the window has already been shown (from SfxDockingWindow::Initialize if docked),
        //  set the splitter position now, otherwise it is set in StateChanged with type INITSHOW

        UseSplitterInitPos();
    }
}

//-------------------------------------------------------------------------

void ScFunctionDockWin::FillInfo(SfxChildWinInfo& rInfo) const
{
    SfxDockingWindow::FillInfo(rInfo);
    Point aPoint=aPrivatSplit.GetPosPixel();
    rInfo.aExtraString += "ScFuncList:(" +
                          OUString::number(aPoint.Y()) + ";" +
                          OUString::number(aCatBox.GetSelectEntryPos()) + ")";
}

void ScFunctionDockWin::UseSplitterInitPos()
{
    if ( IsVisible() && aPrivatSplit.IsEnabled() && aSplitterInitPos != Point() )
    {
        aPrivatSplit.MoveSplitTo(aSplitterInitPos);
        aSplitterInitPos = Point();     // use only once
    }
}

void ScFunctionDockWin::StateChanged( StateChangedType nStateChange )
{
    SfxDockingWindow::StateChanged( nStateChange );

    if (nStateChange == STATE_CHANGE_INITSHOW)
    {
        UseSplitterInitPos();           //  set initial splitter position if necessary
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
