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

#include <dbtreelistbox.hxx>
#include <dbexchange.hxx>
#include <callbacks.hxx>

#include <com/sun/star/awt/PopupMenuDirection.hpp>
#include <com/sun/star/ui/XContextMenuInterceptor.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <dbaccess/IController.hxx>
#include <framework/actiontriggerhelper.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svx/dbaobjectex.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>

#include <memory>

namespace dbaui
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::view;

InterimDBTreeListBox::InterimDBTreeListBox(vcl::Window* pParent)
    : InterimItemWindow(pParent, "dbaccess/ui/dbtreelist.ui", "DBTreeList")
    , TreeListBox(m_xBuilder->weld_tree_view("treeview"), true)
    , m_xStatusBar(m_xBuilder->weld_label("statusbar"))
{
    InitControlBase(&GetWidget());
}

InterimDBTreeListBox::~InterimDBTreeListBox()
{
    disposeOnce();
}

void InterimDBTreeListBox::dispose()
{
    implStopSelectionTimer();
    m_xStatusBar.reset();
    m_xTreeView.reset();
    InterimItemWindow::dispose();
}

bool InterimDBTreeListBox::DoChildKeyInput(const KeyEvent& rKEvt)
{
    return ChildKeyInput(rKEvt);
}

TreeListBoxDropTarget::TreeListBoxDropTarget(TreeListBox& rTreeView)
    : DropTargetHelper(rTreeView.GetWidget().get_drop_target())
    , m_rTreeView(rTreeView)
{
}

sal_Int8 TreeListBoxDropTarget::AcceptDrop(const AcceptDropEvent& rEvt)
{
    sal_Int8 nAccept = m_rTreeView.AcceptDrop(rEvt);

    if (nAccept != DND_ACTION_NONE)
    {
        // to enable the autoscroll when we're close to the edges
        weld::TreeView& rWidget = m_rTreeView.GetWidget();
        rWidget.get_dest_row_at_pos(rEvt.maPosPixel, nullptr, true);
    }

    return nAccept;
}

sal_Int8 TreeListBoxDropTarget::ExecuteDrop(const ExecuteDropEvent& rEvt)
{
    return m_rTreeView.ExecuteDrop(rEvt);
}

TreeListBox::TreeListBox(std::unique_ptr<weld::TreeView> xTreeView, bool bSQLType)
    : m_xTreeView(std::move(xTreeView))
    , m_aDropTargetHelper(*this)
    , m_pActionListener(nullptr)
    , m_pContextMenuProvider(nullptr)
    , m_aTimer("dbaccess TreeListBox m_aTimer")
{
    m_xTreeView->connect_key_press(LINK(this, TreeListBox, KeyInputHdl));
    m_xTreeView->connect_changed(LINK(this, TreeListBox, SelectHdl));
    m_xTreeView->connect_query_tooltip(LINK(this, TreeListBox, QueryTooltipHdl));
    m_xTreeView->connect_popup_menu(LINK(this, TreeListBox, CommandHdl));

    if (bSQLType)
        m_xHelper.set(new ODataClipboard);
    else
        m_xHelper.set(new svx::OComponentTransferable);
    m_xTreeView->enable_drag_source(m_xHelper, DND_ACTION_COPY);
    m_xTreeView->connect_drag_begin(LINK(this, TreeListBox, DragBeginHdl));

    m_aTimer.SetTimeout(900);
    m_aTimer.SetInvokeHandler(LINK(this, TreeListBox, OnTimeOut));
}

bool TreeListBox::DoChildKeyInput(const KeyEvent& /*rKEvt*/)
{
    // nothing by default
    return false;
}

IMPL_LINK(TreeListBox, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    KeyFuncType eFunc = rKEvt.GetKeyCode().GetFunction();
    bool bHandled = false;

    switch (eFunc)
    {
        case KeyFuncType::COPY:
            bHandled = m_aCopyHandler.IsSet() && !m_xTreeView->get_selected(nullptr);
            if (bHandled)
                m_aCopyHandler.Call(nullptr);
            break;
        case KeyFuncType::PASTE:
            bHandled = m_aPasteHandler.IsSet() && !m_xTreeView->get_selected(nullptr);
            if (bHandled)
                m_aPasteHandler.Call(nullptr);
            break;
        case KeyFuncType::DELETE:
            bHandled = m_aDeleteHandler.IsSet() && !m_xTreeView->get_selected(nullptr);
            if (bHandled)
                m_aDeleteHandler.Call(nullptr);
            break;
        default:
            break;
    }

    return bHandled || DoChildKeyInput(rKEvt);
}

