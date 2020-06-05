#pragma once

#include <vcl/weld.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>

#include <osl/mutex.hxx>
#include <cppuhelper/implbase.hxx>
class AdditionsDialog : public weld::GenericDialogController
{
public:
    AdditionsDialog(weld::Widget* pParent);
    virtual ~AdditionsDialog() override;

private:
    std::unique_ptr<weld::Button> m_xInstallButton;
    std::unique_ptr<weld::Label> m_xExtensionName;
    std::unique_ptr<weld::Label> m_xAuthor;
    std::unique_ptr<weld::Label> m_xIntro;
    DECL_LINK(InstallButtonTestHdl, weld::Button&, void);
};

class TmpRepositoryCommandEnv
    : public ::cppu::WeakImplHelper<css::ucb::XCommandEnvironment, css::task::XInteractionHandler,
                                    css::ucb::XProgressHandler>
{
public:
    virtual ~TmpRepositoryCommandEnv() override;
    TmpRepositoryCommandEnv();

    // XCommandEnvironment
    virtual css::uno::Reference<css::task::XInteractionHandler>
        SAL_CALL getInteractionHandler() override;
    virtual css::uno::Reference<css::ucb::XProgressHandler> SAL_CALL getProgressHandler() override;

    // XInteractionHandler
    virtual void SAL_CALL
    handle(css::uno::Reference<css::task::XInteractionRequest> const& xRequest) override;

    // XProgressHandler
    virtual void SAL_CALL push(css::uno::Any const& Status) override;
    virtual void SAL_CALL update(css::uno::Any const& Status) override;
    virtual void SAL_CALL pop() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
