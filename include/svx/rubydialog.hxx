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
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/scrbar.hxx>
#include <com/sun/star/uno/Reference.h>
#include <svx/svxdllapi.h>

namespace com{namespace sun{namespace star{
    namespace view{
        class XSelectionChangeListener;
    }
}}}


class SvxRubyDialog;
class RubyPreview : public vcl::Window
{
protected:
    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;
    VclPtr<SvxRubyDialog> m_pParentDlg;

public:
    RubyPreview(vcl::Window *pParent);
    virtual ~RubyPreview();
    virtual void dispose() override;
    void setRubyDialog(SvxRubyDialog* pParentDlg)
    {
        m_pParentDlg = pParentDlg;
    }
    virtual Size GetOptimalSize() const override;
};

class SVX_DLLPUBLIC SvxRubyChildWindow : public SfxChildWindow
{
 public:

    SvxRubyChildWindow( vcl::Window*, sal_uInt16, SfxBindings*, SfxChildWinInfo* );

    SFX_DECL_CHILDWINDOW( SvxRubyChildWindow );

};
class SvxRubyData_Impl;
class RubyEdit  : public Edit
{
    Link<sal_Int32,bool>  aScrollHdl;
    Link<sal_Int32,void>  aJumpHdl;
    virtual void        GetFocus() override;
    virtual bool        PreNotify( NotifyEvent& rNEvt ) override;
public:
    RubyEdit(vcl::Window* pParent, const ResId& rResId)
        : Edit(pParent, rResId)
    {
    }
    RubyEdit(vcl::Window* pParent)
        : Edit(pParent, WB_BORDER)
    {
    }
    void    SetScrollHdl(Link<sal_Int32,bool>& rLink) {aScrollHdl = rLink;}
    void    SetJumpHdl(Link<sal_Int32,void>& rLink) {aJumpHdl = rLink;}
};


class SvxRubyDialog : public SfxModelessDialog
{
    friend class RubyPreview;

    VclPtr<FixedText>          m_pLeftFT;
    VclPtr<FixedText>          m_pRightFT;
    VclPtr<RubyEdit>           m_pLeft1ED;
    VclPtr<RubyEdit>           m_pRight1ED;
    VclPtr<RubyEdit>           m_pLeft2ED;
    VclPtr<RubyEdit>           m_pRight2ED;
    VclPtr<RubyEdit>           m_pLeft3ED;
    VclPtr<RubyEdit>           m_pRight3ED;
    VclPtr<RubyEdit>           m_pLeft4ED;
    VclPtr<RubyEdit>           m_pRight4ED;

    VclPtr<RubyEdit>           aEditArr[8];
    VclPtr<VclScrolledWindow>  m_pScrolledWindow;
    VclPtr<ScrollBar>          m_pScrollSB;

    VclPtr<ListBox>            m_pAdjustLB;

    VclPtr<ListBox>            m_pPositionLB;

    VclPtr<FixedText>          m_pCharStyleFT;
    VclPtr<ListBox>            m_pCharStyleLB;
    VclPtr<PushButton>         m_pStylistPB;

    VclPtr<RubyPreview>        m_pPreviewWin;

    VclPtr<PushButton>         m_pApplyPB;
    VclPtr<PushButton>         m_pClosePB;

    long                nLastPos;
    long                nCurrentEdit;

    bool                bModified;

    com::sun::star::uno::Reference<com::sun::star::view::XSelectionChangeListener> xImpl;
    SfxBindings*    pBindings;
    SvxRubyData_Impl* pImpl;

    DECL_LINK_TYPED(ApplyHdl_Impl, Button*, void);
    DECL_LINK_TYPED(CloseHdl_Impl, Button*, void);
    DECL_LINK_TYPED(StylistHdl_Impl, Button*, void);
    DECL_LINK_TYPED(ScrollHdl_Impl, ScrollBar*, void);
    DECL_LINK_TYPED(PositionHdl_Impl, ListBox&, void);
    DECL_LINK_TYPED(AdjustHdl_Impl, ListBox&, void);
    DECL_LINK_TYPED(CharStyleHdl_Impl, ListBox&, void);
    DECL_LINK(EditModifyHdl_Impl, Edit*);
    DECL_LINK_TYPED(EditScrollHdl_Impl, sal_Int32, bool);
    DECL_LINK_TYPED(EditJumpHdl_Impl, sal_Int32, void);

    void                SetRubyText(sal_Int32 nPos, Edit& rLeft, Edit& rRight);
    void                GetRubyText();
    void                ClearCharStyleList();
    void                AssertOneEntry();

    void                Update();
    virtual bool        Close() override;

    long                GetLastPos() const {return nLastPos;}
    void                SetLastPos(long nSet) {nLastPos = nSet;}

    bool                IsModified() const {return bModified;}
    void                SetModified(bool bSet) {bModified = bSet;}

    void EnableControls(bool bEnable);

    void                GetCurrentText(OUString& rBase, OUString& rRuby);

    void                UpdateColors();
protected:
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;
public:

                        SvxRubyDialog(SfxBindings *pBindings, SfxChildWindow *pCW,
                                    vcl::Window* pParent);
    virtual             ~SvxRubyDialog();
    virtual void        dispose() override;

    virtual void        Activate() override;
    virtual void        Deactivate() override;
};

#endif // INCLUDED_SVX_RUBYDIALOG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
