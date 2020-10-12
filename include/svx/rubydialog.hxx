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


#ifndef INCLUDED_SVX_RUBYDIALOG_HXX
#define INCLUDED_SVX_RUBYDIALOG_HXX

#include <sfx2/childwin.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/customweld.hxx>
#include <svx/svxdllapi.h>
#include <rtl/ref.hxx>

class SvxRubyDialog;
class RubyPreview final : public weld::CustomWidgetController
{
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    SvxRubyDialog* m_pParentDlg;

public:
    RubyPreview();
    virtual ~RubyPreview() override;
    void setRubyDialog(SvxRubyDialog* pParentDlg)
    {
        m_pParentDlg = pParentDlg;
    }
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
};

class SVX_DLLPUBLIC SvxRubyChildWindow final : public SfxChildWindow
{
public:
    SvxRubyChildWindow( vcl::Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo const * );
    SFX_DECL_CHILDWINDOW( SvxRubyChildWindow );
};

class SvxRubyData_Impl;

class SvxRubyDialog : public SfxModelessDialogController
{
    friend class RubyPreview;


    tools::Long                nLastPos;
    tools::Long                nCurrentEdit;
    bool                bModified;
    SfxBindings*        pBindings;
    rtl::Reference<SvxRubyData_Impl> m_pImpl;
    weld::Entry* aEditArr[8];

    std::unique_ptr<weld::Label> m_xLeftFT;
    std::unique_ptr<weld::Label> m_xRightFT;
    std::unique_ptr<weld::Entry> m_xLeft1ED;
    std::unique_ptr<weld::Entry> m_xRight1ED;
    std::unique_ptr<weld::Entry> m_xLeft2ED;
    std::unique_ptr<weld::Entry> m_xRight2ED;
    std::unique_ptr<weld::Entry> m_xLeft3ED;
    std::unique_ptr<weld::Entry> m_xRight3ED;
    std::unique_ptr<weld::Entry> m_xLeft4ED;
    std::unique_ptr<weld::Entry> m_xRight4ED;

    std::unique_ptr<weld::ScrolledWindow> m_xScrolledWindow;

    std::unique_ptr<weld::ComboBox> m_xAdjustLB;

    std::unique_ptr<weld::ComboBox> m_xPositionLB;

    std::unique_ptr<weld::Label> m_xCharStyleFT;
    std::unique_ptr<weld::ComboBox> m_xCharStyleLB;
    std::unique_ptr<weld::Button> m_xStylistPB;

    std::unique_ptr<weld::Button> m_xApplyPB;
    std::unique_ptr<weld::Button> m_xClosePB;

    std::unique_ptr<weld::Container> m_xContentArea;
    std::unique_ptr<weld::Widget> m_xGrid;

    std::unique_ptr<RubyPreview> m_xPreviewWin;
    std::unique_ptr<weld::CustomWeld> m_xPreview;

    DECL_LINK(ApplyHdl_Impl, weld::Button&, void);
    DECL_LINK(CloseHdl_Impl, weld::Button&, void);
    DECL_LINK(StylistHdl_Impl, weld::Button&, void);
    DECL_LINK(ScrollHdl_Impl, weld::ScrolledWindow&, void);
    DECL_LINK(PositionHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(AdjustHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(CharStyleHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(EditModifyHdl_Impl, weld::Entry&, void);
    DECL_LINK(EditFocusHdl_Impl, weld::Widget&, void);
    DECL_LINK(KeyUpDownHdl_Impl, const KeyEvent&, bool);
    DECL_LINK(KeyUpDownTabHdl_Impl, const KeyEvent&, bool);

    bool EditScrollHdl_Impl(sal_Int32 nParam);
    bool EditJumpHdl_Impl(sal_Int32 nParam);

    void                SetRubyText(sal_Int32 nPos, weld::Entry& rLeft, weld::Entry& rRight);
    void                GetRubyText();
    void                ClearCharStyleList();
    void                AssertOneEntry();

    void                Update();
    virtual void        Close() override;

    tools::Long                GetLastPos() const {return nLastPos;}
    void                SetLastPos(tools::Long nSet) {nLastPos = nSet;}

    bool                IsModified() const {return bModified;}
    void                SetModified(bool bSet) {bModified = bSet;}

    void EnableControls(bool bEnable);

    void                GetCurrentText(OUString& rBase, OUString& rRuby);

public:
    SvxRubyDialog(SfxBindings *pBindings, SfxChildWindow *pCW, weld::Window* pParent);
    virtual ~SvxRubyDialog() override;

    virtual void        Activate() override;
};

#endif // INCLUDED_SVX_RUBYDIALOG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
