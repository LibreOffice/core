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

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/status/FontHeight.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>
#include <vcl/window.hxx>
#include <vcl/settings.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <tools/json_writer.hxx>
#include <vcl/toolbox.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <memory>

#include <vcl/InterimItemWindow.hxx>

using namespace ::com::sun::star;

namespace {

class SvxFontSizeBox_Base;
class SvxFontSizeBox_Impl;

class FontHeightToolBoxControl : public svt::ToolboxController,
                                 public lang::XServiceInfo
{
    public:
        explicit FontHeightToolBoxControl(
            const css::uno::Reference< css::uno::XComponentContext >& rServiceManager );

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
        virtual void SAL_CALL acquire() noexcept override;
        virtual void SAL_CALL release() noexcept override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // XComponent
        virtual void SAL_CALL dispose() override;

        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& Event ) override;

        // XToolbarController
        virtual void SAL_CALL execute( sal_Int16 KeyModifier ) override;
        virtual void SAL_CALL click() override;
        virtual void SAL_CALL doubleClick() override;
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createPopupWindow() override;
        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL createItemWindow( const css::uno::Reference< css::awt::XWindow >& Parent ) override;

        void dispatchCommand( const css::uno::Sequence< css::beans::PropertyValue >& rArgs );
        using svt::ToolboxController::dispatchCommand;

    private:
        VclPtr<SvxFontSizeBox_Impl> m_xVclBox;
        std::unique_ptr<SvxFontSizeBox_Base> m_xWeldBox;
        SvxFontSizeBox_Base* m_pBox;
        css::awt::FontDescriptor m_aCurrentFont;
};

class SvxFontSizeBox_Base
{
public:
    SvxFontSizeBox_Base(std::unique_ptr<weld::ComboBox> xWidget,
                        const uno::Reference< frame::XFrame >& _xFrame,
                        FontHeightToolBoxControl& rCtrl);

    virtual ~SvxFontSizeBox_Base()
    {
    }

    virtual void set_sensitive(bool bSensitive)
    {
        m_xWidget->set_sensitive(bSensitive);
    }

    void statusChanged_Impl(tools::Long nHeight, bool bErase);
    void UpdateFont(const css::awt::FontDescriptor& rCurrentFont);

protected:
    FontHeightToolBoxControl& m_rCtrl;
    OUString m_aCurText;
    bool m_bRelease;
    uno::Reference<frame::XFrame> m_xFrame;
    std::unique_ptr<FontSizeBox> m_xWidget;

    void                ReleaseFocus_Impl();
    void                Select();

    virtual bool DoKeyInput(const KeyEvent& rKEvt);

