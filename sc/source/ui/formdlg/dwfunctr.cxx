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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

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
#include "cell.hxx"
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "tabvwsh.hxx"
#include "appoptio.hxx"
#include "compiler.hxx"

#include "dwfunctr.hrc"
#include "dwfunctr.hxx"

// -----------------------------------------------------------------------

#define ARG_SEPERATOR String("; ")
SFX_IMPL_DOCKINGWINDOW( ScFunctionChildWindow, FID_FUNCTION_BOX )

/*************************************************************************
#*  Member:     ScFunctionChildWindow                       Datum:06.10.97
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

__EXPORT ScFunctionChildWindow::ScFunctionChildWindow( Window* pParentP,
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
#*  Member:     ScFunctionDockWin                           Datum:06.10.97
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

    eSfxNewAlignment=GetAlignment();
    eSfxOldAlignment=eSfxNewAlignment;
    aFiFuncDesc.SetUpdateMode(sal_True);
    pAllFuncList=&aFuncList;
    aDDFuncList.Disable();
    aDDFuncList.Hide();
    nArgs=0;
    nDockMode=0;
    bSizeFlag=sal_False;
    aCatBox.SetDropDownLineCount(9);
    Font aFont=aFiFuncDesc.GetFont();
    aFont.SetColor(Color(COL_BLACK));
    aFiFuncDesc.SetFont(aFont);
    aFiFuncDesc.SetBackground( GetBackground() );       //! never transparent?
//? SetBackground();

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
    //String aString=aCatBox.GetEntry( 0)+String("www");
    String aString=String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("ww"));
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
#*  Member:     ScFunctionDockWin                           Datum:06.10.97
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

__EXPORT ScFunctionDockWin::~ScFunctionDockWin()
{
    EndListening( GetBindings() );
}

/*************************************************************************
#*  Member:     UpdateFunctionList                          Datum:06.10.97
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
    const ScAppOptions& rAppOpt = SC_MOD()->GetAppOptions();
    sal_uInt16 nLRUFuncCount = Min( rAppOpt.GetLRUFuncListCount(), (sal_uInt16)LRU_MAX );
    sal_uInt16* pLRUListIds = rAppOpt.GetLRUFuncList();

    sal_uInt16 i;
    for ( i=0; i<LRU_MAX; i++ )
        aLRUList[i] = NULL;

    if ( pLRUListIds )
    {
        ScFunctionMgr* pFuncMgr = ScGlobal::GetStarCalcFunctionMgr();
        for ( i=0; i<nLRUFuncCount; i++ )
            aLRUList[i] = pFuncMgr->Get( pLRUListIds[i] );
    }

    sal_uInt16  nSelPos   = aCatBox.GetSelectEntryPos();

    if(nSelPos == 0)
        UpdateFunctionList();
}

/*************************************************************************
#*  Member:     UpdateFunctionList                          Datum:10.12.99
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
#*  Member:     SetSize                                     Datum:06.10.97
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
#*  Member:     SetLeftRightSize                            Datum:15.10.97
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
    if(bSizeFlag==sal_False)
    {
        bSizeFlag=sal_True;

        Size aDiffSize=GetSizePixel();
        Size aNewSize=GetOutputSizePixel();
        aDiffSize.Width()-=aNewSize.Width();
        aDiffSize.Height()-=aNewSize.Height();

        //@ SetUpdateMode( sal_False);

        String aString = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("ww"));

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
        //SetSizePixel(aNewSize);
        //@ SetUpdateMode( sal_True);
        bSizeFlag=sal_False;
    }

}
/*************************************************************************
#*  Member:     SetTopBottonSize                            Datum:15.10.97
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
    if(bSizeFlag==sal_False)
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
        //SetSizePixel(aNewSize);
        //@ SetUpdateMode( sal_True);
        bSizeFlag=sal_False;
    }
}

/*************************************************************************
#*  Member:     SetMyWidthLeRi                              Datum:15.10.97
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
    Point aSplitterTopLeft=aPrivatSplit.GetPosPixel();
    Point aFDTopLeft=aFiFuncDesc.GetPosPixel();

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
#*  Member:     SetHeight                                   Datum:06.10.97
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
#*  Member:     SetMyWidthToBo                              Datum:16.10.97
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
#*  Member:     SetHeight                                   Datum:16.10.97
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
#*  Member:     SetDescription                              Datum:13.10.97
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

        String  aString=pAllFuncList->GetSelectEntry();
        if(nDockMode==0)
        {
            aString.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ":\n\n" ));
        }
        else
        {
            aString.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ":   " ));
        }

        aString+=pDesc->GetParamList();

        if(nDockMode==0)
        {
            aString.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "\n\n" ));
        }
        else
        {
            aString += '\n';
        }

        aString+=*(pDesc->pFuncDesc);

        aFiFuncDesc.SetText(aString);
        aFiFuncDesc.StateChanged(STATE_CHANGE_TEXT);
        aFiFuncDesc.Invalidate();
        aFiFuncDesc.Update();

    }
 }

/*************************************************************************
#*  Member:     Resizing                                    Datum:06.10.97
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

void __EXPORT ScFunctionDockWin::Resizing( Size& rNewSize )
{
    if((sal_uLong)rNewSize.Width()<nMinWidth) rNewSize.Width()=nMinWidth;
    if((sal_uLong)rNewSize.Height()<nMinHeight) rNewSize.Height()=nMinHeight;

}

/*************************************************************************
#*  Member:     Close                                       Datum:07.10.97
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

sal_Bool __EXPORT ScFunctionDockWin::Close()
{
    SfxBoolItem aItem( FID_FUNCTION_BOX, sal_False );

    GetBindings().GetDispatcher()->Execute( FID_FUNCTION_BOX,
                                SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                                &aItem, 0L );

    SfxDockingWindow::Close();

    return( sal_True );
}


/*************************************************************************
#*  Member:     CheckAlignment                              Datum:16.10.97
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
SfxChildAlignment __EXPORT ScFunctionDockWin::CheckAlignment(SfxChildAlignment /* abla */,
                                SfxChildAlignment aChildAlign)
{
    String aString = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("ww"));
    Size aTxtSize( aFiFuncDesc.GetTextWidth(aString), aFiFuncDesc.GetTextHeight() );
    if(!bInit)
    {
        eSfxOldAlignment=eSfxNewAlignment;
        eSfxNewAlignment=aChildAlign;
    }
    else
    {
        bInit=sal_False;
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

                        nMinWidth= 0;/*aDDFuncList.GetPosPixel().X()+
                                    10*aTxtSize.Width()+
                                    aFuncList.GetPosPixel().X();*/
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
                            //aCatBox.SelectEntryPos(0);

                        break;
    }

    return aChildAlign;
}
/*************************************************************************
#*  Member:     Close                                       Datum:07.10.97
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
//    const SfxPoolItemHint *pPoolItemHint = PTR_CAST(SfxPoolItemHint, &rHint);
    /*
    if ( pPoolItemHint
         && ( pPoolItemHint->GetObject()->ISA( SvxColorTableItem ) ) )
    {
        // Die Liste der Farben hat sich geaendert
        pColorTable = ( (SvxColorTableItem*) pPoolItemHint->GetObject() )->GetColorTable();
        FillValueSet();
    }
    */
}


