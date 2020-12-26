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

#include <fmtpdsc.hxx>

namespace vcl
{
class Window;
}
class SwWrtShell;
class SwPageDesc;

class SwTitlePageDlg : public SfxDialogController
{
private:
    SwWrtShell& mrSh;

    std::unique_ptr<const SwFormatPageDesc> mpPageFormatDesc;

    SwPageDesc* mpTitleDesc;
    const SwPageDesc* mpIndexDesc;
    const SwPageDesc* mpNormalDesc;

    std::unique_ptr<weld::RadioButton> m_xUseExistingPagesRB;
    std::unique_ptr<weld::SpinButton> m_xPageCountNF;
    std::unique_ptr<weld::RadioButton> m_xDocumentStartRB;
    std::unique_ptr<weld::RadioButton> m_xPageStartRB;
    std::unique_ptr<weld::SpinButton> m_xPageStartNF;
    std::unique_ptr<weld::CheckButton> m_xRestartNumberingCB;
    std::unique_ptr<weld::SpinButton> m_xRestartNumberingNF;
    std::unique_ptr<weld::CheckButton> m_xSetPageNumberCB;
    std::unique_ptr<weld::SpinButton> m_xSetPageNumberNF;
    std::unique_ptr<weld::ComboBox> m_xPagePropertiesLB;
    std::unique_ptr<weld::Button> m_xPagePropertiesPB;
    std::unique_ptr<weld::Button> m_xOkPB;

    void FillList();

    sal_uInt16 GetInsertPosition() const;

    DECL_LINK(OKHdl, weld::Button&, void);
    DECL_LINK(EditHdl, weld::Button&, void);
    DECL_LINK(RestartNumberingHdl, weld::ToggleButton&, void);
    DECL_LINK(SetPageNumberHdl, weld::ToggleButton&, void);
    DECL_LINK(ValueChangeHdl, weld::SpinButton&, void);
    DECL_LINK(StartPageHdl, weld::ToggleButton&, void);

public:
    SwTitlePageDlg(weld::Window* pParent);
    virtual ~SwTitlePageDlg() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
