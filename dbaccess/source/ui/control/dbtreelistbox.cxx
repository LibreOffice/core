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


#include "dbtreelistbox.hxx"
#include "dbu_resource.hrc"
#include "browserids.hxx"
#include "listviewitems.hxx"
#include "callbacks.hxx"

#include <com/sun/star/datatransfer/dnd/XDragGestureListener.hpp>
#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#include <com/sun/star/ui/XContextMenuInterceptor.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/URL.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <vcl/help.hxx>
#include "tabletree.hrc"
#include "IController.hxx"
#include <framework/actiontriggerhelper.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <framework/imageproducer.hxx>
#include <vcl/svapp.hxx>
#include <memory>

// .........................................................................
namespace dbaui
{
// .........................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::view;

DBG_NAME(DBTreeListBox)
#define SPACEBETWEENENTRIES     4
//========================================================================
// class DBTreeListBox
//========================================================================
//------------------------------------------------------------------------
DBTreeListBox::DBTreeListBox( Window* pParent, const Reference< XMultiServiceFactory >& _rxORB, WinBits nWinStyle ,sal_Bool _bHandleEnterKey)
    :SvTreeListBox(pParent,nWinStyle)
    ,m_pDragedEntry(NULL)
    ,m_pActionListener(NULL)
    ,m_pContextMenuProvider( NULL )
    ,m_bHandleEnterKey(_bHandleEnterKey)
    ,m_xORB(_rxORB)
{
    DBG_CTOR(DBTreeListBox,NULL);
    init();
}
// -----------------------------------------------------------------------------
DBTreeListBox::DBTreeListBox( Window* pParent, const Reference< XMultiServiceFactory >& _rxORB, const ResId& rResId,sal_Bool _bHandleEnterKey)
    :SvTreeListBox(pParent,rResId)
    ,m_pDragedEntry(NULL)
    ,m_pActionListener(NULL)
    ,m_pContextMenuProvider( NULL )
    ,m_bHandleEnterKey(_bHandleEnterKey)
    ,m_xORB(_rxORB)
{
    DBG_CTOR(DBTreeListBox,NULL);
    init();
}
// -----------------------------------------------------------------------------
void DBTreeListBox::init()
{
    sal_uInt16 nSize = SPACEBETWEENENTRIES;
    SetSpaceBetweenEntries(nSize);

    m_aTimer.SetTimeout(900);
    m_aTimer.SetTimeoutHdl(LINK(this, DBTreeListBox, OnTimeOut));

    m_aScrollHelper.setUpScrollMethod( LINK(this, DBTreeListBox, ScrollUpHdl) );
    m_aScrollHelper.setDownScrollMethod( LINK(this, DBTreeListBox, ScrollDownHdl) );

    SetNodeDefaultImages( );

    EnableContextMenuHandling();

    SetStyle( GetStyle() | WB_QUICK_SEARCH );
}
//------------------------------------------------------------------------
DBTreeListBox::~DBTreeListBox()
{
    DBG_DTOR(DBTreeListBox,NULL);
    implStopSelectionTimer();
}
//------------------------------------------------------------------------
SvTreeListEntry* DBTreeListBox::GetEntryPosByName( const String& aName, SvTreeListEntry* pStart, const IEntryFilter* _pFilter ) const
{
    SvLBoxTreeList* myModel = GetModel();
    SvTreeEntryList* pChildren = myModel->GetChildList(pStart);
    SvTreeListEntry* pEntry = NULL;
    if ( pChildren )
    {
        size_t nCount = pChildren->size();
        for (size_t i = 0; i < nCount; ++i)
        {
            pEntry = static_cast<SvTreeListEntry*>((*pChildren)[ i ]);
            SvLBoxString* pItem = (SvLBoxString*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
            if ( pItem->GetText().equals(aName) )
            {
                if ( !_pFilter || _pFilter->includeEntry( pEntry ) )
                    // found
                    break;
            }
            pEntry = NULL;
        }
    }

    return pEntry;
}

// -------------------------------------------------------------------------
void DBTreeListBox::EnableExpandHandler(SvTreeListEntry* _pEntry)
{
    LINK(this, DBTreeListBox, OnResetEntry).Call(_pEntry);
}

// -------------------------------------------------------------------------
void DBTreeListBox::RequestingChildren( SvTreeListEntry* pParent )
{
    if (m_aPreExpandHandler.IsSet())
    {
        if (!m_aPreExpandHandler.Call(pParent))
        {
            // an error occurred. The method calling us will reset the entry flags, so it can't be expanded again.
            // But we want that the user may do a second try (i.e. because he misstypes a password in this try), so
            // we have to reset these flags controlling the expand ability
            PostUserEvent(LINK(this, DBTreeListBox, OnResetEntry), pParent);
        }
    }
}

// -------------------------------------------------------------------------
void DBTreeListBox::InitEntry( SvTreeListEntry* _pEntry, const XubString& aStr, const Image& _rCollEntryBmp, const Image& _rExpEntryBmp, SvLBoxButtonKind eButtonKind)
{
    SvTreeListBox::InitEntry( _pEntry, aStr, _rCollEntryBmp,_rExpEntryBmp, eButtonKind);
    SvLBoxItem* pTextItem(_pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    SvLBoxString* pString = new OBoldListboxString( _pEntry, 0, aStr );
    _pEntry->ReplaceItem( pString,_pEntry->GetPos(pTextItem));
}

// -------------------------------------------------------------------------
void DBTreeListBox::implStopSelectionTimer()
{
    if ( m_aTimer.IsActive() )
        m_aTimer.Stop();
}

// -------------------------------------------------------------------------
void DBTreeListBox::implStartSelectionTimer()
{
    implStopSelectionTimer();
    m_aTimer.Start();
}

// -----------------------------------------------------------------------------

void DBTreeListBox::DeselectHdl()
{
    m_aSelectedEntries.erase( GetHdlEntry() );
    SvTreeListBox::DeselectHdl();
    implStartSelectionTimer();
}
// -------------------------------------------------------------------------
void DBTreeListBox::SelectHdl()
{
    m_aSelectedEntries.insert( GetHdlEntry() );
    SvTreeListBox::SelectHdl();
    implStartSelectionTimer();
}

// -------------------------------------------------------------------------
void DBTreeListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    sal_Bool bHitEmptySpace = (NULL == GetEntry(rMEvt.GetPosPixel(), sal_True));
    if (bHitEmptySpace && (rMEvt.GetClicks() == 2) && rMEvt.IsMod1())
        Control::MouseButtonDown(rMEvt);
    else
        SvTreeListBox::MouseButtonDown(rMEvt);
}

// -------------------------------------------------------------------------
IMPL_LINK(DBTreeListBox, OnResetEntry, SvTreeListEntry*, pEntry)
{
    // set the flag which allows if the entry can be expanded
    pEntry->SetFlags( (pEntry->GetFlags() & ~(SV_ENTRYFLAG_NO_NODEBMP | SV_ENTRYFLAG_HAD_CHILDREN)) | SV_ENTRYFLAG_CHILDREN_ON_DEMAND );
    // redraw the entry
    GetModel()->InvalidateEntry( pEntry );
    return 0L;
}
// -----------------------------------------------------------------------------
void DBTreeListBox::ModelHasEntryInvalidated( SvTreeListEntry* _pEntry )
{
    SvTreeListBox::ModelHasEntryInvalidated( _pEntry );

    SvTreeListEntry* pLBEntry = static_cast<SvTreeListEntry*>(_pEntry);
    if (m_aSelectedEntries.find(pLBEntry) != m_aSelectedEntries.end())
    {
        SvLBoxItem* pTextItem = pLBEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING);
        if ( pTextItem && !static_cast< OBoldListboxString* >( pTextItem )->isEmphasized() )
        {
            implStopSelectionTimer();
            m_aSelectedEntries.erase(pLBEntry);
                // ehm - why?
        }
    }
}
// -------------------------------------------------------------------------
void DBTreeListBox::ModelHasRemoved( SvTreeListEntry* _pEntry )
{
    SvTreeListBox::ModelHasRemoved(_pEntry);
    SvTreeListEntry* pLBEntry = static_cast<SvTreeListEntry*>(_pEntry);
    if (m_aSelectedEntries.find(pLBEntry) != m_aSelectedEntries.end())
    {
        implStopSelectionTimer();
        m_aSelectedEntries.erase(pLBEntry);
    }
}

// -------------------------------------------------------------------------
sal_Int8 DBTreeListBox::AcceptDrop( const AcceptDropEvent& _rEvt )
{
    sal_Int8 nDropOption = DND_ACTION_NONE;
    if ( m_pActionListener )
    {
        SvTreeListEntry* pDroppedEntry = GetEntry(_rEvt.maPosPixel);
        // check if drag is on child entry, which is not allowed
        SvTreeListEntry* pParent = NULL;
        if ( _rEvt.mnAction & DND_ACTION_MOVE )
        {
            if ( !m_pDragedEntry ) // no entry to move
            {
                nDropOption = m_pActionListener->queryDrop( _rEvt, GetDataFlavorExVector() );
                m_aMousePos = _rEvt.maPosPixel;
                m_aScrollHelper.scroll(m_aMousePos,GetOutputSizePixel());
                return nDropOption;
            }

            pParent = pDroppedEntry ? GetParent(pDroppedEntry) : NULL;
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

// -------------------------------------------------------------------------
sal_Int8 DBTreeListBox::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    if ( m_pActionListener )
        return m_pActionListener->executeDrop( _rEvt );

    return DND_ACTION_NONE;
}

// -------------------------------------------------------------------------
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

// -------------------------------------------------------------------------
void DBTreeListBox::RequestHelp( const HelpEvent& rHEvt )
{
    if ( !m_pActionListener )
    {
        SvTreeListBox::RequestHelp( rHEvt );
        return;
    }

    if( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvTreeListEntry* pEntry = GetEntry( aPos );
        if( pEntry )
        {
            String sQuickHelpText;
            if ( m_pActionListener->requestQuickHelp( pEntry, sQuickHelpText ) )
            {
                Size aSize( GetOutputSizePixel().Width(), GetEntryHeight() );
                Rectangle aScreenRect( OutputToScreenPixel( GetEntryPosition( pEntry ) ), aSize );

                Help::ShowQuickHelp( this, aScreenRect,
                                     sQuickHelpText, QUICKHELP_LEFT | QUICKHELP_VCENTER );
                return;
            }
        }
    }

    SvTreeListBox::RequestHelp( rHEvt );
}

// -----------------------------------------------------------------------------
void DBTreeListBox::KeyInput( const KeyEvent& rKEvt )
{
    KeyFuncType eFunc = rKEvt.GetKeyCode().GetFunction();
    sal_uInt16      nCode = rKEvt.GetKeyCode().GetCode();
    sal_Bool bHandled = sal_False;

    if(eFunc != KEYFUNC_DONTKNOW)
    {
        switch(eFunc)
        {
            case KEYFUNC_CUT:
                bHandled = ( m_aCutHandler.IsSet() && !m_aSelectedEntries.empty() );
                if ( bHandled )
                    m_aCutHandler.Call( NULL );
                break;
            case KEYFUNC_COPY:
                bHandled = ( m_aCopyHandler.IsSet() && !m_aSelectedEntries.empty() );
                if ( bHandled )
                    m_aCopyHandler.Call( NULL );
                break;
            case KEYFUNC_PASTE:
                bHandled = ( m_aPasteHandler.IsSet() && !m_aSelectedEntries.empty() );
                if ( bHandled )
                    m_aPasteHandler.Call( NULL );
                break;
            case KEYFUNC_DELETE:
                bHandled = ( m_aDeleteHandler.IsSet() && !m_aSelectedEntries.empty() );
                if ( bHandled )
                    m_aDeleteHandler.Call( NULL );
                break;
            default:
                break;
        }
    }

    if ( KEY_RETURN == nCode )
    {
        bHandled = m_bHandleEnterKey;
        if ( m_aEnterKeyHdl.IsSet() )
            m_aEnterKeyHdl.Call(this);
        // this is a HACK. If the data source browser is opened in the "beamer", while the main frame
        // contains a writer document, then pressing enter in the DSB would be rerouted to the writer
        // document if we would not do this hack here.
        // The problem is that the Writer uses RETURN as _accelerator_ (which is quite weird itself),
        // so the SFX framework is _obligated_ to pass it to the Writer if nobody else handled it. There
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
// -----------------------------------------------------------------------------
sal_Bool DBTreeListBox::EditingEntry( SvTreeListEntry* pEntry, Selection& /*_aSelection*/)
{
    return m_aEditingHandler.Call(pEntry) != 0;
}
// -----------------------------------------------------------------------------
sal_Bool DBTreeListBox::EditedEntry( SvTreeListEntry* pEntry, const rtl::OUString& rNewText )
{
    DBTreeEditedEntry aEntry;
    aEntry.pEntry = pEntry;
    aEntry.aNewText = rNewText;
    if(m_aEditedHandler.Call(&aEntry) != 0)
    {
        implStopSelectionTimer();
        m_aSelectedEntries.erase( pEntry );
    }
    SetEntryText(pEntry,aEntry.aNewText);

    return sal_False;  // we never want that the base change our text
}

// -----------------------------------------------------------------------------
sal_Bool DBTreeListBox::DoubleClickHdl()
{
    long nResult = aDoubleClickHdl.Call( this );
    // continue default processing if the DoubleClickHandler didn't handle it
    return nResult == 0;
}

// -----------------------------------------------------------------------------
void scrollWindow(DBTreeListBox* _pListBox, const Point& _rPos,sal_Bool _bUp)
{
    SvTreeListEntry* pEntry = _pListBox->GetEntry( _rPos );
    if( pEntry && pEntry != _pListBox->Last() )
    {
        _pListBox->ScrollOutputArea( _bUp ? -1 : 1 );
    }
}
// -----------------------------------------------------------------------------
IMPL_LINK( DBTreeListBox, ScrollUpHdl, SvTreeListBox*, /*pBox*/ )
{
    scrollWindow(this,m_aMousePos,sal_True);
    return 0;
}

//------------------------------------------------------------------------------
IMPL_LINK( DBTreeListBox, ScrollDownHdl, SvTreeListBox*, /*pBox*/ )
{
    scrollWindow(this,m_aMousePos,sal_False);
    return 0;
}
// -----------------------------------------------------------------------------
namespace
{
    void lcl_enableEntries( PopupMenu* _pPopup, IController& _rController )
    {
        if ( !_pPopup )
            return;

        sal_uInt16 nCount = _pPopup->GetItemCount();
        for (sal_uInt16 i=0; i < nCount; ++i)
        {
            if ( _pPopup->GetItemType(i) != MENUITEM_SEPARATOR )
            {
                sal_uInt16 nId = _pPopup->GetItemId(i);
                PopupMenu* pSubPopUp = _pPopup->GetPopupMenu(nId);
                if ( pSubPopUp )
                {
                    lcl_enableEntries( pSubPopUp, _rController );
                    _pPopup->EnableItem(nId,pSubPopUp->HasValidEntries());
                }
                else
                {
                    ::rtl::OUString sCommandURL( _pPopup->GetItemCommand( nId ) );
                    bool bEnabled =  sCommandURL.isEmpty()
                                  ? _rController.isCommandEnabled( nId )
                                  : _rController.isCommandEnabled( sCommandURL );
                    _pPopup->EnableItem( nId, bEnabled );
                }
            }
        }

        _pPopup->RemoveDisabledEntries();
    }
}

// -----------------------------------------------------------------------------
namespace
{
    void lcl_adjustMenuItemIDs( Menu& _rMenu, IController& _rCommandController )
    {
        sal_uInt16 nCount = _rMenu.GetItemCount();
        for ( sal_uInt16 pos = 0; pos < nCount; ++pos )
        {
            // do not adjust separators
            if ( _rMenu.GetItemType( pos ) == MENUITEM_SEPARATOR )
                continue;

            sal_uInt16 nId = _rMenu.GetItemId(pos);
            String aCommand = _rMenu.GetItemCommand( nId );
            PopupMenu* pPopup = _rMenu.GetPopupMenu( nId );
            if ( pPopup )
            {
                lcl_adjustMenuItemIDs( *pPopup, _rCommandController );
                continue;
            }

            const sal_uInt16 nCommandId = _rCommandController.registerCommandURL( aCommand );
            _rMenu.InsertItem( nCommandId, _rMenu.GetItemText( nId ), _rMenu.GetItemImage( nId ),
                _rMenu.GetItemBits( nId ), pos );

            // more things to preserve:
            // - the help command
            ::rtl::OUString sHelpURL = _rMenu.GetHelpCommand( nId );
            if ( !sHelpURL.isEmpty() )
                _rMenu.SetHelpCommand(  nCommandId, sHelpURL  );

            // remove the "old" item
            _rMenu.RemoveItem( pos+1 );
        }
    }
    void lcl_insertMenuItemImages( Menu& _rMenu, IController& _rCommandController )
    {
        uno::Reference< frame::XController > xController = _rCommandController.getXController();
        uno::Reference< frame::XFrame> xFrame;
        if ( xController.is() )
            xFrame = xController->getFrame();
        sal_uInt16 nCount = _rMenu.GetItemCount();
        for ( sal_uInt16 pos = 0; pos < nCount; ++pos )
        {
            // do not adjust separators
            if ( _rMenu.GetItemType( pos ) == MENUITEM_SEPARATOR )
                continue;

            sal_uInt16 nId = _rMenu.GetItemId(pos);
            String aCommand = _rMenu.GetItemCommand( nId );
            PopupMenu* pPopup = _rMenu.GetPopupMenu( nId );
            if ( pPopup )
            {
                lcl_insertMenuItemImages( *pPopup, _rCommandController );
                continue;
            }

            if ( xFrame.is() )
                _rMenu.SetItemImage(nId,framework::GetImageFromURL(xFrame,aCommand,sal_False));
        }
    }
    // =========================================================================
    // = SelectionSupplier
    // =========================================================================
    typedef ::cppu::WeakImplHelper1 <   XSelectionSupplier
                                    >   SelectionSupplier_Base;
    class SelectionSupplier : public SelectionSupplier_Base
    {
    public:
        SelectionSupplier( const Any& _rSelection )
            :m_aSelection( _rSelection )
        {
        }

        virtual ::sal_Bool SAL_CALL select( const Any& xSelection ) throw (IllegalArgumentException, RuntimeException);
        virtual Any SAL_CALL getSelection(  ) throw (RuntimeException);
        virtual void SAL_CALL addSelectionChangeListener( const Reference< XSelectionChangeListener >& xListener ) throw (RuntimeException);
        virtual void SAL_CALL removeSelectionChangeListener( const Reference< XSelectionChangeListener >& xListener ) throw (RuntimeException);

    protected:
        virtual ~SelectionSupplier()
        {
        }

    private:
        Any m_aSelection;
    };

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL SelectionSupplier::select( const Any& /*_Selection*/ ) throw (IllegalArgumentException, RuntimeException)
    {
        throw IllegalArgumentException();
        // API bug: this should be a NoSupportException
    }

    //--------------------------------------------------------------------
    Any SAL_CALL SelectionSupplier::getSelection(  ) throw (RuntimeException)
    {
        return m_aSelection;
    }

    //--------------------------------------------------------------------
    void SAL_CALL SelectionSupplier::addSelectionChangeListener( const Reference< XSelectionChangeListener >& /*_Listener*/ ) throw (RuntimeException)
    {
        OSL_FAIL( "SelectionSupplier::removeSelectionChangeListener: no support!" );
        // API bug: this should be a NoSupportException
    }

    //--------------------------------------------------------------------
    void SAL_CALL SelectionSupplier::removeSelectionChangeListener( const Reference< XSelectionChangeListener >& /*_Listener*/ ) throw (RuntimeException)
    {
        OSL_FAIL( "SelectionSupplier::removeSelectionChangeListener: no support!" );
        // API bug: this should be a NoSupportException
    }
}

// -----------------------------------------------------------------------------
PopupMenu* DBTreeListBox::CreateContextMenu( void )
{
    ::std::auto_ptr< PopupMenu > pContextMenu;

    if ( !m_pContextMenuProvider )
        return pContextMenu.release();

    // the basic context menu
    pContextMenu.reset( m_pContextMenuProvider->getContextMenu( *this ) );
    // disable what is not available currently
    lcl_enableEntries( pContextMenu.get(), m_pContextMenuProvider->getCommandController() );
    // set images
    lcl_insertMenuItemImages( *pContextMenu, m_pContextMenuProvider->getCommandController() );
    // allow context menu interception
    ::cppu::OInterfaceContainerHelper* pInterceptors = m_pContextMenuProvider->getContextMenuInterceptors();
    if ( !pInterceptors || !pInterceptors->getLength() )
        return pContextMenu.release();

    ContextMenuExecuteEvent aEvent;
    aEvent.SourceWindow = VCLUnoHelper::GetInterface( this );
    aEvent.ExecutePosition.X = -1;
    aEvent.ExecutePosition.Y = -1;
    aEvent.ActionTriggerContainer = ::framework::ActionTriggerHelper::CreateActionTriggerContainerFromMenu(
        m_xORB, pContextMenu.get(), 0 );
    aEvent.Selection = new SelectionSupplier( m_pContextMenuProvider->getCurrentSelection( *this ) );

    ::cppu::OInterfaceIteratorHelper aIter( *pInterceptors );
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
                    return NULL;

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
        // the interceptor(s) modified the menu description => create a new PopupMenu
        PopupMenu* pModifiedMenu = new PopupMenu;
        ::framework::ActionTriggerHelper::CreateMenuFromActionTriggerContainer(
            pModifiedMenu, aEvent.ActionTriggerContainer );
        aEvent.ActionTriggerContainer.clear();
        pContextMenu.reset( pModifiedMenu );

        // the interceptors only know command URLs, but our menus primarily work
        // with IDs -> we need to translate the commands to IDs
        lcl_adjustMenuItemIDs( *pModifiedMenu, m_pContextMenuProvider->getCommandController() );
    }

    return pContextMenu.release();
}

// -----------------------------------------------------------------------------
void DBTreeListBox::ExcecuteContextMenuAction( sal_uInt16 _nSelectedPopupEntry )
{
    if ( m_pContextMenuProvider && _nSelectedPopupEntry )
        m_pContextMenuProvider->getCommandController().executeChecked( _nSelectedPopupEntry, Sequence< PropertyValue >() );
}

// -----------------------------------------------------------------------------
IMPL_LINK(DBTreeListBox, OnTimeOut, void*, /*EMPTY_ARG*/)
{
    implStopSelectionTimer();

    m_aSelChangeHdl.Call( NULL );
    return 0L;
}
// -----------------------------------------------------------------------------
void DBTreeListBox::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_VISIBLE )
        implStopSelectionTimer();
}
// .........................................................................
}   // namespace dbaui
// .........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
