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
#include <tools/fract.hxx>
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
    SdDisplay(vcl::Window* pWin);
    virtual ~SdDisplay();

    virtual void Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;

    void    SetBitmapEx( BitmapEx* pBmpEx );
    void    SetScale( const Fraction& rFrac );

    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual Size GetOptimalSize() const override;
};

class AnimationWindow : public SfxDockingWindow
{
 friend class AnimationChildWindow;
 friend class AnimationControllerItem;

public:
    AnimationWindow(SfxBindings* pBindings, SfxChildWindow *pCW, vcl::Window* pParent);
    virtual ~AnimationWindow();
    virtual void dispose() override;

    void    AddObj( ::sd::View& rView );
    void    CreateAnimObj( ::sd::View& rView );

    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;

protected:
    virtual bool    Close() override;
    virtual void    Resize() override;
    virtual void    FillInfo( SfxChildWinInfo& ) const override;

private:
    VclPtr<SdDisplay>      m_pCtlDisplay;
    VclPtr<PushButton>     m_pBtnFirst;
    VclPtr<PushButton>     m_pBtnReverse;
    VclPtr<PushButton>     m_pBtnStop;
    VclPtr<PushButton>     m_pBtnPlay;
    VclPtr<PushButton>     m_pBtnLast;
    VclPtr<NumericField>   m_pNumFldBitmap;
    VclPtr<TimeField>      m_pTimeField;
    VclPtr<ListBox>        m_pLbLoopCount;
    VclPtr<PushButton>     m_pBtnGetOneObject;
    VclPtr<PushButton>     m_pBtnGetAllObjects;
    VclPtr<PushButton>     m_pBtnRemoveBitmap;
    VclPtr<PushButton>     m_pBtnRemoveAll;
    VclPtr<FixedText>      m_pFiCount;

    VclPtr<RadioButton>    m_pRbtGroup;
    VclPtr<RadioButton>    m_pRbtBitmap;
    VclPtr<FixedText>      m_pFtAdjustment;
    VclPtr<ListBox>        m_pLbAdjustment;
    VclPtr<PushButton>     m_pBtnCreateGroup;

    VclPtr<vcl::Window>    pWin;
    ::std::vector< ::std::pair<BitmapEx*, ::tools::Time*> > m_FrameList;
    static const size_t EMPTY_FRAMELIST;
    size_t          m_nCurrentFrame;
    SdDrawDocument* pMyDoc;

    bool            bMovie;
    bool            bAllObjects;

    SfxBindings*                pBindings;
    AnimationControllerItem*    pControllerItem;

    ScopeLock       maPlayLock;

    DECL_LINK_TYPED( ClickFirstHdl, Button*, void );
    DECL_LINK_TYPED( ClickStopHdl, Button*, void );
    DECL_LINK_TYPED( ClickPlayHdl, Button*, void );
    DECL_LINK_TYPED( ClickLastHdl, Button*, void );
    DECL_LINK_TYPED( ClickGetObjectHdl, Button*, void );
    DECL_LINK_TYPED( ClickRemoveBitmapHdl, Button*, void );
    DECL_LINK_TYPED( ClickRbtHdl, Button*, void );
    DECL_LINK_TYPED( ClickCreateGroupHdl, Button*, void );
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
        const SfxPoolItem* pState ) override;
private:
    VclPtr<AnimationWindow> pAnimationWin;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
