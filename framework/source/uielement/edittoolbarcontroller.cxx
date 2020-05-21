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

#include <uielement/edittoolbarcontroller.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/InterimItemWindow.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/event.hxx>

using namespace ::com::sun::star;
using namespace css::uno;
using namespace css::beans;
using namespace css::lang;
using namespace css::frame;
using namespace css::util;

namespace framework
{

// Wrapper class to notify controller about events from edit.
// Unfortunaltly the events are notified through virtual methods instead
// of Listeners.

class EditControl final : public InterimItemWindow
{
public:
    EditControl(vcl::Window* pParent, EditToolbarController* pEditToolbarController);
    virtual ~EditControl() override;
    virtual void dispose() override;

    OUString get_text() const { return m_xWidget->get_text(); }
    void set_text(const OUString& rText) { m_xWidget->set_text(rText); }

private:
    std::unique_ptr<weld::Entry> m_xWidget;
    EditToolbarController* m_pEditToolbarController;

    DECL_LINK(FocusInHdl, weld::Widget&, void);
    DECL_LINK(FocusOutHdl, weld::Widget&, void);
    DECL_LINK(ModifyHdl, weld::Entry&, void);
    DECL_LINK(ActivateHdl, weld::Entry&, bool);
};

EditControl::EditControl(vcl::Window* pParent, EditToolbarController* pEditToolbarController)
    : InterimItemWindow(pParent, "svt/ui/editcontrol.ui", "EditControl")
    , m_xWidget(m_xBuilder->weld_entry("entry"))
    , m_pEditToolbarController(pEditToolbarController)
{
    m_xWidget->connect_focus_in(LINK(this, EditControl, FocusInHdl));
    m_xWidget->connect_focus_out(LINK(this, EditControl, FocusOutHdl));
    m_xWidget->connect_changed(LINK(this, EditControl, ModifyHdl));
    m_xWidget->connect_activate(LINK(this, EditControl, ActivateHdl));

    SetSizePixel(get_preferred_size());
}

EditControl::~EditControl()
{
    disposeOnce();
}

void EditControl::dispose()
{
    m_pEditToolbarController = nullptr;
    m_xWidget.reset();
    InterimItemWindow::dispose();
}

IMPL_LINK_NOARG(EditControl, ModifyHdl, weld::Entry&, void)
{
    if (m_pEditToolbarController)
        m_pEditToolbarController->Modify();
}

IMPL_LINK_NOARG(EditControl, FocusInHdl, weld::Widget&, void)
{
    if (m_pEditToolbarController)
        m_pEditToolbarController->GetFocus();
}

IMPL_LINK_NOARG(EditControl, FocusOutHdl, weld::Widget&, void)
{
    if ( m_pEditToolbarController )
        m_pEditToolbarController->LoseFocus();
}

IMPL_LINK_NOARG(EditControl, ActivateHdl, weld::Entry&, bool)
{
    if (m_pEditToolbarController)
        m_pEditToolbarController->Activate();
    return true;
}

EditToolbarController::EditToolbarController(
    const Reference< XComponentContext >&    rxContext,
    const Reference< XFrame >&               rFrame,
    ToolBox*                                 pToolbar,
    sal_uInt16                                   nID,
    sal_Int32                                nWidth,
    const OUString&                          aCommand ) :
    ComplexToolbarController( rxContext, rFrame, pToolbar, nID, aCommand )
    ,   m_pEditControl( nullptr )
{
    m_pEditControl = VclPtr<EditControl>::Create(m_xToolbar, this);
    if ( nWidth == 0 )
        nWidth = 100;

    // EditControl ctor has set a suitable height already
    auto nHeight = m_pEditControl->GetSizePixel().Height();

    m_pEditControl->SetSizePixel( ::Size( nWidth, nHeight ));
    m_xToolbar->SetItemWindow( m_nID, m_pEditControl );
}

EditToolbarController::~EditToolbarController()
{
}

void SAL_CALL EditToolbarController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    m_xToolbar->SetItemWindow( m_nID, nullptr );
    m_pEditControl.disposeAndClear();

    ComplexToolbarController::dispose();
}

Sequence<PropertyValue> EditToolbarController::getExecuteArgs(sal_Int16 KeyModifier) const
{
    Sequence<PropertyValue> aArgs( 2 );
    OUString aSelectedText = m_pEditControl->get_text();

    // Add key modifier to argument list
    aArgs[0].Name = "KeyModifier";
    aArgs[0].Value <<= KeyModifier;
    aArgs[1].Name = "Text";
    aArgs[1].Value <<= aSelectedText;
    return aArgs;
}

void EditToolbarController::Modify()
{
    notifyTextChanged(m_pEditControl->get_text());
}

void EditToolbarController::GetFocus()
{
    notifyFocusGet();
}

void EditToolbarController::LoseFocus()
{
    notifyFocusLost();
}

void EditToolbarController::Activate()
{
    // Call execute only with non-empty text
    if (!m_pEditControl->get_text().isEmpty())
        execute(0);
}

void EditToolbarController::executeControlCommand( const css::frame::ControlCommand& rControlCommand )
{
    if ( !rControlCommand.Command.startsWith( "SetText" ))
        return;

    for ( const NamedValue& rArg : rControlCommand.Arguments )
    {
        if ( rArg.Name.startsWith( "Text" ))
        {
            OUString aText;
            rArg.Value >>= aText;
            m_pEditControl->set_text(aText);

            // send notification
            notifyTextChanged( aText );
            break;
        }
    }
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
