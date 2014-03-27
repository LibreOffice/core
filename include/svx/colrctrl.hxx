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
#ifndef INCLUDED_SVX_COLRCTRL_HXX
#define INCLUDED_SVX_COLRCTRL_HXX

#include <sfx2/dockwin.hxx>
#include <sfx2/childwin.hxx>
#include <svx/SvxColorValueSet.hxx>
#include <svtools/transfer.hxx>
#include <svl/lstner.hxx>
#include <svx/svxdllapi.h>
#include <svx/xtable.hxx>

class SvData;

/*************************************************************************
|*
|* SvxColorValueSet_docking
|*
\************************************************************************/

class SvxColorValueSet_docking : public SvxColorValueSet, public DragSourceHelper
{
private:
    using SvxColorValueSet::StartDrag;

    bool            mbLeftButton;
    Point           aDragPosPixel;

protected:

    void            DoDrag();

    // ValueSet
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    Command(const CommandEvent& rCEvt ) SAL_OVERRIDE;

    // DragSourceHelper
    virtual void    StartDrag( sal_Int8 nAction, const Point& rPtPixel ) SAL_OVERRIDE;

                    DECL_STATIC_LINK(SvxColorValueSet_docking, ExecDragHdl, void*);

public:
                    SvxColorValueSet_docking( Window* pParent, WinBits nWinStyle = WB_ITEMBORDER );
                    SvxColorValueSet_docking( Window* pParent, const ResId& rResId );

    bool IsLeftButton() const { return mbLeftButton; }
};

/*************************************************************************
|*
|* SvxColorDockingWindow
|*
\************************************************************************/

class SvxColorDockingWindow : public SfxDockingWindow, public SfxListener
{
 friend class SvxColorChildWindow;

private:
    XColorListRef       pColorList;
    SvxColorValueSet_docking aColorSet;
    sal_uInt16          nLeftSlot;
    sal_uInt16          nRightSlot;
    sal_uInt16          nCols;
    sal_uInt16          nLines;
    long                nCount;
    Size                aItemSize;

    void                FillValueSet();
    void                SetSize();
                        DECL_LINK( SelectHdl, void * );

    /** This function is called when the window gets the focus.  It grabs
        the focus to the color value set so that it can be controlled with
        the keyboard.
    */
    virtual void GetFocus (void) SAL_OVERRIDE;

protected:
    virtual bool    Close() SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    Resizing( Size& rSize ) SAL_OVERRIDE;

public:
                    SvxColorDockingWindow( SfxBindings* pBindings,
                                           SfxChildWindow *pCW,
                                           Window* pParent,
                                           const ResId& rResId );
                    ~SvxColorDockingWindow();

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    void            SetSlotIDs( sal_uInt16 nLeft, sal_uInt16 nRight )
                        { nLeftSlot = nLeft; nRightSlot = nRight; }

    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