    DECL_LINK(SelectHdl, weld::ComboBox&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(ActivateHdl, weld::ComboBox&, bool);
    DECL_LINK(FocusOutHdl, weld::Widget&, void);
    DECL_LINK(DumpAsPropertyTreeHdl, tools::JsonWriter&, void);
};

class SvxFontSizeBox_Impl final : public InterimItemWindow
                                , public SvxFontSizeBox_Base
{
public:
    SvxFontSizeBox_Impl(vcl::Window* pParent,
                        const uno::Reference< frame::XFrame >& _xFrame,
                        FontHeightToolBoxControl& rCtrl);

    virtual void dispose() override
    {
        m_xWidget.reset();
        InterimItemWindow::dispose();
    }

    virtual void GetFocus() override
    {
        if (m_xWidget)
            m_xWidget->grab_focus();
        InterimItemWindow::GetFocus();
    }

    virtual ~SvxFontSizeBox_Impl() override
    {
        disposeOnce();
    }

    void SetOptimalSize();

    virtual void DataChanged(const DataChangedEvent& rDCEvt) override;

    virtual void set_sensitive(bool bSensitive) override
    {
        m_xWidget->set_sensitive(bSensitive);
        if (bSensitive)
            InterimItemWindow::Enable();
        else
            InterimItemWindow::Disable();
    }

private:
    virtual bool DoKeyInput(const KeyEvent& rKEvt) override;
};

SvxFontSizeBox_Base::SvxFontSizeBox_Base(std::unique_ptr<weld::ComboBox> xWidget,
                                         const uno::Reference<frame::XFrame>& rFrame,
                                         FontHeightToolBoxControl& rCtrl)
    : m_rCtrl(rCtrl)
    , m_bRelease(true)
    , m_xFrame(rFrame)
    , m_xWidget(new FontSizeBox(std::move(xWidget)))
{
    m_xWidget->set_value(0);
    m_xWidget->set_active_or_entry_text("");
    m_xWidget->disable_entry_completion();

    m_xWidget->connect_changed(LINK(this, SvxFontSizeBox_Base, SelectHdl));
    m_xWidget->connect_key_press(LINK(this, SvxFontSizeBox_Base, KeyInputHdl));
    m_xWidget->connect_entry_activate(LINK(this, SvxFontSizeBox_Base, ActivateHdl));
    m_xWidget->connect_focus_out(LINK(this, SvxFontSizeBox_Base, FocusOutHdl));
    m_xWidget->connect_get_property_tree(LINK(this, SvxFontSizeBox_Base, DumpAsPropertyTreeHdl));
}

void SvxFontSizeBox_Base::ReleaseFocus_Impl()
{
    if ( !m_bRelease )
    {
        m_bRelease = true;
        return;
    }

    if ( m_xFrame.is() && m_xFrame->getContainerWindow().is() )
        m_xFrame->getContainerWindow()->setFocus();
}

IMPL_LINK(SvxFontSizeBox_Base, SelectHdl, weld::ComboBox&, rCombo, void)
{
    if (rCombo.changed_by_direct_pick()) // only when picked from the list
        Select();
}

IMPL_LINK_NOARG(SvxFontSizeBox_Base, ActivateHdl, weld::ComboBox&, bool)
{
    Select();
    return true;
}

void SvxFontSizeBox_Base::Select()
{
    sal_Int64 nSelVal = m_xWidget->get_value();
    float fSelVal     = float( nSelVal ) / 10;

    uno::Sequence< beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name  = "FontHeight.Height";
    aArgs[0].Value <<= fSelVal;

    /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() call.
        This instance may be deleted in the meantime (i.e. when a dialog is opened
        while in Dispatch()), accessing members will crash in this case. */
    ReleaseFocus_Impl();

    m_rCtrl.dispatchCommand( aArgs );
}

void SvxFontSizeBox_Base::statusChanged_Impl( tools::Long nPoint, bool bErase )
{
    if ( !bErase )
    {
        // convert the metric
        tools::Long nVal = nPoint;

        // changed => set new value
        if (m_xWidget->get_value() != nVal)
            m_xWidget->set_value(nVal);
    }
    else
    {
        // delete value in the display
        m_xWidget->set_value(-1L);
        m_xWidget->set_active_or_entry_text("");
    }
    m_aCurText = m_xWidget->get_active_text();
}

void SvxFontSizeBox_Base::UpdateFont(const css::awt::FontDescriptor& rCurrentFont)
{
    // filling up the sizes list
    auto nOldVal = m_xWidget->get_value(); // memorize old value
    std::unique_ptr<FontList> xFontList(new FontList(Application::GetDefaultDevice()));

    if (!rCurrentFont.Name.isEmpty())
    {
        FontMetric aFontMetric;
        aFontMetric.SetFamilyName(rCurrentFont.Name);
        aFontMetric.SetStyleName(rCurrentFont.StyleName);
        aFontMetric.SetFontHeight(rCurrentFont.Height);
        m_xWidget->Fill(&aFontMetric, xFontList.get());
    }
    else
    {
        m_xWidget->Fill(nullptr, xFontList.get());
    }
    m_xWidget->set_value(nOldVal); // restore old value
    m_aCurText = m_xWidget->get_active_text(); // memorize to reset at ESC
}

IMPL_LINK(SvxFontSizeBox_Base, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return DoKeyInput(rKEvt);
}

bool SvxFontSizeBox_Base::DoKeyInput(const KeyEvent& rKEvt)
{
    bool bHandled = false;

    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();

    switch (nCode)
    {
        case KEY_TAB:
            m_bRelease = false;
            Select();
            break;

        case KEY_ESCAPE:
            m_xWidget->set_active_or_entry_text(m_aCurText);
            if (!m_rCtrl.IsInSidebar())
            {
                ReleaseFocus_Impl();
                bHandled = true;
            }
            break;
    }

    return bHandled;
}

bool SvxFontSizeBox_Impl::DoKeyInput(const KeyEvent& rKEvt)
{
    return SvxFontSizeBox_Base::DoKeyInput(rKEvt) || ChildKeyInput(rKEvt);
}

IMPL_LINK_NOARG(SvxFontSizeBox_Base, FocusOutHdl, weld::Widget&, void)
{
    if (!m_xWidget->has_focus()) // a combobox can be comprised of different subwidget so double-check if none of those has focus
        m_xWidget->set_active_or_entry_text(m_aCurText);
}

void SvxFontSizeBox_Impl::SetOptimalSize()
{
    SetSizePixel(get_preferred_size());
}

SvxFontSizeBox_Impl::SvxFontSizeBox_Impl(vcl::Window* pParent,
                                         const uno::Reference<frame::XFrame>& rFrame,
                                         FontHeightToolBoxControl& rCtrl)
    : InterimItemWindow(pParent, "svx/ui/fontsizebox.ui", "FontSizeBox")
    , SvxFontSizeBox_Base(m_xBuilder->weld_combo_box("fontsizecombobox"), rFrame, rCtrl)
{
}

void SvxFontSizeBox_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        SetOptimalSize();
    }
}

