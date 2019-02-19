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
#include <browserids.hxx>
#include <listviewitems.hxx>
#include <callbacks.hxx>

#include <com/sun/star/datatransfer/dnd/XDragGestureListener.hpp>
#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#include <com/sun/star/ui/XContextMenuInterceptor.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <vcl/help.hxx>
#include <dbaccess/IController.hxx>
#include <framework/actiontriggerhelper.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/event.hxx>

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

#define SPACEBETWEENENTRIES     4
// class DBTreeListBox
DBTreeListBox::DBTreeListBox( vcl::Window* pParent, WinBits nWinStyle )
    :SvTreeListBox(pParent,nWinStyle)
    ,m_pDragedEntry(nullptr)
    ,m_pActionListener(nullptr)
    ,m_pContextMenuProvider(nullptr)
    ,m_pResetEvent(nullptr)
{
    init();
}

void DBTreeListBox::init()
{
    SetSpaceBetweenEntries(SPACEBETWEENENTRIES);

    m_aTimer.SetTimeout(900);
    m_aTimer.SetInvokeHandler(LINK(this, DBTreeListBox, OnTimeOut));

    m_aScrollHelper.setUpScrollMethod( LINK(this, DBTreeListBox, ScrollUpHdl) );
    m_aScrollHelper.setDownScrollMethod( LINK(this, DBTreeListBox, ScrollDownHdl) );

    SetNodeDefaultImages( );

    EnableContextMenuHandling();

    SetQuickSearch( true );
}

DBTreeListBox::~DBTreeListBox()
{
    assert(!m_xMenuController.is());
    disposeOnce();
}

void DBTreeListBox::dispose()
{
    if (m_pResetEvent)
    {
        RemoveUserEvent(m_pResetEvent);
        m_pResetEvent = nullptr;
    }
    implStopSelectionTimer();
    SvTreeListBox::dispose();
}

SvTreeListEntry* DBTreeListBox::GetEntryPosByName( const OUString& aName, SvTreeListEntry* pStart, const IEntryFilter* _pFilter ) const
{
    SvTreeList* myModel = GetModel();
    std::pair<SvTreeListEntries::const_iterator,SvTreeListEntries::const_iterator> aIters =
        myModel->GetChildIterators(pStart);

    SvTreeListEntry* pEntry = nullptr;
    SvTreeListEntries::const_iterator it = aIters.first, itEnd = aIters.second;
    for (; it != itEnd; ++it)
    {
        pEntry = (*it).get();
        const SvLBoxString* pItem = static_cast<const SvLBoxString*>(
            pEntry->GetFirstItem(SvLBoxItemType::String));

        if (pItem && pItem->GetText() == aName)
        {
            if (!_pFilter || _pFilter->includeEntry(pEntry))
                // found
                break;
        }
        pEntry = nullptr;
    }

    return pEntry;
}

void DBTreeListBox::RequestingChildren( SvTreeListEntry* pParent )
{
    if (m_aPreExpandHandler.IsSet() && !m_aPreExpandHandler.Call(pParent))
    {
        // an error occurred. The method calling us will reset the entry flags, so it can't be expanded again.
        // But we want that the user may do a second try (i.e. because he mistypes a password in this try), so
        // we have to reset these flags controlling the expand ability
        m_pResetEvent = PostUserEvent(LINK(this, DBTreeListBox, OnResetEntryHdl), pParent, true);
    }
}

void DBTreeListBox::InitEntry(SvTreeListEntry* _pEntry, const OUString& aStr, const Image& _rCollEntryBmp, const Image& _rExpEntryBmp, SvLBoxButtonKind eButtonKind)
{
    SvTreeListBox::InitEntry( _pEntry, aStr, _rCollEntryBmp,_rExpEntryBmp, eButtonKind);
    SvLBoxItem* pTextItem(_pEntry->GetFirstItem(SvLBoxItemType::String));
    _pEntry->ReplaceItem(std::make_unique<OBoldListboxString>(aStr), _pEntry->GetPos(pTextItem));
}

void DBTreeListBox::implStopSelectionTimer()
{
    if ( m_aTimer.IsActive() )
        m_aTimer.Stop();
}

void DBTreeListBox::implStartSelectionTimer()
{
    implStopSelectionTimer();
    m_aTimer.Start();
}

