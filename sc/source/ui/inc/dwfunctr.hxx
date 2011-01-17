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
#ifndef _SC_DWFUNCTR_HXX
#define _SC_DWFUNCTR_HXX

#include <sfx2/childwin.hxx>
#include <sfx2/dockwin.hxx>
#include <svl/lstner.hxx>
#include <svtools/stdctrl.hxx>

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#include "anyrefdg.hxx"
#include "global.hxx"       // ScAddress
#include "privsplt.hxx"
#include "funcdesc.hxx"

#ifndef LRU_MAX
#define LRU_MAX 10
#endif
/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Controller
|*
\************************************************************************/

class ScFunctionChildWindow : public SfxChildWindow
{
 public:
    ScFunctionChildWindow( Window*, sal_uInt16, SfxBindings*,
                            SfxChildWinInfo* );

    SFX_DECL_CHILDWINDOW(ScFunctionChildWindow);
};

/*************************************************************************
|*
|* ScFuncDockWin
|*
\************************************************************************/

class ScFunctionDockWin : public SfxDockingWindow, public SfxListener
{

private:
    Timer               aTimer;
    ScPrivatSplit       aPrivatSplit;
    ListBox             aCatBox;
    ListBox             aFuncList;
    ListBox             aDDFuncList;
    ListBox*            pAllFuncList;

    SfxChildAlignment   eSfxNewAlignment;
    SfxChildAlignment   eSfxOldAlignment;
    ImageButton         aInsertButton;
    FixedText           aFiFuncDesc;
    sal_uInt16              nLeftSlot;
    sal_uInt16              nRightSlot;
    sal_uLong               nMinWidth;
    sal_uLong               nMinHeight;
    Size                aOldSize;
    sal_Bool                bSizeFlag;
    sal_Bool                bInit;
    short               nDockMode;
    Point               aSplitterInitPos;
    const ScFuncDesc*   pFuncDesc;
    sal_uInt16              nArgs;
    String**            pArgArr;


    const ScFuncDesc*   aLRUList[LRU_MAX];

    void            UpdateFunctionList();
    void            UpdateLRUList();
    void            DoEnter(sal_Bool bOk); //@@ ???
    void            SetDescription();
    void            SetLeftRightSize();
    void            SetTopBottonSize();
    void            SetMyWidthLeRi(Size &aNewSize);
    void            SetMyHeightLeRi(Size &aNewSize);
    void            SetMyWidthToBo(Size &aNewSize);
    void            SetMyHeightToBo(Size &aNewSize);
    void            UseSplitterInitPos();

                    DECL_LINK( SetSelectionHdl, void* );
                    DECL_LINK( SelHdl, ListBox* );
                    DECL_LINK(SetSplitHdl,ScPrivatSplit*);
                    DECL_LINK( TimerHdl, Timer*);

protected:

    virtual sal_Bool    Close();
    virtual void    Resize();
    virtual void    Resizing( Size& rSize );
    virtual void    SetSize();
    virtual void    ToggleFloatingMode();
    virtual void    StateChanged( StateChangedType nStateChange );


    virtual SfxChildAlignment CheckAlignment(SfxChildAlignment,
                                SfxChildAlignment eAlign);

public:
                    ScFunctionDockWin(  SfxBindings* pBindings,
                                        SfxChildWindow *pCW,
                                        Window* pParent,
                                        const ResId& rResId );

                    ~ScFunctionDockWin();

    using SfxDockingWindow::Notify;
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    void            SetSlotIDs( sal_uInt16 nLeft, sal_uInt16 nRight )
                        { nLeftSlot = nLeft; nRightSlot = nRight; }

    void            InitLRUList();

    void            Initialize (SfxChildWinInfo* pInfo);
    virtual void    FillInfo(SfxChildWinInfo&) const;
};

#endif


