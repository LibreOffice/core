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

#ifndef INCLUDED_SD_SOURCE_UI_INC_ANIMOBJS_HXX
#define INCLUDED_SD_SOURCE_UI_INC_ANIMOBJS_HXX

#include <sfx2/dockwin.hxx>
#include <vcl/fixed.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/group.hxx>
#include <sfx2/ctrlitem.hxx>

#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <svx/dlgctrl.hxx>
#include <sfx2/progress.hxx>


#include <vcl/lstbox.hxx>

#include "sdresid.hxx"
#include "misc/scopelock.hxx"

class SdDrawDocument;
class BitmapEx;

namespace sd {

class AnimationControllerItem;
class View;



enum BitmapAdjustment
{
    BA_LEFT_UP,
    BA_LEFT,
    BA_LEFT_DOWN,
    BA_UP,
    BA_CENTER,
    BA_DOWN,
    BA_RIGHT_UP,
    BA_RIGHT,
    BA_RIGHT_DOWN
};



class SdDisplay : public Control
{
private:
    BitmapEx    aBitmapEx;
    Fraction    aScale;

public:
    SdDisplay( ::Window* pWin, SdResId Id );
    virtual ~SdDisplay();

    virtual void Paint( const Rectangle& rRect ) SAL_OVERRIDE;

    void    SetBitmapEx( BitmapEx* pBmpEx );
    void    SetScale( const Fraction& rFrac );

    virtual void DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
};



class AnimationWindow : public SfxDockingWindow
{
 friend class AnimationChildWindow;
 friend class AnimationControllerItem;

public:
    AnimationWindow( SfxBindings* pBindings, SfxChildWindow *pCW,
        ::Window* pParent, const SdResId& rSdResId );
    virtual ~AnimationWindow();

    void    AddObj( ::sd::View& rView );
    void    CreateAnimObj( ::sd::View& rView );

    virtual void DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

protected:
    virtual bool    Close() SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    FillInfo( SfxChildWinInfo& ) const SAL_OVERRIDE;

private:
    SdDisplay       aCtlDisplay;
    ImageButton     aBtnFirst;
    ImageButton     aBtnReverse;
    ImageButton     aBtnStop;
    ImageButton     aBtnPlay;
    ImageButton     aBtnLast;
    NumericField    aNumFldBitmap;
    TimeField       aTimeField;
    ListBox         aLbLoopCount;
    FixedLine       aGrpBitmap;
    ImageButton     aBtnGetOneObject;
    ImageButton     aBtnGetAllObjects;
    ImageButton     aBtnRemoveBitmap;
    ImageButton     aBtnRemoveAll;
    FixedText       aFtCount;
    FixedInfo       aFiCount;
    FixedLine       aGrpAnimation;

    RadioButton     aRbtGroup;
    RadioButton     aRbtBitmap;
    FixedText       aFtAdjustment;
    ListBox         aLbAdjustment;
    PushButton      aBtnCreateGroup;

    HelpButton      aBtnHelp;


    ::Window*       pWin;
    ::std::vector< ::std::pair<BitmapEx*, Time*> > m_FrameList;
    static size_t EMPTY_FRAMELIST;
    size_t          m_nCurrentFrame;
    SdDrawDocument* pMyDoc;

    Size            aSize;
    Size            aFltWinSize;
    Size            aDisplaySize;
    Size            aBmpSize;
    bool            bMovie;
    bool            bAllObjects;

    SfxBindings*                pBindings;
    AnimationControllerItem*    pControllerItem;

    ScopeLock       maPlayLock;


    DECL_LINK( ClickFirstHdl, void * );
    DECL_LINK( ClickStopHdl, void * );
    DECL_LINK( ClickPlayHdl, void * );
    DECL_LINK( ClickLastHdl, void * );
    DECL_LINK( ClickGetObjectHdl, void * );
    DECL_LINK( ClickRemoveBitmapHdl, void * );
    DECL_LINK( ClickRbtHdl, void * );
    DECL_LINK( ClickCreateGroupHdl, void * );
    DECL_LINK( ModifyBitmapHdl, void * );
    DECL_LINK( ModifyTimeHdl, void * );

    void            UpdateControl(bool bDisableCtrls = false);
    void            ResetAttrs();
    void            WaitInEffect( sal_uLong nMilliSeconds, sal_uLong nTime,
                                        SfxProgress* pStbMgr ) const;
    Fraction        GetScale();
};

/**
 * ControllerItem for Animator
 */
class AnimationControllerItem : public SfxControllerItem
{

public:
    AnimationControllerItem( sal_uInt16, AnimationWindow*, SfxBindings* );

protected:
    virtual void StateChanged( sal_uInt16 nSId, SfxItemState eState,
        const SfxPoolItem* pState ) SAL_OVERRIDE;
private:
    AnimationWindow* pAnimationWin;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