void DBTreeListBox::DeselectHdl()
{
    m_aSelectedEntries.erase( GetHdlEntry() );
    SvTreeListBox::DeselectHdl();
    implStartSelectionTimer();
}

void DBTreeListBox::SelectHdl()
{
    m_aSelectedEntries.insert( GetHdlEntry() );
    SvTreeListBox::SelectHdl();
    implStartSelectionTimer();
}

void DBTreeListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    bool bHitEmptySpace = (nullptr == GetEntry(rMEvt.GetPosPixel(), true));
    if (bHitEmptySpace && (rMEvt.GetClicks() == 2) && rMEvt.IsMod1())
        Control::MouseButtonDown(rMEvt);
    else
        SvTreeListBox::MouseButtonDown(rMEvt);
}

void DBTreeListBox::EnableExpandHandler(SvTreeListEntry* pEntry)
{
    // set the flag which allows if the entry can be expanded
    pEntry->SetFlags( (pEntry->GetFlags() & ~SvTLEntryFlags(SvTLEntryFlags::NO_NODEBMP | SvTLEntryFlags::HAD_CHILDREN)) | SvTLEntryFlags::CHILDREN_ON_DEMAND );
    // redraw the entry
    GetModel()->InvalidateEntry(pEntry);
}

IMPL_LINK(DBTreeListBox, OnResetEntryHdl, void*, p, void)
{
    m_pResetEvent = nullptr;
    EnableExpandHandler(static_cast<SvTreeListEntry*>(p));
}

void DBTreeListBox::ModelHasEntryInvalidated( SvTreeListEntry* _pEntry )
{
    SvTreeListBox::ModelHasEntryInvalidated( _pEntry );

    if (m_aSelectedEntries.find(_pEntry) != m_aSelectedEntries.end())
    {
        SvLBoxItem* pTextItem = _pEntry->GetFirstItem(SvLBoxItemType::String);
        if ( pTextItem && !static_cast< OBoldListboxString* >( pTextItem )->isEmphasized() )
        {
            implStopSelectionTimer();
            m_aSelectedEntries.erase(_pEntry);
                // ehm - why?
        }
    }
}

void DBTreeListBox::ModelHasRemoved( SvTreeListEntry* _pEntry )
{
    SvTreeListBox::ModelHasRemoved(_pEntry);
    if (m_aSelectedEntries.find(_pEntry) != m_aSelectedEntries.end())
    {
        implStopSelectionTimer();
        m_aSelectedEntries.erase(_pEntry);
    }
}

sal_Int8 DBTreeListBox::AcceptDrop( const AcceptDropEvent& _rEvt )
{
    sal_Int8 nDropOption = DND_ACTION_NONE;
    if ( m_pActionListener )
    {
        SvTreeListEntry* pDroppedEntry = GetEntry(_rEvt.maPosPixel);
        // check if drag is on child entry, which is not allowed
        SvTreeListEntry* pParent = nullptr;
        if ( _rEvt.mnAction & DND_ACTION_MOVE )
        {
            if ( !m_pDragedEntry ) // no entry to move
            {
                nDropOption = m_pActionListener->queryDrop( _rEvt, GetDataFlavorExVector() );
                m_aMousePos = _rEvt.maPosPixel;
                m_aScrollHelper.scroll(m_aMousePos,GetOutputSizePixel());
                return nDropOption;
            }

            pParent = pDroppedEntry ? GetParent(pDroppedEntry) : nullptr;
            while ( pParent && pParent != m_pDragedEntry )
                pParent = GetParent(pParent);
        }

        if ( !pParent )
        {
            nDropOption = m_pActionListener->queryDrop( _rEvt, GetDataFlavorExVector() );
            // check if move is allowed
            if ( nDropOption & DND_ACTION_MOVE )
            {
                if ( m_pDragedEntry == pDroppedEntry || GetEntryPosByName(GetEntryText(m_pDragedEntry),pDroppedEntry) )
                    nDropOption = nDropOption & ~DND_ACTION_MOVE;//DND_ACTION_NONE;
            }
            m_aMousePos = _rEvt.maPosPixel;
            m_aScrollHelper.scroll(m_aMousePos,GetOutputSizePixel());
        }
    }

    return nDropOption;
}

