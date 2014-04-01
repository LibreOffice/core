/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_UI_INC_TITLEPAGE_HXX
#define INCLUDED_SW_SOURCE_UI_INC_TITLEPAGE_HXX

#include <sfx2/basedlgs.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <numberingtypelistbox.hxx>

class Window;
class SwWrtShell;
class SwPageDesc;

class SwTitlePageDlg : public SfxModalDialog
{
private:
    RadioButton* m_pUseExistingPagesRB;
    NumericField* m_pPageCountNF;

    RadioButton* m_pDocumentStartRB;
    RadioButton* m_pPageStartRB;
    NumericField* m_pPageStartNF;

    CheckBox* m_pRestartNumberingCB;
    NumericField* m_pRestartNumberingNF;
    CheckBox* m_pSetPageNumberCB;
    NumericField* m_pSetPageNumberNF;

    ListBox* m_pPagePropertiesLB;
    PushButton* m_pPagePropertiesPB;

    OKButton* m_pOkPB;

    SwWrtShell *mpSh;

    const SwFmtPageDesc *mpPageFmtDesc;

    SwPageDesc *mpTitleDesc;
    const SwPageDesc *mpIndexDesc;
    const SwPageDesc *mpNormalDesc;

    void FillList();

    sal_uInt16 GetInsertPosition() const;

    DECL_LINK(OKHdl, void *);
    DECL_LINK(EditHdl, void *);
    DECL_LINK(RestartNumberingHdl, void *);
    DECL_LINK(SetPageNumberHdl, void *);
    DECL_LINK(UpHdl, void *);
    DECL_LINK(DownHdl, void *);
    DECL_LINK(StartPageHdl, void *);
public:
    SwTitlePageDlg( Window *pParent );
    virtual ~SwTitlePageDlg();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
