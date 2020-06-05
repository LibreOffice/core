#pragma once

#include <vcl/weld.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Reference.hxx>

class AdditionsDialog : public weld::GenericDialogController
{
    DECL_LINK(InstallButtonTestHdl, weld::Button&, void);

public:
    AdditionsDialog(weld::Widget* pParent);

protected:
private:
    std::unique_ptr<weld::Button> m_xInstallButton;
    std::unique_ptr<weld::Label> m_xExtensionName;
    std::unique_ptr<weld::Label> m_xAuthor;
    std::unique_ptr<weld::Label> m_xIntro;
    weld::Widget* mpParent;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