sal_Int8 DBTreeListBox::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    if ( m_pActionListener )
        return m_pActionListener->executeDrop( _rEvt );

    return DND_ACTION_NONE;
}

void DBTreeListBox::StartDrag( sal_Int8 _nAction, const Point& _rPosPixel )
{
    if ( m_pActionListener )
    {
        m_pDragedEntry = GetEntry(_rPosPixel);
        if ( m_pDragedEntry && m_pActionListener->requestDrag( _nAction, _rPosPixel ) )
        {
            // if the (asynchronous) drag started, stop the selection timer
            implStopSelectionTimer();
            // and stop selecting entries by simply moving the mouse
            EndSelection();
        }
    }
}

void DBTreeListBox::RequestHelp( const HelpEvent& rHEvt )
{
    if ( !m_pActionListener )
    {
        SvTreeListBox::RequestHelp( rHEvt );
        return;
    }

    if( rHEvt.GetMode() & HelpEventMode::QUICK )
    {
        Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvTreeListEntry* pEntry = GetEntry( aPos );
        if( pEntry )
        {
            OUString sQuickHelpText;
            if ( m_pActionListener->requestQuickHelp( pEntry, sQuickHelpText ) )
            {
                Size aSize( GetOutputSizePixel().Width(), GetEntryHeight() );
                tools::Rectangle aScreenRect( OutputToScreenPixel( GetEntryPosition( pEntry ) ), aSize );

                Help::ShowQuickHelp( this, aScreenRect,
                                     sQuickHelpText, QuickHelpFlags::Left | QuickHelpFlags::VCenter );
                return;
            }
        }
    }

    SvTreeListBox::RequestHelp( rHEvt );
}

void DBTreeListBox::KeyInput( const KeyEvent& rKEvt )
{
    KeyFuncType eFunc = rKEvt.GetKeyCode().GetFunction();
    sal_uInt16      nCode = rKEvt.GetKeyCode().GetCode();
    bool bHandled = false;

    if(eFunc != KeyFuncType::DONTKNOW)
    {
        switch(eFunc)
        {
            case KeyFuncType::COPY:
                bHandled = ( m_aCopyHandler.IsSet() && !m_aSelectedEntries.empty() );
                if ( bHandled )
                    m_aCopyHandler.Call( nullptr );
                break;
            case KeyFuncType::PASTE:
                bHandled = ( m_aPasteHandler.IsSet() && !m_aSelectedEntries.empty() );
                if ( bHandled )
                    m_aPasteHandler.Call( nullptr );
                break;
            case KeyFuncType::DELETE:
                bHandled = ( m_aDeleteHandler.IsSet() && !m_aSelectedEntries.empty() );
                if ( bHandled )
                    m_aDeleteHandler.Call( nullptr );
                break;
            default:
                break;
        }
    }

    if ( KEY_RETURN == nCode )
    {
        bHandled = false;
        m_aEnterKeyHdl.Call(this);
        // this is a HACK. If the data source browser is opened in the "beamer", while the main frame
        //
        // contains a writer document, then pressing enter in the DSB would be rerouted to the writer
        //
        // document if we would not do this hack here.
        // The problem is that the Writer uses RETURN as _accelerator_ (which is quite weird itself),
        //
        // so the SFX framework is _obligated_ to pass it to the Writer if nobody else handled it. There
        //
        // is no chance to distinguish between
        //   "accelerators which are to be executed if the main document has the focus"
        // and
        //   "accelerators which are always to be executed"
        //
        // Thus we cannot prevent the handling of this key in the writer without declaring the key event
        // as "handled" herein.
        //
        // The bad thing about this approach is that it does not scale. Every other accelerator which
        // is used by the document will raise a similar bug once somebody discovers it.
        // If this is the case, we should discuss a real solution with the framework (SFX) and the
        // applications.
    }

    if ( !bHandled )
        SvTreeListBox::KeyInput(rKEvt);
}

bool DBTreeListBox::EditingEntry( SvTreeListEntry* /*pEntry*/, Selection& /*_aSelection*/)
{
    return false;
}

bool DBTreeListBox::EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText )
{
    DBTreeEditedEntry aEntry;
    aEntry.aNewText = rNewText;
    SetEntryText(pEntry,aEntry.aNewText);

    return false;  // we never want that the base change our text
}

