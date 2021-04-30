/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "limitboxcontroller.hxx"
#include <apitools.hxx>

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <vcl/InterimItemWindow.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <core_resource.hxx>
#include <strings.hrc>

using namespace ::com::sun::star;

namespace
{

/// Default values
sal_Int64 const aDefLimitAry[] =
{
    5,
    10,
    20,
    50
};

}

namespace dbaui
{

/**
 * Input box to add limit to an SQL query (maximum number of result's rows)
 * This box is reachable on the Query Design Toolbar
 */
class LimitBox final : public InterimItemWindow
{
public:
    LimitBox(vcl::Window* pParent, LimitBoxController* pCtrl)
        : InterimItemWindow(pParent, "dbaccess/ui/limitbox.ui", "LimitBox")
        , m_pControl( pCtrl )
        , m_xWidget(m_xBuilder->weld_combo_box("limit"))
    {
        InitControlBase(m_xWidget.get());

        LoadDefaultLimits();

        m_xWidget->connect_key_press(LINK(this, LimitBox, KeyInputHdl));
        m_xWidget->connect_entry_activate(LINK(this, LimitBox, ActivateHdl));
        m_xWidget->connect_changed(LINK(this, LimitBox, ChangeHdl));
        m_xWidget->connect_focus_out(LINK(this, LimitBox, FocusOutHdl));
        m_xWidget->set_entry_width_chars(6);
        SetSizePixel(m_xContainer->get_preferred_size());
    }

    virtual void dispose() override
    {
        m_xWidget.reset();
        InterimItemWindow::dispose();
    }

    virtual ~LimitBox() override
    {
        disposeOnce();
    }

    void set_sensitive(bool bSensitive)
    {
        m_xWidget->set_sensitive(bSensitive);
    }

    void set_value(int nLimit)
    {
        if (nLimit < 0)
            m_xWidget->set_active(0);
        else
            m_xWidget->set_entry_text(OUString::number(nLimit));
        m_xWidget->save_value();
    }

private:
    LimitBoxController* m_pControl;
    std::unique_ptr<weld::ComboBox> m_xWidget;

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(ActivateHdl, weld::ComboBox&, bool);
    DECL_LINK(ChangeHdl, weld::ComboBox&, void);
    DECL_LINK(FocusOutHdl, weld::Widget&, void);

    void Apply()
    {
        if (!m_xWidget->get_value_changed_from_saved())
            return;
        uno::Sequence< beans::PropertyValue > aArgs( 1 );
        aArgs[0].Name  = "DBLimit.Value";
        sal_Int64 nLimit;
        OUString sActiveText = m_xWidget->get_active_text();
        if (sActiveText == DBA_RES(STR_QUERY_LIMIT_ALL))
            nLimit = -1;
        else
        {
            nLimit = m_xWidget->get_active_text().toInt64();
            if (nLimit < 0)
                nLimit = -1;
        }
        set_value(nLimit);
        aArgs[0].Value <<= nLimit;
        m_pControl->dispatchCommand( aArgs );
    }

    ///Initialize entries
    void LoadDefaultLimits()
    {
        m_xWidget->freeze();
        m_xWidget->append_text(DBA_RES(STR_QUERY_LIMIT_ALL));
        for (auto nIndex : aDefLimitAry)
        {
            m_xWidget->append_text(OUString::number(nIndex));
        }
        m_xWidget->thaw();
    }
};

IMPL_LINK(LimitBox, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    bool bHandled = false;
    const sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();
    switch (nCode)
    {
        case KEY_ESCAPE:
            m_xWidget->set_entry_text(m_xWidget->get_saved_value());
            bHandled = true;
            break;
        case KEY_RETURN:
        {
            bHandled = ActivateHdl(*m_xWidget);
            break;
        }
    }
    return bHandled || ChildKeyInput(rKEvt);
}

IMPL_LINK_NOARG(LimitBox, FocusOutHdl, weld::Widget&, void)
{
    if (!m_xWidget || m_xWidget->has_focus()) // comboboxes can be comprised of multiple widgets, ensure all have lost focus
        return;
    Apply();
}

IMPL_LINK(LimitBox, ChangeHdl, weld::ComboBox&, rComboBox, void)
{
    if (rComboBox.changed_by_direct_pick())
        ActivateHdl(rComboBox);
}

IMPL_LINK_NOARG(LimitBox, ActivateHdl, weld::ComboBox&, bool)
{
    GrabFocusToDocument();
    Apply();
    return true;
}

LimitBoxController::LimitBoxController(
    const uno::Reference< uno::XComponentContext >& rxContext ) :
    svt::ToolboxController( rxContext,
                            uno::Reference< frame::XFrame >(),
                            ".uno:DBLimit" ),
    m_xLimitBox( nullptr )
{
}

LimitBoxController::~LimitBoxController()
{
}

/// XInterface
uno::Any SAL_CALL LimitBoxController::queryInterface( const uno::Type& aType )
{
    uno::Any a = ToolboxController::queryInterface( aType );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< lang::XServiceInfo* >( this ));
}