void TreeListBox::implStopSelectionTimer()
{
    if ( m_aTimer.IsActive() )
        m_aTimer.Stop();
}

void TreeListBox::implStartSelectionTimer()
{
    implStopSelectionTimer();
    m_aTimer.Start();
}

IMPL_LINK_NOARG(TreeListBox, SelectHdl, weld::TreeView&, void)
{
    implStartSelectionTimer();
}

TreeListBox::~TreeListBox()
{
}

std::unique_ptr<weld::TreeIter> TreeListBox::GetEntryPosByName(std::u16string_view aName, const weld::TreeIter* pStart, const IEntryFilter* _pFilter) const
{
    auto xEntry(m_xTreeView->make_iterator(pStart));
    if (!pStart && !m_xTreeView->get_iter_first(*xEntry))
        return nullptr;

    do
    {
        if (m_xTreeView->get_text(*xEntry) == aName)
        {
            if (!_pFilter || _pFilter->includeEntry(reinterpret_cast<void*>(m_xTreeView->get_id(*xEntry).toUInt64())))
            {
                // found
                return xEntry;
            }
        }
    } while (m_xTreeView->iter_next(*xEntry));

    return nullptr;
}

IMPL_LINK(TreeListBox, DragBeginHdl, bool&, rUnsetDragIcon, bool)
{
    rUnsetDragIcon = false;

    if (m_pActionListener)
    {
        m_xDragedEntry = m_xTreeView->make_iterator();
        if (!m_xTreeView->get_selected(m_xDragedEntry.get()))
            m_xDragedEntry.reset();
        if (m_xDragedEntry && m_pActionListener->requestDrag(*m_xDragedEntry))
        {
            // if the (asynchronous) drag started, stop the selection timer
            implStopSelectionTimer();
            return false;
        }
    }

    return true;
}

sal_Int8 TreeListBox::AcceptDrop(const AcceptDropEvent& rEvt)
{
    sal_Int8 nDropOption = DND_ACTION_NONE;
    if ( m_pActionListener )
    {
        ::Point aDropPos = rEvt.maPosPixel;
        std::unique_ptr<weld::TreeIter> xDropTarget(m_xTreeView->make_iterator());
        if (!m_xTreeView->get_dest_row_at_pos(aDropPos, xDropTarget.get(), true))
            xDropTarget.reset();

        // check if drag is on child entry, which is not allowed
        std::unique_ptr<weld::TreeIter> xParent;
        if (rEvt.mnAction & DND_ACTION_MOVE)
        {
            if (!m_xDragedEntry) // no entry to move
                return m_pActionListener->queryDrop(rEvt, m_aDropTargetHelper.GetDataFlavorExVector());

            if (xDropTarget)
            {
                xParent = m_xTreeView->make_iterator(xDropTarget.get());
                if (!m_xTreeView->iter_parent(*xParent))
                    xParent.reset();
            }
            while (xParent && m_xTreeView->iter_compare(*xParent, *m_xDragedEntry) != 0)
            {
                if (!m_xTreeView->iter_parent(*xParent))
                    xParent.reset();
            }
        }

        if (!xParent)
        {
            nDropOption = m_pActionListener->queryDrop(rEvt, m_aDropTargetHelper.GetDataFlavorExVector());
            // check if move is allowed
            if ( nDropOption & DND_ACTION_MOVE )
            {
                if (!m_xDragedEntry || !xDropTarget ||
                    m_xTreeView->iter_compare(*m_xDragedEntry, *xDropTarget) == 0 ||
                    GetEntryPosByName(m_xTreeView->get_text(*m_xDragedEntry), xDropTarget.get()))
                {
                    nDropOption = nDropOption & ~DND_ACTION_MOVE;//DND_ACTION_NONE;
                }
            }
        }
    }

    return nDropOption;
}

sal_Int8 TreeListBox::ExecuteDrop(const ExecuteDropEvent& rEvt)
{
    if (m_pActionListener)
        m_pActionListener->executeDrop(rEvt);
    m_xTreeView->unset_drag_dest_row();
    return DND_ACTION_NONE;
}

