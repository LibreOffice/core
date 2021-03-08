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

#include <uielement/dropdownboxtoolbarcontroller.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <vcl/InterimItemWindow.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>

using namespace ::com::sun::star;
using namespace css::awt;
using namespace css::uno;
using namespace css::beans;
using namespace css::lang;
using namespace css::frame;
using namespace css::util;

namespace framework
{

// Wrapper class to notify controller about events from ListBox.
// Unfortunaltly the events are notified through virtual methods instead
// of Listeners.

class ListBoxControl final : public InterimItemWindow
{
public:
    ListBoxControl(vcl::Window* pParent, DropdownToolbarController* pListBoxListener);
    virtual ~ListBoxControl() override;
    virtual void dispose() override;

    void set_active(int nPos) { m_xWidget->set_active(nPos); }
    void append_text(const OUString& rStr) { m_xWidget->append_text(rStr); }
    void insert_text(int nPos, const OUString& rStr) { m_xWidget->insert_text(nPos, rStr); }
    int get_count() const { return m_xWidget->get_count(); }
    int find_text(const OUString& rStr) const { return m_xWidget->find_text(rStr); }
    OUString get_active_text() const { return m_xWidget->get_active_text(); }
    void clear() { return m_xWidget->clear(); }
    void remove(int nPos) { m_xWidget->remove(nPos); }

    DECL_LINK(FocusInHdl, weld::Widget&, void);
    DECL_LINK(FocusOutHdl, weld::Widget&, void);
    DECL_LINK(ModifyHdl, weld::ComboBox&, void);
    DECL_LINK(KeyInputHdl, const ::KeyEvent&, bool);

private:
    std::unique_ptr<weld::ComboBox> m_xWidget;
    DropdownToolbarController* m_pListBoxListener;
};

ListBoxControl::ListBoxControl(vcl::Window* pParent, DropdownToolbarController* pListBoxListener)
    : InterimItemWindow(pParent, "svt/ui/listcontrol.ui", "ListControl")
    , m_xWidget(m_xBuilder->weld_combo_box("listbox"))
    , m_pListBoxListener( pListBoxListener )
{
    InitControlBase(m_xWidget.get());

    m_xWidget->connect_focus_in(LINK(this, ListBoxControl, FocusInHdl));
    m_xWidget->connect_focus_out(LINK(this, ListBoxControl, FocusOutHdl));
    m_xWidget->connect_changed(LINK(this, ListBoxControl, ModifyHdl));
    m_xWidget->connect_key_press(LINK(this, ListBoxControl, KeyInputHdl));

    m_xWidget->set_size_request(42, -1); // so a later narrow size request can stick
    SetSizePixel(get_preferred_size());
}

IMPL_LINK(ListBoxControl, KeyInputHdl, const ::KeyEvent&, rKEvt, bool)
{
    return ChildKeyInput(rKEvt);
}

ListBoxControl::~ListBoxControl()
{
    disposeOnce();
}

void ListBoxControl::dispose()
{
    m_pListBoxListener = nullptr;
    m_xWidget.reset();
    InterimItemWindow::dispose();
}

IMPL_LINK_NOARG(ListBoxControl, ModifyHdl, weld::ComboBox&, void)
{
    if (m_pListBoxListener)
        m_pListBoxListener->Select();
}

IMPL_LINK_NOARG(ListBoxControl, FocusInHdl, weld::Widget&, void)
{
    if (m_pListBoxListener)
        m_pListBoxListener->GetFocus();
}

IMPL_LINK_NOARG(ListBoxControl, FocusOutHdl, weld::Widget&, void)
{
    if (m_pListBoxListener)
        m_pListBoxListener->LoseFocus();
}

DropdownToolbarController::DropdownToolbarController(
    const Reference< XComponentContext >&    rxContext,
    const Reference< XFrame >&               rFrame,
    ToolBox*                                 pToolbar,
    ToolBoxItemId                            nID,
    sal_Int32                                nWidth,
    const OUString&                          aCommand ) :
    ComplexToolbarController( rxContext, rFrame, pToolbar, nID, aCommand )
    ,   m_pListBoxControl( nullptr )
{
    m_pListBoxControl = VclPtr<ListBoxControl>::Create(m_xToolbar, this);
    if ( nWidth == 0 )
        nWidth = 100;

    // ListBoxControl ctor has set a suitable height already
    auto nHeight = m_pListBoxControl->GetSizePixel().Height();

    m_pListBoxControl->SetSizePixel( ::Size( nWidth, nHeight ));
    m_xToolbar->SetItemWindow( m_nID, m_pListBoxControl );
}

DropdownToolbarController::~DropdownToolbarController()
{
}

void SAL_CALL DropdownToolbarController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    m_xToolbar->SetItemWindow( m_nID, nullptr );
    m_pListBoxControl.disposeAndClear();