bool DBTreeListBox::DoubleClickHdl()
{
    // continue default processing if the DoubleClickHandler didn't handle it
    return !aDoubleClickHdl.Call( this );
}

static void scrollWindow(DBTreeListBox* _pListBox, const Point& _rPos,bool _bUp)
{
    SvTreeListEntry* pEntry = _pListBox->GetEntry( _rPos );
    if( pEntry && pEntry != _pListBox->Last() )
    {
        _pListBox->ScrollOutputArea( _bUp ? -1 : 1 );
    }
}

IMPL_LINK_NOARG( DBTreeListBox, ScrollUpHdl, LinkParamNone*, void )
{
    scrollWindow(this,m_aMousePos,true);
}

IMPL_LINK_NOARG( DBTreeListBox, ScrollDownHdl, LinkParamNone*, void )
{
    scrollWindow(this,m_aMousePos,false);
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

VclPtr<PopupMenu> DBTreeListBox::CreateContextMenu()
{
    if ( !m_pContextMenuProvider )
        return nullptr;

    OUString aResourceName( m_pContextMenuProvider->getContextMenuResourceName( *this ) );
    if ( aResourceName.isEmpty() )
        return nullptr;

    css::uno::Sequence< css::uno::Any > aArgs( 3 );
    aArgs[0] <<= comphelper::makePropertyValue( "Value", aResourceName );
    aArgs[1] <<= comphelper::makePropertyValue( "Frame", m_pContextMenuProvider->getCommandController().getXController()->getFrame() );
    aArgs[2] <<= comphelper::makePropertyValue( "IsContextMenu", true );

    css::uno::Reference< css::uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
    m_xMenuController.set( xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
        "com.sun.star.comp.framework.ResourceMenuController", aArgs, xContext ), css::uno::UNO_QUERY );

    if ( !m_xMenuController.is() )
        return nullptr;

    rtl::Reference xPopupMenu( new VCLXPopupMenu );
    m_xMenuController->setPopupMenu( xPopupMenu.get() );
    VclPtr<PopupMenu> pContextMenu( static_cast< PopupMenu* >( xPopupMenu->GetMenu() ) );
    pContextMenu->AddEventListener( LINK( this, DBTreeListBox, MenuEventListener ) );

    // allow context menu interception
    ::comphelper::OInterfaceContainerHelper2* pInterceptors = m_pContextMenuProvider->getContextMenuInterceptors();
    if ( !pInterceptors || !pInterceptors->getLength() )
        return pContextMenu;

    OUString aMenuIdentifier( "private:resource/popupmenu/" + aResourceName );

    ContextMenuExecuteEvent aEvent;
    aEvent.SourceWindow = VCLUnoHelper::GetInterface( this );
    aEvent.ExecutePosition.X = -1;
    aEvent.ExecutePosition.Y = -1;
    aEvent.ActionTriggerContainer = ::framework::ActionTriggerHelper::CreateActionTriggerContainerFromMenu(
        pContextMenu.get(), &aMenuIdentifier );
    aEvent.Selection = new SelectionSupplier( m_pContextMenuProvider->getCurrentSelection( *this ) );

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
                    return nullptr;

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
        pContextMenu->Clear();
        ::framework::ActionTriggerHelper::CreateMenuFromActionTriggerContainer(
            pContextMenu, aEvent.ActionTriggerContainer );
        aEvent.ActionTriggerContainer.clear();
    }

    return pContextMenu;
}

void DBTreeListBox::ExecuteContextMenuAction( sal_uInt16 )
{
}

IMPL_LINK( DBTreeListBox, MenuEventListener, VclMenuEvent&, rMenuEvent, void )
{
    if ( rMenuEvent.GetId() == VclEventId::ObjectDying )
    {
        css::uno::Reference< css::lang::XComponent > xComponent( m_xMenuController, css::uno::UNO_QUERY );
        if ( xComponent.is() )
            xComponent->dispose();
        m_xMenuController.clear();
    }
}

IMPL_LINK_NOARG(DBTreeListBox, OnTimeOut, Timer*, void)
{
    implStopSelectionTimer();

    m_aSelChangeHdl.Call( nullptr );
}

void DBTreeListBox::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == StateChangedType::Visible )
        implStopSelectionTimer();
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
