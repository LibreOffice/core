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

#include <sfx2/tabdlg.hxx>
#include <svx/sdtakitm.hxx>
#include <svx/sdtaditm.hxx>
#include <vcl/weld.hxx>

class SdrView;

/*************************************************************************
|*
|* Page for changing TextAnimations (running text etc.)
|*
\************************************************************************/

class SvxTextAnimationPage : public SfxTabPage
{
private:
    static const sal_uInt16     pRanges[];

    SdrTextAniKind      eAniKind;
    FieldUnit           eFUnit;
    MapUnit             eUnit;

    TriState m_aUpState;
    TriState m_aLeftState;
    TriState m_aRightState;
    TriState m_aDownState;

    std::unique_ptr<weld::ComboBox> m_xLbEffect;
    std::unique_ptr<weld::Widget> m_xBoxDirection;
    std::unique_ptr<weld::ToggleButton> m_xBtnUp;
    std::unique_ptr<weld::ToggleButton> m_xBtnLeft;
    std::unique_ptr<weld::ToggleButton> m_xBtnRight;
    std::unique_ptr<weld::ToggleButton> m_xBtnDown;

    std::unique_ptr<weld::Frame> m_xFlProperties;
    std::unique_ptr<weld::CheckButton> m_xTsbStartInside;
    std::unique_ptr<weld::CheckButton> m_xTsbStopInside;

    std::unique_ptr<weld::Widget> m_xBoxCount;
    std::unique_ptr<weld::CheckButton> m_xTsbEndless;
    std::unique_ptr<weld::SpinButton> m_xNumFldCount;

    std::unique_ptr<weld::CheckButton> m_xTsbPixel;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldAmount;

    std::unique_ptr<weld::CheckButton> m_xTsbAuto;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldDelay;

    DECL_LINK( SelectEffectHdl_Impl, weld::ComboBox&, void );
    DECL_LINK( ClickEndlessHdl_Impl, weld::Button&, void );
    DECL_LINK( ClickAutoHdl_Impl, weld::Button&, void );
    DECL_LINK( ClickPixelHdl_Impl, weld::Button&, void );
    DECL_LINK( ClickDirectionHdl_Impl, weld::Button&, void );

    void SelectDirection( SdrTextAniDirection nValue );
    sal_uInt16 GetSelectedDirection() const;

public:
    SvxTextAnimationPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs);
    virtual ~SvxTextAnimationPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* );
    static const sal_uInt16* GetRanges() { return pRanges; }

    virtual bool        FillItemSet( SfxItemSet* ) override;
    virtual void        Reset( const SfxItemSet * ) override;
};

/*************************************************************************
|*
|* Text-Tab-Dialog
|*
\************************************************************************/
class SvxTextTabDialog : public SfxTabDialogController
{
private:
    const SdrView*      pView;

    virtual void        PageCreated(const OString& rId, SfxTabPage &rPage) override;

public:
    SvxTextTabDialog(weld::Window* pParent, const SfxItemSet* pAttr, const SdrView* pView);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
