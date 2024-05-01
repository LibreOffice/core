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

#include <sal/config.h>

#include <sal/types.h>
#include <vcl/event.hxx>
#include <vcl/toolbox.hxx>
#include <sfx2/bindings.hxx>
#include <svtools/toolbarmenu.hxx>
#include <svx/dialmgr.hxx>
#include <lboxctrl.hxx>
#include <tools/urlobj.hxx>

#include <svx/strings.hrc>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>

#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;

class SvxPopupWindowListBox final : public WeldToolbarPopup
{
    rtl::Reference<SvxUndoRedoControl> m_xControl;
    std::unique_ptr<weld::TreeView> m_xListBox;
    std::unique_ptr<weld::TreeIter> m_xScratchIter;
    int m_nSelectedRows;
    int m_nVisRows;

    void UpdateRow(int nRow);

    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(ActivateHdl, weld::TreeView&, bool);
    DECL_LINK(MouseMoveHdl, const MouseEvent&, bool);
    DECL_LINK(MousePressHdl, const MouseEvent&, bool);
    DECL_LINK(MouseReleaseHdl, const MouseEvent&, bool);

public:
    SvxPopupWindowListBox(SvxUndoRedoControl* pControl, weld::Widget* pParent,
                          const std::vector<OUString>& rUndoRedoList);

    virtual void GrabFocus() override
    {
        m_xListBox->grab_focus();
    }
};

SvxPopupWindowListBox::SvxPopupWindowListBox(SvxUndoRedoControl* pControl, weld::Widget* pParent,
                                             const std::vector<OUString>& rUndoRedoList)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, "svx/ui/floatingundoredo.ui", "FloatingUndoRedo")
    , m_xControl(pControl)
    , m_xListBox(m_xBuilder->weld_tree_view("treeview"))
    , m_xScratchIter(m_xListBox->make_iterator())
    , m_nVisRows(10)
{
    m_xListBox->set_selection_mode(SelectionMode::Multiple);

    for (const OUString& s : rUndoRedoList)
        m_xListBox->append_text(s);
    if (!rUndoRedoList.empty())
    {
        m_xListBox->set_cursor(0);
        m_xListBox->select(0);
        m_nSelectedRows = 1;
    }
    else
        m_nSelectedRows = 0;

    m_xListBox->set_size_request(m_xListBox->get_approximate_digit_width() * 25,
                                 m_xListBox->get_height_rows(m_nVisRows) + 2);

    m_xListBox->connect_row_activated(LINK(this, SvxPopupWindowListBox, ActivateHdl));
    m_xListBox->connect_mouse_move(LINK(this, SvxPopupWindowListBox, MouseMoveHdl));
    m_xListBox->connect_mouse_press(LINK(this, SvxPopupWindowListBox, MousePressHdl));
    m_xListBox->connect_mouse_release(LINK(this, SvxPopupWindowListBox, MouseReleaseHdl));
    m_xListBox->connect_key_press(LINK(this, SvxPopupWindowListBox, KeyInputHdl));
}

void SvxUndoRedoControl::SetInfo( sal_Int32 nCount )
{
    TranslateId pId;
    if (nCount == 1)
        pId = getCommandURL() == ".uno:Undo" ? RID_SVXSTR_NUM_UNDO_ACTION : RID_SVXSTR_NUM_REDO_ACTION;
    else
        pId = getCommandURL() == ".uno:Undo" ? RID_SVXSTR_NUM_UNDO_ACTIONS : RID_SVXSTR_NUM_REDO_ACTIONS;
    OUString aActionStr = SvxResId(pId);
    OUString aText = aActionStr.replaceAll("$(ARG1)", OUString::number(nCount));
    SetText(aText);
}

void SvxPopupWindowListBox::UpdateRow(int nRow)
{
    int nOldSelectedRows = m_nSelectedRows;
    while (m_nSelectedRows < nRow + 1)
    {
        m_xListBox->select(m_nSelectedRows++);
    }
    while (m_nSelectedRows - 1 > nRow)
    {
        m_xListBox->unselect(--m_nSelectedRows);
    }
    if (nOldSelectedRows != m_nSelectedRows)
        m_xControl->SetInfo(m_nSelectedRows);
}

IMPL_LINK(SvxPopupWindowListBox, MouseMoveHdl, const MouseEvent&, rMEvt, bool)
{
    if (m_xListBox->get_dest_row_at_pos(rMEvt.GetPosPixel(), m_xScratchIter.get(), false))
        UpdateRow(m_xListBox->get_iter_index_in_parent(*m_xScratchIter));
    return false;
}

IMPL_LINK(SvxPopupWindowListBox, MousePressHdl, const MouseEvent&, rMEvt, bool)
{
    if (m_xListBox->get_dest_row_at_pos(rMEvt.GetPosPixel(), m_xScratchIter.get(), false))
    {
        UpdateRow(m_xListBox->get_iter_index_in_parent(*m_xScratchIter));
        ActivateHdl(*m_xListBox);
    }
    return true;
}

IMPL_LINK(SvxPopupWindowListBox, MouseReleaseHdl, const MouseEvent&, rMEvt, bool)
{
    if (m_xListBox->get_dest_row_at_pos(rMEvt.GetPosPixel(), m_xScratchIter.get(), false))
        UpdateRow(m_xListBox->get_iter_index_in_parent(*m_xScratchIter));
    return true;
}

IMPL_LINK(SvxPopupWindowListBox, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    const vcl::KeyCode& rKCode = rKEvt.GetKeyCode();
    if (rKCode.GetModifier()) // only with no modifiers held
        return true;

    sal_uInt16 nCode = rKCode.GetCode();

    if (nCode == KEY_UP || nCode == KEY_PAGEUP ||
        nCode == KEY_DOWN || nCode == KEY_PAGEDOWN)
    {
        sal_Int32 nIndex = m_nSelectedRows - 1;
        sal_Int32 nOrigIndex = nIndex;
        sal_Int32 nCount = m_xListBox->n_children();

        if (nCode == KEY_UP)
            --nIndex;
        else if (nCode == KEY_DOWN)
            ++nIndex;
        else if (nCode == KEY_PAGEUP)
            nIndex -= m_nVisRows;
        else if (nCode == KEY_PAGEDOWN)
            nIndex += m_nVisRows;

        if (nIndex < 0)
            nIndex = 0;
        if (nIndex >= nCount)
            nIndex = nCount - 1;

        if (nIndex != nOrigIndex)
        {
            m_xListBox->scroll_to_row(nIndex);
            if (nIndex > nOrigIndex)
            {
                for (int i = nOrigIndex + 1; i <= nIndex; ++i)
                    UpdateRow(i);
            }
            else
            {
                for (int i = nOrigIndex - 1; i >= nIndex; --i)
                    UpdateRow(i);
            }
        }
        return true;
    }

    return false;
}

IMPL_LINK_NOARG(SvxPopupWindowListBox, ActivateHdl, weld::TreeView&, bool)
{
    m_xControl->Do(m_nSelectedRows);
    m_xControl->EndPopupMode();
    return true;
}

void SvxUndoRedoControl::Do(sal_Int16 nCount)
{
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
    if ( !xDispatchProvider.is() )
        return;

    css::util::URL aTargetURL;
    Reference < XURLTransformer > xTrans( URLTransformer::create(::comphelper::getProcessComponentContext()) );
    aTargetURL.Complete = m_aCommandURL;
    xTrans->parseStrict( aTargetURL );

    Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
    if ( xDispatch.is() )
    {
        INetURLObject aObj( m_aCommandURL );
        Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue(aObj.GetURLPath(), nCount) };
        xDispatch->dispatch(aTargetURL, aArgs);
    }
}

SvxUndoRedoControl::SvxUndoRedoControl(const css::uno::Reference<css::uno::XComponentContext>& rContext)
    : PopupWindowController(rContext, nullptr, OUString())
{
}

