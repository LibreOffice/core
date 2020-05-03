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

#include <uielement/comboboxtoolbarcontroller.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/Color.hpp>

#include <svtools/InterimItemWindow.hxx>
#include <svtools/toolboxcontroller.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>

using namespace ::com::sun::star;
using namespace css::uno;
using namespace css::beans;
using namespace css::lang;
using namespace css::frame;
using namespace css::util;

namespace framework
{

// Wrapper class to notify controller about events from combobox.
// Unfortunately the events are notified through virtual methods instead
// of Listeners.

class ComboBoxControl final : public InterimItemWindow
{
public:
    ComboBoxControl(vcl::Window* pParent, ComboboxToolbarController* pComboboxToolbarController);
    virtual ~ComboBoxControl() override;
    virtual void dispose() override;

    void set_active_or_entry_text(const OUString& rText);
    OUString get_active_text() const { return m_xWidget->get_active_text(); }

    void clear() { m_xWidget->clear(); }
    void remove(int nIndex) { m_xWidget->remove(nIndex); }
    void append_text(const OUString& rStr) { m_xWidget->append_text(rStr); }
    void insert_text(int pos, const OUString& rStr) { m_xWidget->insert_text(pos, rStr); }
    int get_count() const { return m_xWidget->get_count(); }
    int find_text(const OUString& rStr) const { return m_xWidget->find_text(rStr); }