IMPL_LINK(SvxFontSizeBox_Base, DumpAsPropertyTreeHdl, tools::JsonWriter&, rJsonWriter, void)
{
    {
        auto entriesNode = rJsonWriter.startNode("entries");
        for (int i = 0, nCount = m_xWidget->get_count(); i < nCount; ++i)
        {
            auto entryNode = rJsonWriter.startNode("");
            rJsonWriter.put("", m_xWidget->get_text(i));
        }
    }

    int nActive = m_xWidget->get_active();
    rJsonWriter.put("selectedCount", static_cast<sal_Int32>(nActive == -1 ? 0 : 1));
    {
        auto selectedNode = rJsonWriter.startNode("selectedEntries");
        if (nActive != -1)
        {
            auto node = rJsonWriter.startNode("");
            rJsonWriter.put("", static_cast<sal_Int32>(nActive));
        }
    }

    rJsonWriter.put("command", ".uno:FontHeight");
}

FontHeightToolBoxControl::FontHeightToolBoxControl( const uno::Reference< uno::XComponentContext >& rxContext )
 : svt::ToolboxController( rxContext,
                           uno::Reference< frame::XFrame >(),
                           ".uno:FontHeight" ),
   m_pBox( nullptr )
{
    addStatusListener( ".uno:CharFontName");
}

// XInterface
css::uno::Any SAL_CALL FontHeightToolBoxControl::queryInterface( const css::uno::Type& aType )
{
    uno::Any a = ToolboxController::queryInterface( aType );
    if ( a.hasValue() )
        return a;

    return ::cppu::queryInterface( aType, static_cast< lang::XServiceInfo* >( this ));
}

void SAL_CALL FontHeightToolBoxControl::acquire() noexcept
{
    ToolboxController::acquire();
}

void SAL_CALL FontHeightToolBoxControl::release() noexcept
{
    ToolboxController::release();
}

// XServiceInfo
sal_Bool SAL_CALL FontHeightToolBoxControl::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

OUString SAL_CALL FontHeightToolBoxControl::getImplementationName()
{
    return "com.sun.star.svx.FontHeightToolBoxController";
}

uno::Sequence< OUString > SAL_CALL FontHeightToolBoxControl::getSupportedServiceNames(  )
{
    return { "com.sun.star.frame.ToolbarController" };
}

// XComponent
void SAL_CALL FontHeightToolBoxControl::dispose()
{
    svt::ToolboxController::dispose();

    SolarMutexGuard aSolarMutexGuard;
    m_xVclBox.disposeAndClear();
    m_xWeldBox.reset();
    m_pBox = nullptr;
}

