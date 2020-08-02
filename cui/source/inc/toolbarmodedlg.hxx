/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef INCLUDED_CUI_INC_TOOLBARMODEDIALOG_HXX
#define INCLUDED_CUI_INC_TOOLBARMODEDIALOG_HXX

#include <vcl/weld.hxx>

class ToolbarmodeDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Image> m_pImage;
    std::unique_ptr<weld::Button> m_pApply;
    std::unique_ptr<weld::RadioButton> m_pRadioButtons[9];
    std::unique_ptr<weld::Label> m_pInfoLabel;

    void UpdateImage(OUString aFileName);
    DECL_LINK(SelectToolbarmode, weld::ToggleButton&, void);
    DECL_LINK(OnApplyClick, weld::Button&, void);

public:
    ToolbarmodeDialog(weld::Window* pWindow);
    virtual ~ToolbarmodeDialog() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */