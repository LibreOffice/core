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
#include <tools/fract.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/progress.hxx>
#include <misc/scopelock.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>
#include <vcl/weldutils.hxx>

class SdDrawDocument;

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

class SdDisplay : public weld::CustomWidgetController
{
private:
    BitmapEx    aBitmapEx;
    Fraction    aScale;

public:
    SdDisplay();
    virtual ~SdDisplay() override;

    virtual void Paint( vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect ) override;

    void    SetBitmapEx( BitmapEx const * pBmpEx );
    void    SetScale( const Fraction& rFrac );

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
};

class AnimationWindow : public SfxDockingWindow
{
 friend class AnimationChildWindow;
 friend class AnimationControllerItem;

public:
    AnimationWindow(SfxBindings* pBindings, SfxChildWindow *pCW, vcl::Window* pParent);
    virtual ~AnimationWindow() override;
    virtual void dispose() override;

    void    AddObj( ::sd::View& rView );
    void    CreateAnimObj( ::sd::View& rView );

    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;

protected:
    virtual bool    Close() override;
    virtual void    Resize() override;

private:
    std::unique_ptr<SdDisplay> m_xCtlDisplay;
    std::unique_ptr<weld::CustomWeld> m_xCtlDisplayWin;
    std::unique_ptr<weld::Button> m_xBtnFirst;
    std::unique_ptr<weld::Button> m_xBtnReverse;
    std::unique_ptr<weld::Button> m_xBtnStop;
    std::unique_ptr<weld::Button> m_xBtnPlay;
    std::unique_ptr<weld::Button> m_xBtnLast;
    std::unique_ptr<weld::SpinButton> m_xNumFldBitmap;
    std::unique_ptr<weld::FormattedSpinButton> m_xTimeField;
    std::unique_ptr<weld::TimeFormatter> m_xFormatter;
    std::unique_ptr<weld::ComboBox> m_xLbLoopCount;
    std::unique_ptr<weld::Button> m_xBtnGetOneObject;
    std::unique_ptr<weld::Button> m_xBtnGetAllObjects;
    std::unique_ptr<weld::Button> m_xBtnRemoveBitmap;
    std::unique_ptr<weld::Button> m_xBtnRemoveAll;
    std::unique_ptr<weld::Label> m_xFiCount;

    std::unique_ptr<weld::RadioButton> m_xRbtGroup;
    std::unique_ptr<weld::RadioButton> m_xRbtBitmap;
    std::unique_ptr<weld::Label> m_xFtAdjustment;
    std::unique_ptr<weld::ComboBox> m_xLbAdjustment;
    std::unique_ptr<weld::Button> m_xBtnCreateGroup;
    std::unique_ptr<weld::Button> m_xBtnHelp;

    ::std::vector< ::std::pair<BitmapEx, ::tools::Time> > m_FrameList;
    static const size_t EMPTY_FRAMELIST;
    size_t          m_nCurrentFrame;
    std::unique_ptr<SdDrawDocument> pMyDoc;

    bool            bMovie;
    bool            bAllObjects;

    std::unique_ptr<AnimationControllerItem> pControllerItem;

    ScopeLock       maPlayLock;

    DECL_LINK( ClickFirstHdl, weld::Button&, void );
    DECL_LINK( ClickStopHdl, weld::Button&, void );
    DECL_LINK( ClickPlayHdl, weld::Button&, void );
    DECL_LINK( ClickLastHdl, weld::Button&, void );
    DECL_LINK( ClickGetObjectHdl, weld::Button&, void );
    DECL_LINK( ClickRemoveBitmapHdl, weld::Button&, void );
    DECL_LINK( ClickRbtHdl, weld::Button&, void );
    DECL_LINK( ClickHelpHdl, weld::Button&, void );
    DECL_LINK( ClickCreateGroupHdl, weld::Button&, void );
    DECL_LINK( ModifyBitmapHdl, weld::SpinButton&, void );
    DECL_LINK( ModifyTimeHdl, weld::FormattedSpinButton&, void );

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