/*************************************************************************
#*  Member:     Resize                                  Datum:06.10.97
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

void __EXPORT ScFunctionDockWin::Resize()
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
#*  Member:     UpdateFunctionList                          Datum:06.10.97
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
    pAllFuncList->SetUpdateMode( sal_False );

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
        for ( sal_uInt16 i=0; i<LRU_MAX && aLRUList[i]; i++ )
        {
            const ScFuncDesc* pDesc = aLRUList[i];
            pAllFuncList->SetEntryData(
                    pAllFuncList->InsertEntry( *(pDesc->pFuncName) ),
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
#*  Member:     DoEnter                                     Datum:06.10.97
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
    String aParaStr;
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
                aFirstArgStr.EraseLeadingAndTrailingChars();
                aFirstArgStr.SearchAndReplaceAll(' ', '_');
                aArgStr = aFirstArgStr;
                if ( nArgs != VAR_ARGS )
                {   // no VarArgs or Fix plus VarArgs, but not VarArgs only
                    String aArgSep = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM( "; " ));
                    sal_uInt16 nFix = ( nArgs < VAR_ARGS ? nArgs : nArgs - VAR_ARGS + 1 );
                    for ( sal_uInt16 nArg = 1;
                            nArg < nFix && !pDesc->pDefArgFlags[nArg].bOptional; nArg++ )
                    {
                        if (!pDesc->pDefArgFlags[nArg].bSuppress)
                        {
                            aArgStr += aArgSep;
                            String sTmp(*(pDesc->ppDefArgNames[nArg]));
                            sTmp.EraseLeadingAndTrailingChars();
                            sTmp.SearchAndReplaceAll(' ', '_');
                            aArgStr += sTmp;
                        }
                    }
                }
            }
        }
        if (pHdl)
        {
            if(pHdl->GetEditString().Len()==0)
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
                    pEdView->InsertText(aString,sal_False);
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
#*  Handle:     SelHdl                                      Datum:06.10.97
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
#*  Handle:     SelHdl                                      Datum:06.10.97
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
#*  Handle:     SetSplitHdl                                 Datum:13.10.97
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
        /*
        aFuncList.Invalidate();
        aFuncList.Update();
        aFiFuncDesc.Invalidate();
        aFiFuncDesc.Update();
        */
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