IMPL_LINK(TreeListBox, QueryTooltipHdl, const weld::TreeIter&, rIter, OUString)
{
    OUString sQuickHelpText;
    if (m_pActionListener &&
        m_pActionListener->requestQuickHelp(reinterpret_cast<void*>(m_xTreeView->get_id(rIter).toUInt64()), sQuickHelpText))
    {
        return sQuickHelpText;
    }
    return m_xTreeView->get_tooltip_text();
}

namespace
{
    // SelectionSupplier
    typedef ::cppu::WeakImplHelper<   XSelectionSupplier
                                  >   SelectionSupplier_Base;
    class SelectionSupplier : public SelectionSupplier_Base
    {
    public:
        explicit SelectionSupplier( const Any& _rSelection )
            :m_aSelection( _rSelection )
        {
        }

        virtual sal_Bool SAL_CALL select( const Any& xSelection ) override;
        virtual Any SAL_CALL getSelection(  ) override;
        virtual void SAL_CALL addSelectionChangeListener( const Reference< XSelectionChangeListener >& xListener ) override;
        virtual void SAL_CALL removeSelectionChangeListener( const Reference< XSelectionChangeListener >& xListener ) override;

    protected:
        virtual ~SelectionSupplier() override
        {
        }

    private:
        Any m_aSelection;
    };

    sal_Bool SAL_CALL SelectionSupplier::select( const Any& /*_Selection*/ )
    {
        throw IllegalArgumentException();
        // API bug: this should be a NoSupportException
    }

    Any SAL_CALL SelectionSupplier::getSelection(  )
    {
        return m_aSelection;
    }

    void SAL_CALL SelectionSupplier::addSelectionChangeListener( const Reference< XSelectionChangeListener >& /*_Listener*/ )
    {
        OSL_FAIL( "SelectionSupplier::removeSelectionChangeListener: no support!" );
        // API bug: this should be a NoSupportException
    }

    void SAL_CALL SelectionSupplier::removeSelectionChangeListener( const Reference< XSelectionChangeListener >& /*_Listener*/ )
    {
        OSL_FAIL( "SelectionSupplier::removeSelectionChangeListener: no support!" );
        // API bug: this should be a NoSupportException
    }
}

