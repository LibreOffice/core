/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_TITLEPAGE_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_TITLEPAGE_HXX

#include <sfx2/basedlgs.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <numberingtypelistbox.hxx>

namespace vcl { class Window; }
class SwWrtShell;
class SwPageDesc;

class SwTitlePageDlg : public SfxModalDialog
{
private:
    VclPtr<RadioButton> m_pUseExistingPagesRB;
    VclPtr<NumericField> m_pPageCountNF;

    VclPtr<RadioButton> m_pDocumentStartRB;
    VclPtr<RadioButton> m_pPageStartRB;
    VclPtr<NumericField> m_pPageStartNF;

    VclPtr<CheckBox> m_pRestartNumberingCB;
    VclPtr<NumericField> m_pRestartNumberingNF;
    VclPtr<CheckBox> m_pSetPageNumberCB;
    VclPtr<NumericField> m_pSetPageNumberNF;

    VclPtr<ListBox> m_pPagePropertiesLB;
    VclPtr<PushButton> m_pPagePropertiesPB;

    VclPtr<OKButton> m_pOkPB;

    SwWrtShell *mpSh;

    const SwFormatPageDesc *mpPageFormatDesc;

    SwPageDesc *mpTitleDesc;
    const SwPageDesc *mpIndexDesc;
    const SwPageDesc *mpNormalDesc;

    void FillList();

    sal_uInt16 GetInsertPosition() const;

    DECL_LINK_TYPED(OKHdl, Button*, void);
    DECL_LINK_TYPED(EditHdl, Button*, void);
    DECL_LINK_TYPED(RestartNumberingHdl, Button*, void);
    DECL_LINK_TYPED(SetPageNumberHdl, Button*, void);
    DECL_LINK_TYPED(UpHdl, SpinField&, void);
    DECL_LINK_TYPED(DownHdl, SpinField&, void);
    DECL_LINK_TYPED(StartPageHdl, Button*, void);
public:
    SwTitlePageDlg( vcl::Window *pParent );
    virtual ~SwTitlePageDlg();
    virtual void dispose() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