    ComplexToolbarController::dispose();
}

Sequence<PropertyValue> DropdownToolbarController::getExecuteArgs(sal_Int16 KeyModifier) const
{
    Sequence<PropertyValue> aArgs( 2 );
    OUString aSelectedText = m_pListBoxControl->get_active_text();

    // Add key modifier to argument list
    aArgs[0].Name = "KeyModifier";
    aArgs[0].Value <<= KeyModifier;
    aArgs[1].Name = "Text";
    aArgs[1].Value <<= aSelectedText;
    return aArgs;
}

void DropdownToolbarController::Select()
{
    if (m_pListBoxControl->get_count() > 0)
        execute(0);
}

void DropdownToolbarController::GetFocus()
{
    notifyFocusGet();
}

void DropdownToolbarController::LoseFocus()
{
    notifyFocusLost();
}

void DropdownToolbarController::executeControlCommand( const css::frame::ControlCommand& rControlCommand )
{
    if ( rControlCommand.Command == "SetList" )
    {
        for ( const NamedValue& rArg : rControlCommand.Arguments )
        {
            if ( rArg.Name == "List" )
            {
                Sequence< OUString > aList;
                m_pListBoxControl->clear();

                rArg.Value >>= aList;
                for (OUString const & rName : std::as_const(aList))
                    m_pListBoxControl->append_text(rName);

                m_pListBoxControl->set_active(0);

                // send notification
                uno::Sequence< beans::NamedValue > aInfo { { "List", css::uno::makeAny(aList) } };
                addNotifyInfo( "ListChanged",
                               getDispatchFromCommand( m_aCommandURL ),
                               aInfo );

                break;
            }
        }
    }
    else if ( rControlCommand.Command == "AddEntry" )
    {
        OUString   aText;
        for ( const NamedValue& rArg : rControlCommand.Arguments )
        {
            if ( rArg.Name == "Text" )
            {
                if ( rArg.Value >>= aText )
                    m_pListBoxControl->append_text(aText);
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "InsertEntry" )
    {
        sal_Int32 nPos(-1);
        OUString   aText;
        for ( const NamedValue& rArg : rControlCommand.Arguments )
        {
            if ( rArg.Name == "Pos" )
            {
                sal_Int32 nTmpPos = 0;
                if ( rArg.Value >>= nTmpPos )
                {
                    if (( nTmpPos >= 0 ) &&
                        ( nTmpPos < m_pListBoxControl->get_count() ))
                        nPos = nTmpPos;
                }
            }
            else if ( rArg.Name == "Text" )
                rArg.Value >>= aText;
        }

        m_pListBoxControl->insert_text(nPos, aText);
    }
    else if ( rControlCommand.Command == "RemoveEntryPos" )
    {
        for ( const NamedValue& rArg : rControlCommand.Arguments )
        {
            if ( rArg.Name == "Pos" )
            {
                sal_Int32 nPos( -1 );
                if ( rArg.Value >>= nPos )
                {
                    if ( 0 <= nPos && nPos < m_pListBoxControl->get_count() )
                        m_pListBoxControl->remove(nPos);
                }
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "RemoveEntryText" )
    {
        for ( const NamedValue& rArg : rControlCommand.Arguments )
        {
            if ( rArg.Name == "Text" )
            {
                OUString aText;
                if ( rArg.Value >>= aText )
                {
                    auto nPos = m_pListBoxControl->find_text(aText);
                    if (nPos != -1)
                        m_pListBoxControl->remove(nPos);
                }
                break;
            }
        }
    }
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