    DECL_LINK(FocusInHdl, weld::Widget&, void);
    DECL_LINK(FocusOutHdl, weld::Widget&, void);
    DECL_LINK(ModifyHdl, weld::ComboBox&, void);
    DECL_LINK(ActivateHdl, weld::ComboBox&, bool);

private:
    std::unique_ptr<weld::ComboBox> m_xWidget;
    ComboboxToolbarController* m_pComboboxToolbarController;
};

ComboBoxControl::ComboBoxControl(vcl::Window* pParent, ComboboxToolbarController* pComboboxToolbarController)
    : InterimItemWindow(pParent, "svt/ui/combocontrol.ui", "ComboControl")
    , m_xWidget(m_xBuilder->weld_combo_box("combobox"))
    , m_pComboboxToolbarController(pComboboxToolbarController)
{
    m_xWidget->connect_focus_in(LINK(this, ComboBoxControl, FocusInHdl));
    m_xWidget->connect_focus_out(LINK(this, ComboBoxControl, FocusOutHdl));
    m_xWidget->connect_changed(LINK(this, ComboBoxControl, ModifyHdl));
    m_xWidget->connect_entry_activate(LINK(this, ComboBoxControl, ActivateHdl));

    m_xWidget->set_entry_width_chars(1); // so a smaller that default width can be used by ComboboxToolbarController
    SetSizePixel(get_preferred_size());
}

void ComboBoxControl::set_active_or_entry_text(const OUString& rText)
{
    const int nFound = m_xWidget->find_text(rText);
    if (nFound != -1)
        m_xWidget->set_active(nFound);
    else
        m_xWidget->set_entry_text(rText);
}

ComboBoxControl::~ComboBoxControl()
{
    disposeOnce();
}

void ComboBoxControl::dispose()
{
    m_pComboboxToolbarController = nullptr;
    m_xWidget.reset();
    InterimItemWindow::dispose();
}

IMPL_LINK_NOARG(ComboBoxControl, ModifyHdl, weld::ComboBox&, void)
{
    if (m_pComboboxToolbarController)
    {
        if (m_xWidget->get_count() && m_xWidget->changed_by_direct_pick())
            m_pComboboxToolbarController->Select();
        else
            m_pComboboxToolbarController->Modify();
    }
}

IMPL_LINK_NOARG(ComboBoxControl, FocusInHdl, weld::Widget&, void)
{
    if (m_pComboboxToolbarController)
        m_pComboboxToolbarController->GetFocus();
}

IMPL_LINK_NOARG(ComboBoxControl, FocusOutHdl, weld::Widget&, void)
{
    if (m_pComboboxToolbarController)
        m_pComboboxToolbarController->LoseFocus();
}

IMPL_LINK_NOARG(ComboBoxControl, ActivateHdl, weld::ComboBox&, bool)
{
    if (m_pComboboxToolbarController)
        m_pComboboxToolbarController->Activate();
    return true;
}

ComboboxToolbarController::ComboboxToolbarController(
    const Reference< XComponentContext >& rxContext,
    const Reference< XFrame >&            rFrame,
    ToolBox*                              pToolbar,
    sal_uInt16                            nID,
    sal_Int32                             nWidth,
    const OUString&                       aCommand ) :
    ComplexToolbarController( rxContext, rFrame, pToolbar, nID, aCommand )
    ,   m_pComboBox( nullptr )
{
    m_pComboBox = VclPtr<ComboBoxControl>::Create(m_xToolbar, this);
    if ( nWidth == 0 )
        nWidth = 100;

    // ComboBoxControl ctor has set a suitable height already
    auto nHeight = m_pComboBox->GetSizePixel().Height();

    m_pComboBox->SetSizePixel( ::Size( nWidth, nHeight ));
    m_xToolbar->SetItemWindow( m_nID, m_pComboBox );
}

ComboboxToolbarController::~ComboboxToolbarController()
{
}

void SAL_CALL ComboboxToolbarController::dispose()
{
    SolarMutexGuard aSolarMutexGuard;

    m_xToolbar->SetItemWindow( m_nID, nullptr );
    m_pComboBox.disposeAndClear();

    ComplexToolbarController::dispose();
}

Sequence<PropertyValue> ComboboxToolbarController::getExecuteArgs(sal_Int16 KeyModifier) const
{
    Sequence<PropertyValue> aArgs( 2 );
    OUString aSelectedText = m_pComboBox->get_active_text();

    // Add key modifier to argument list
    aArgs[0].Name = "KeyModifier";
    aArgs[0].Value <<= KeyModifier;
    aArgs[1].Name = "Text";
    aArgs[1].Value <<= aSelectedText;
    return aArgs;
}

void ComboboxToolbarController::Select()
{
    vcl::Window::PointerState aState = m_pComboBox->GetPointerState();

    sal_uInt16 nKeyModifier = sal_uInt16( aState.mnState & KEY_MODIFIERS_MASK );
    execute( nKeyModifier );
}

void ComboboxToolbarController::Modify()
{
    notifyTextChanged(m_pComboBox->get_active_text());
}

void ComboboxToolbarController::GetFocus()
{
    notifyFocusGet();
}

void ComboboxToolbarController::LoseFocus()
{
    notifyFocusLost();
}

void ComboboxToolbarController::Activate()
{
    // Call execute only with non-empty text
    if (!m_pComboBox->get_active_text().isEmpty())
        execute(0);
}

void ComboboxToolbarController::executeControlCommand( const css::frame::ControlCommand& rControlCommand )
{
    if ( rControlCommand.Command == "SetText" )
    {
        for ( const NamedValue& rArg : rControlCommand.Arguments )
        {
            if ( rArg.Name == "Text" )
            {
                OUString aText;
                rArg.Value >>= aText;
                m_pComboBox->set_active_or_entry_text(aText);

                // send notification
                notifyTextChanged( aText );
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "SetList" )
    {
        for ( const NamedValue& rArg : rControlCommand.Arguments )
        {
            if ( rArg.Name == "List" )
            {
                Sequence< OUString > aList;
                m_pComboBox->clear();

                rArg.Value >>= aList;
                for (OUString const & rName : std::as_const(aList))
                    m_pComboBox->append_text(rName);

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
                    m_pComboBox->append_text(aText);
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
                        ( nTmpPos < m_pComboBox->get_count() ))
                        nPos = nTmpPos;
                }
            }
            else if ( rArg.Name == "Text" )
                rArg.Value >>= aText;
        }

        m_pComboBox->insert_text(nPos, aText);
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
                    if (0 <= nPos && nPos < m_pComboBox->get_count())
                        m_pComboBox->remove(nPos);
                }
                break;
            }
        }
    }
    else if ( rControlCommand.Command == "RemoveEntryText" )
    {
        for ( const NamedValue& rArg : rControlCommand.Arguments )
        {
            if ( rArg.Name == "Text")
            {
                OUString aText;
                if ( rArg.Value >>= aText )
                {
                    auto nPos = m_pComboBox->find_text(aText);
                    if (nPos != -1)
                        m_pComboBox->remove(nPos);
                }
                break;
            }
        }
    }
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
