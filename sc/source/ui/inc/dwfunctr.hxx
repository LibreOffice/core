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
#ifndef INCLUDED_SC_SOURCE_UI_INC_DWFUNCTR_HXX
#define INCLUDED_SC_SOURCE_UI_INC_DWFUNCTR_HXX

#include <sfx2/childwin.hxx>
#include <sfx2/dockwin.hxx>
#include <svl/lstner.hxx>
#include <svtools/stdctrl.hxx>

#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include "anyrefdg.hxx"
#include "global.hxx"
#include "privsplt.hxx"
#include "funcdesc.hxx"

/** Derivation of SfxChildWindow as "containers" for Controller */
class ScFunctionChildWindow : public SfxChildWindow
{
 public:
    ScFunctionChildWindow( vcl::Window*, sal_uInt16, SfxBindings*,
                            SfxChildWinInfo* );

    SFX_DECL_CHILDWINDOW_WITHID(ScFunctionChildWindow);
};

class ScFunctionDockWin : public SfxDockingWindow, public SfxListener
{

private:
    Idle                aIdle;
    VclPtr<ScPrivatSplit> aPrivatSplit;
    VclPtr<ListBox>     aCatBox;
    VclPtr<ListBox>     aFuncList;
    VclPtr<ListBox>     aDDFuncList;
    VclPtr<ListBox>            pAllFuncList;

    SfxChildAlignment   eSfxNewAlignment;
    SfxChildAlignment   eSfxOldAlignment;
    VclPtr<ImageButton> aInsertButton;
    VclPtr<FixedText>   aFiFuncDesc;
    sal_uLong           nMinWidth;
    sal_uLong           nMinHeight;
    Size                aOldSize;
    bool                bSizeFlag;
    bool                bInit;
    short               nDockMode;
    Point               aSplitterInitPos;
    const ScFuncDesc*   pFuncDesc;
    sal_uInt16              nArgs;

    ::std::vector< const formula::IFunctionDescription*> aLRUList;

    void            UpdateFunctionList();
    void            UpdateLRUList();
    void            DoEnter();
    void            SetDescription();
    void            SetLeftRightSize();
    void            SetTopBottonSize();
    void            SetMyWidthLeRi(Size &aNewSize);
    void            SetMyHeightLeRi(Size &aNewSize);
    void            SetMyWidthToBo(Size &aNewSize);
    void            SetMyHeightToBo(Size &aNewSize);
    void            UseSplitterInitPos();

                    DECL_LINK_TYPED( SetSelectionHdl, ListBox&, void );
                    DECL_LINK_TYPED( SetSelectionClickHdl, Button*, void );
                    DECL_LINK( SelHdl, ListBox* );
                    DECL_LINK(SetSplitHdl,ScPrivatSplit*);
                    DECL_LINK_TYPED( TimerHdl, Idle*, void );

protected:

    virtual bool    Close() SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    Resizing( Size& rSize ) SAL_OVERRIDE;
    void            SetSize();
    virtual void    ToggleFloatingMode() SAL_OVERRIDE;
    virtual void    StateChanged( StateChangedType nStateChange ) SAL_OVERRIDE;

    virtual SfxChildAlignment CheckAlignment(SfxChildAlignment,
                                SfxChildAlignment eAlign) SAL_OVERRIDE;

public:
                    ScFunctionDockWin(  SfxBindings* pBindings,
                                        SfxChildWindow *pCW,
                                        vcl::Window* pParent,
                                        const ResId& rResId );

                    virtual ~ScFunctionDockWin();
    virtual void    dispose() SAL_OVERRIDE;

    using SfxDockingWindow::Notify;
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    void            InitLRUList();

    void            Initialize (SfxChildWinInfo* pInfo);
    virtual void    FillInfo(SfxChildWinInfo&) const SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
