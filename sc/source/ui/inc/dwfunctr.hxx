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
#pragma once
#if 1

#include <sfx2/childwin.hxx>
#include <sfx2/dockwin.hxx>
#include <svl/lstner.hxx>
#include <svtools/stdctrl.hxx>

#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include "anyrefdg.hxx"
#include "global.hxx"       // ScAddress
#include "privsplt.hxx"
#include "funcdesc.hxx"

/** Derivation of SfxChildWindow as "containers" for Controller */
class ScFunctionChildWindow : public SfxChildWindow
{
 public:
    ScFunctionChildWindow( Window*, sal_uInt16, SfxBindings*,
                            SfxChildWinInfo* );

    SFX_DECL_CHILDWINDOW_WITHID(ScFunctionChildWindow);
};

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

    ::std::vector< const formula::IFunctionDescription*> aLRUList;

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
                    DECL_LINK( TimerHdl, void*);

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