IMPL_LINK( ScFunctionDockWin, TimerHdl, Timer*, EMPTYARG )
{
    CheckAlignment(eSfxOldAlignment,eSfxNewAlignment);
    SetSize();
    return 0;
}

void ScFunctionDockWin::Initialize(SfxChildWinInfo *pInfo)
{
    String aStr;
    if(pInfo!=NULL)
    {
        if ( pInfo->aExtraString.Len() )
        {
            xub_StrLen nPos = pInfo->aExtraString.Search(
                String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("ScFuncList:")));

            // Versuche, den Alignment-String "ALIGN:(...)" einzulesen; wenn
            // er nicht vorhanden ist, liegt eine "altere Version vor
            if ( nPos != STRING_NOTFOUND )
            {
                xub_StrLen n1 = pInfo->aExtraString.Search('(', nPos);
                if ( n1 != STRING_NOTFOUND )
                {
                    xub_StrLen n2 = pInfo->aExtraString.Search(')', n1);
                    if ( n2 != STRING_NOTFOUND )
                    {
                        // Alignment-String herausschneiden
                        aStr = pInfo->aExtraString.Copy(nPos, n2 - nPos + 1);
                        pInfo->aExtraString.Erase(nPos, n2 - nPos + 1);
                        aStr.Erase(0, n1-nPos+1);
                    }
                }
            }
        }
    }
    SfxDockingWindow::Initialize(pInfo);

    if ( aStr.Len())
    {
        aSplitterInitPos=aPrivatSplit.GetPosPixel();
        aSplitterInitPos.Y()=(sal_uInt16) aStr.ToInt32();
        xub_StrLen n1 = aStr.Search(';');
        aStr.Erase(0, n1+1);
        sal_uInt16 nSelPos=sal::static_int_cast<sal_uInt16>( aStr.ToInt32() );
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
    rInfo.aExtraString.AppendAscii(RTL_CONSTASCII_STRINGPARAM( "ScFuncList:(" ));
    rInfo.aExtraString += String::CreateFromInt32(aPoint.Y());
    rInfo.aExtraString += ';';
    rInfo.aExtraString += String::CreateFromInt32(aCatBox.GetSelectEntryPos());
    rInfo.aExtraString += ')';
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