IMPL_LINK(TreeListBox, CommandHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    ::Point aPos = rCEvt.GetMousePosPixel();

    std::unique_ptr<weld::TreeIter> xIter(m_xTreeView->make_iterator());
    if (m_xTreeView->get_dest_row_at_pos(aPos, xIter.get(), false) && !m_xTreeView->is_selected(*xIter))
    {
        m_xTreeView->unselect_all();
        m_xTreeView->set_cursor(*xIter);
        m_xTreeView->select(*xIter);
        SelectHdl(*m_xTreeView);
    }

    if (!m_pContextMenuProvider)
        return false;

    OUString aResourceName(m_pContextMenuProvider->getContextMenuResourceName());
    if (aResourceName.isEmpty())
        return false;

    css::uno::Sequence< css::uno::Any > aArgs{
        css::uno::Any(comphelper::makePropertyValue( "Value", aResourceName )),
        css::uno::Any(comphelper::makePropertyValue( "Frame", m_pContextMenuProvider->getCommandController().getXController()->getFrame() )),
        css::uno::Any(comphelper::makePropertyValue( "IsContextMenu", true ))
    };

    css::uno::Reference< css::uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    css::uno::Reference<css::frame::XPopupMenuController> xMenuController
        (xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            "com.sun.star.comp.framework.ResourceMenuController", aArgs, xContext), css::uno::UNO_QUERY);

    if (!xMenuController.is())
        return false;

    VclPtr<vcl::Window> xMenuParent = m_pContextMenuProvider->getMenuParent();

    css::uno::Reference< css::awt::XWindow> xSourceWindow = VCLUnoHelper::GetInterface(xMenuParent);

    rtl::Reference xPopupMenu( new VCLXPopupMenu );
    xMenuController->setPopupMenu( xPopupMenu );
    VclPtr<PopupMenu> pContextMenu( static_cast< PopupMenu* >( xPopupMenu->GetMenu() ) );

    // allow context menu interception
    ::comphelper::OInterfaceContainerHelper2* pInterceptors = m_pContextMenuProvider->getContextMenuInterceptors();
    if (pInterceptors && pInterceptors->getLength())
    {
        OUString aMenuIdentifier( "private:resource/popupmenu/" + aResourceName );

        ContextMenuExecuteEvent aEvent;
        aEvent.SourceWindow = xSourceWindow;
        aEvent.ExecutePosition.X = -1;
        aEvent.ExecutePosition.Y = -1;
        aEvent.ActionTriggerContainer = ::framework::ActionTriggerHelper::CreateActionTriggerContainerFromMenu(
            pContextMenu.get(), &aMenuIdentifier );
        aEvent.Selection = new SelectionSupplier(m_pContextMenuProvider->getCurrentSelection(*m_xTreeView));

        ::comphelper::OInterfaceIteratorHelper2 aIter( *pInterceptors );
        bool bModifiedMenu = false;
        bool bAskInterceptors = true;
        while ( aIter.hasMoreElements() && bAskInterceptors )
        {
            Reference< XContextMenuInterceptor > xInterceptor( aIter.next(), UNO_QUERY );
            if ( !xInterceptor.is() )
                continue;

            try
            {
                ContextMenuInterceptorAction eAction = xInterceptor->notifyContextMenuExecute( aEvent );
                switch ( eAction )
                {
                    case ContextMenuInterceptorAction_CANCELLED:
                        return false;

                    case ContextMenuInterceptorAction_EXECUTE_MODIFIED:
                        bModifiedMenu = true;
                        bAskInterceptors = false;
                        break;

                    case ContextMenuInterceptorAction_CONTINUE_MODIFIED:
                        bModifiedMenu = true;
                        bAskInterceptors = true;
                        break;

                    default:
                        OSL_FAIL( "DBTreeListBox::CreateContextMenu: unexpected return value of the interceptor call!" );
                        [[fallthrough]];
                    case ContextMenuInterceptorAction_IGNORED:
                        break;
                }
            }
            catch( const DisposedException& e )
            {
                if ( e.Context == xInterceptor )
                    aIter.remove();
            }
        }

        if ( bModifiedMenu )
        {
            xPopupMenu->clear();
            ::framework::ActionTriggerHelper::CreateMenuFromActionTriggerContainer(
                xPopupMenu, aEvent.ActionTriggerContainer );
            aEvent.ActionTriggerContainer.clear();
        }
    }

    // adjust pos relative to m_xTreeView to relative to xMenuParent
    m_pContextMenuProvider->adjustMenuPosition(*m_xTreeView, aPos);

    // do action for selected entry in popup menu
    css::uno::Reference<css::awt::XWindowPeer> xParent(xSourceWindow, css::uno::UNO_QUERY);
    xPopupMenu->execute(xParent, css::awt::Rectangle(aPos.X(), aPos.Y(), 1, 1), css::awt::PopupMenuDirection::EXECUTE_DOWN);
    pContextMenu.disposeAndClear();

    css::uno::Reference<css::lang::XComponent> xComponent(xMenuController, css::uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
    xMenuController.clear();

    return true;
}

IMPL_LINK_NOARG(TreeListBox, OnTimeOut, Timer*, void)
{
    implStopSelectionTimer();

    m_aSelChangeHdl.Call( nullptr );
}

std::unique_ptr<weld::TreeIter> TreeListBox::GetRootLevelParent(const weld::TreeIter* pEntry) const
{
    if (!pEntry)
        return nullptr;
    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator(pEntry));
    while (m_xTreeView->get_iter_depth(*xEntry))
        m_xTreeView->iter_parent(*xEntry);
    return xEntry;
}

DBTreeViewBase::DBTreeViewBase(weld::Container* pContainer)
    : m_xBuilder(Application::CreateBuilder(pContainer, "dbaccess/ui/dbtreelist.ui"))
    , m_xContainer(m_xBuilder->weld_container("DBTreeList"))
{
}

DBTreeViewBase::~DBTreeViewBase()
{
}

DBTreeView::DBTreeView(weld::Container* pContainer, bool bSQLType)
    : DBTreeViewBase(pContainer)
{
    m_xTreeListBox.reset(new TreeListBox(m_xBuilder->weld_tree_view("treeview"), bSQLType));
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