// XStatusListener
void SAL_CALL FontHeightToolBoxControl::statusChanged(
    const frame::FeatureStateEvent& rEvent )
{
    if ( !m_pBox )
        return;

    SolarMutexGuard aSolarMutexGuard;
    if (rEvent.FeatureURL.Path == "FontHeight")
    {
        if ( rEvent.IsEnabled )
        {
            m_pBox->set_sensitive(true);
            frame::status::FontHeight aFontHeight;
            if ( rEvent.State >>= aFontHeight )
                m_pBox->statusChanged_Impl( tools::Long( 10. * aFontHeight.Height ), false );
            else
                m_pBox->statusChanged_Impl( tools::Long( -1 ), true );
        }
        else
        {
            m_pBox->set_sensitive(false);
            m_pBox->statusChanged_Impl( tools::Long( -1 ), true );
        }

        if (m_pToolbar)
            m_pToolbar->set_item_sensitive(m_aCommandURL.toUtf8(), rEvent.IsEnabled);
        else
        {
            ToolBox* pToolBox = nullptr;
            ToolBoxItemId nId;
            if (getToolboxId(nId, &pToolBox))
                pToolBox->EnableItem(nId, rEvent.IsEnabled);
        }
    }
    else if ( rEvent.FeatureURL.Path == "CharFontName" )
    {
        if ( rEvent.State >>= m_aCurrentFont )
            m_pBox->UpdateFont( m_aCurrentFont );
    }
}

// XToolbarController
void SAL_CALL FontHeightToolBoxControl::execute( sal_Int16 /*KeyModifier*/ )
{
}

void SAL_CALL FontHeightToolBoxControl::click()
{
}

void SAL_CALL FontHeightToolBoxControl::doubleClick()
{
}

uno::Reference< awt::XWindow > SAL_CALL FontHeightToolBoxControl::createPopupWindow()
{
    return uno::Reference< awt::XWindow >();
}

uno::Reference< awt::XWindow > SAL_CALL FontHeightToolBoxControl::createItemWindow(
    const uno::Reference< awt::XWindow >& xParent )
{
    uno::Reference< awt::XWindow > xItemWindow;

    if (m_pBuilder)
    {
        SolarMutexGuard aSolarMutexGuard;

        std::unique_ptr<weld::ComboBox> xWidget(m_pBuilder->weld_combo_box("fontsizecombobox"));

        xItemWindow = css::uno::Reference<css::awt::XWindow>(new weld::TransportAsXWindow(xWidget.get()));

        m_xWeldBox.reset(new SvxFontSizeBox_Base(std::move(xWidget), m_xFrame, *this));
        m_pBox = m_xWeldBox.get();
        //Get the box to fill itself with all its sizes
        m_pBox->UpdateFont(m_aCurrentFont);
    }
    else
    {
        VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow( xParent );
        if ( pParent )
        {
            SolarMutexGuard aSolarMutexGuard;
            m_xVclBox = VclPtr<SvxFontSizeBox_Impl>::Create( pParent, m_xFrame, *this );
            m_pBox = m_xVclBox.get();
            //Get the box to fill itself with all its sizes
            m_pBox->UpdateFont(m_aCurrentFont);
            //Make it size itself to its optimal size re above sizes
            m_xVclBox->SetOptimalSize();
            xItemWindow = VCLUnoHelper::GetInterface(m_xVclBox);
        }
    }

    return xItemWindow;
}

void FontHeightToolBoxControl::dispatchCommand(
    const uno::Sequence< beans::PropertyValue >& rArgs )
{
    uno::Reference< frame::XDispatchProvider > xDispatchProvider( m_xFrame, uno::UNO_QUERY );
    if ( xDispatchProvider.is() )
    {
        util::URL                               aURL;
        uno::Reference< frame::XDispatch >      xDispatch;
        uno::Reference< util::XURLTransformer > xURLTransformer = getURLTransformer();

        aURL.Complete = ".uno:FontHeight";
        xURLTransformer->parseStrict( aURL );
        xDispatch = xDispatchProvider->queryDispatch( aURL, OUString(), 0 );
        if ( xDispatch.is() )
            xDispatch->dispatch( aURL, rArgs );
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_svx_FontHeightToolBoxController_get_implementation(
    css::uno::XComponentContext *rxContext,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new FontHeightToolBoxControl(rxContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