void SvxUndoRedoControl::initialize( const css::uno::Sequence< css::uno::Any >& rArguments )
{
    PopupWindowController::initialize(rArguments);

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (!getToolboxId(nId, &pToolBox) && !m_pToolbar)
        return;

    if (getModuleName() != "com.sun.star.script.BasicIDE")
    {
        if (m_pToolbar)
            aDefaultTooltip = m_pToolbar->get_item_tooltip_text(m_aCommandURL);
        else
        {
            pToolBox->SetItemBits(nId, ToolBoxItemBits::DROPDOWN | pToolBox->GetItemBits(nId));
            aDefaultTooltip = pToolBox->GetQuickHelpText(nId);
        }
    }
}

SvxUndoRedoControl::~SvxUndoRedoControl()
{
}

void SvxUndoRedoControl::SetText(const OUString& rText)
{
    mxInterimPopover->SetText(rText);
}

// XStatusListener
void SAL_CALL SvxUndoRedoControl::statusChanged(const css::frame::FeatureStateEvent& rEvent)
{
    if (rEvent.FeatureURL.Main == ".uno:GetUndoStrings" || rEvent.FeatureURL.Main == ".uno:GetRedoStrings")
    {
        css::uno::Sequence<OUString> aStrings;
        rEvent.State >>= aStrings;
        aUndoRedoList = comphelper::sequenceToContainer<std::vector<OUString>>(aStrings);
        return;
    }

    PopupWindowController::statusChanged(rEvent);

    ToolBox* pToolBox = nullptr;
    ToolBoxItemId nId;
    if (!getToolboxId(nId, &pToolBox) && !m_pToolbar)
        return;

    if (!rEvent.IsEnabled)
    {
        if (m_pToolbar)
            m_pToolbar->set_item_tooltip_text(m_aCommandURL, aDefaultTooltip);
        else
            pToolBox->SetQuickHelpText(nId, aDefaultTooltip);
        return;
    }

    OUString aQuickHelpText;
    if (rEvent.State >>= aQuickHelpText)
    {
        if (m_pToolbar)
            m_pToolbar->set_item_tooltip_text(m_aCommandURL, aQuickHelpText);
        else
            pToolBox->SetQuickHelpText(nId, aQuickHelpText);
    }
}

std::unique_ptr<WeldToolbarPopup> SvxUndoRedoControl::weldPopupWindow()
{
    if ( m_aCommandURL == ".uno:Undo" )
        updateStatus( ".uno:GetUndoStrings");
    else
        updateStatus( ".uno:GetRedoStrings");

    return std::make_unique<SvxPopupWindowListBox>(this, m_pToolbar, aUndoRedoList);
}

VclPtr<vcl::Window> SvxUndoRedoControl::createVclPopupWindow( vcl::Window* pParent )
{
    if ( m_aCommandURL == ".uno:Undo" )
        updateStatus( ".uno:GetUndoStrings");
    else
        updateStatus( ".uno:GetRedoStrings");

    auto xPopupWin = std::make_unique<SvxPopupWindowListBox>(this, pParent->GetFrameWeld(), aUndoRedoList);

    mxInterimPopover = VclPtr<InterimToolbarPopup>::Create(getFrameInterface(), pParent,
        std::move(xPopupWin));

    SetInfo(1); // count of selected rows

    mxInterimPopover->Show();

    return mxInterimPopover;
}

OUString SvxUndoRedoControl::getImplementationName()
{
    return "com.sun.star.comp.svx.UndoRedoToolBoxControl";
}

css::uno::Sequence<OUString> SvxUndoRedoControl::getSupportedServiceNames()
{
    return { "com.sun.star.frame.ToolbarController" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svx_UndoRedoToolBoxControl_get_implementation(
    css::uno::XComponentContext* rContext,
    css::uno::Sequence<css::uno::Any> const & )
{
    return cppu::acquire(new SvxUndoRedoControl(rContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