void SAL_CALL LimitBoxController::acquire() noexcept
{
    ToolboxController::acquire();
}

void SAL_CALL LimitBoxController::release() noexcept
{
    ToolboxController::release();
}


/// XServiceInfo
OUString SAL_CALL LimitBoxController::getImplementationName()
{
    return "org.libreoffice.comp.dbu.LimitBoxController";
}

IMPLEMENT_SERVICE_INFO_SUPPORTS(LimitBoxController)

css::uno::Sequence< OUString > SAL_CALL LimitBoxController::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

/// XComponent
void SAL_CALL LimitBoxController::dispose()
{
    svt::ToolboxController::dispose();

    SolarMutexGuard aSolarMutexGuard;
    m_xLimitBox.disposeAndClear();
}

/// XStatusListener
void SAL_CALL LimitBoxController::statusChanged(
    const frame::FeatureStateEvent& rEvent )
{
    if ( !m_xLimitBox )
        return;

    SolarMutexGuard aSolarMutexGuard;
    if ( rEvent.FeatureURL.Path == "DBLimit" )
    {
        if ( rEvent.IsEnabled )
        {
            m_xLimitBox->set_sensitive(true);
            sal_Int64 nLimit = 0;
            if (rEvent.State >>= nLimit)
                m_xLimitBox->set_value(nLimit);
        }
        else
            m_xLimitBox->set_sensitive(false);
    }
}

/// XToolbarController
void SAL_CALL LimitBoxController::execute( sal_Int16 /*KeyModifier*/ )
{
}

void SAL_CALL LimitBoxController::click()
{
}

void SAL_CALL LimitBoxController::doubleClick()
{
}

uno::Reference< awt::XWindow > SAL_CALL LimitBoxController::createPopupWindow()
{
    return uno::Reference< awt::XWindow >();
}

uno::Reference< awt::XWindow > SAL_CALL LimitBoxController::createItemWindow(
    const uno::Reference< awt::XWindow >& xParent )
{
    uno::Reference< awt::XWindow > xItemWindow;

    VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow( xParent );
    if ( pParent )
    {
        SolarMutexGuard aSolarMutexGuard;
        m_xLimitBox = VclPtr<LimitBox>::Create(pParent, this);
        xItemWindow = VCLUnoHelper::GetInterface(m_xLimitBox);
    }

    return xItemWindow;
}

void LimitBoxController::dispatchCommand(
    const uno::Sequence< beans::PropertyValue >& rArgs )
{
    uno::Reference< frame::XDispatchProvider > xDispatchProvider( m_xFrame, uno::UNO_QUERY );
    if ( xDispatchProvider.is() )
    {
        util::URL                               aURL;
        uno::Reference< frame::XDispatch >      xDispatch;
        uno::Reference< util::XURLTransformer > xURLTransformer = getURLTransformer();

        aURL.Complete = ".uno:DBLimit";
        xURLTransformer->parseStrict( aURL );
        xDispatch = xDispatchProvider->queryDispatch( aURL, OUString(), 0 );
        if ( xDispatch.is() )
            xDispatch->dispatch( aURL, rArgs );
    }
}

} // dbaui namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_libreoffice_comp_dbu_LimitBoxController_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new ::dbaui::LimitBoxController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
