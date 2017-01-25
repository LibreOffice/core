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

#include "sbagrid.hrc"
#include "uiservices.hxx"

#include <sot/exchange.hxx>
#include <svx/svxids.hrc>

#include <svx/numinf.hxx>
#include <svx/dbaexchange.hxx>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>

#include "sbagrid.hxx"
#include "dlgattr.hxx"
#include "dlgsize.hxx"
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/form/ControlFontDialog.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/util/NumberFormat.hpp>

#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/form/DataSelectionType.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <tools/diagnose_ex.h>

#include <svl/intitem.hxx>
#include <svx/algitem.hxx>
#include <tools/multisel.hxx>
#include <svl/numuno.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/rngitem.hxx>

#include <vcl/waitobj.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <svl/zforlist.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbconversion.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <vcl/msgbox.hxx>
#include "dbu_brw.hrc"
#include "browserids.hxx"
#include "dbustrings.hrc"
#include "dbu_reghelper.hxx"
#include "dbexchange.hxx"
#include "TableRowExchange.hxx"
#include "TableRow.hxx"
#include "FieldDescriptions.hxx"
#include <svtools/stringtransfer.hxx>
#include <vcl/stdtext.hxx>
#include "UITools.hxx"
#include "TokenWriter.hxx"
#include <osl/diagnose.h>
#include <algorithm>

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::dbaui;
using namespace ::dbtools;
using namespace ::svx;
using namespace ::svt;

extern "C" void SAL_CALL createRegistryInfo_SbaXGridControl()
{
    static OMultiInstanceAutoRegistration< SbaXGridControl > aAutoRegistration;
}

css::uno::Sequence<OUString> SAL_CALL SbaXGridControl::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

Reference< XInterface > SAL_CALL SbaXGridControl::Create(const Reference<XMultiServiceFactory >& _rxFactory)
{
    return *(new SbaXGridControl( comphelper::getComponentContext(_rxFactory) ));
}

// SbaXGridControl

OUString SAL_CALL SbaXGridControl::getImplementationName()
{
    return getImplementationName_Static();
}

OUString SbaXGridControl::getImplementationName_Static()
{
    return OUString("com.sun.star.comp.dbu.SbaXGridControl");
}

Sequence< OUString> SbaXGridControl::getSupportedServiceNames_Static()
{
    Sequence< OUString> aSupported(3);
    aSupported[0] = "com.sun.star.form.control.InteractionGridControl";
    aSupported[1] = "com.sun.star.form.control.GridControl";
    aSupported[2] = "com.sun.star.awt.UnoControl";
    return aSupported;
}

SbaXGridControl::SbaXGridControl(const Reference< XComponentContext >& _rM)
    : FmXGridControl(_rM)
{
}

SbaXGridControl::~SbaXGridControl()
{
}

FmXGridPeer* SbaXGridControl::imp_CreatePeer(vcl::Window* pParent)
{
    FmXGridPeer* pReturn = new SbaXGridPeer(m_xContext);

    // translate properties into WinBits
    WinBits nStyle = WB_TABSTOP;
    Reference< XPropertySet >  xModelSet(getModel(), UNO_QUERY);
    if (xModelSet.is())
    {
        try
        {
            if (::comphelper::getINT16(xModelSet->getPropertyValue(PROPERTY_BORDER)))
                nStyle |= WB_BORDER;
        }
        catch(Exception&)
        {
        }

    }

    pReturn->Create(pParent, nStyle);
    return pReturn;
}

Any SAL_CALL SbaXGridControl::queryInterface(const Type& _rType)
{
    Any aRet = FmXGridControl::queryInterface(_rType);
    return aRet.hasValue() ? aRet : ::cppu::queryInterface(_rType,static_cast<css::frame::XDispatch*>(this));
}

Sequence< Type > SAL_CALL SbaXGridControl::getTypes(  )
{
    Sequence< Type > aTypes = FmXGridControl::getTypes();

    sal_Int32 nTypes = aTypes.getLength();
    aTypes.realloc(nTypes + 1);
    aTypes[nTypes] = cppu::UnoType<css::frame::XDispatch>::get();

    return aTypes;
}

Sequence< sal_Int8 > SAL_CALL SbaXGridControl::getImplementationId(  )
{
    return css::uno::Sequence<sal_Int8>();
}

void SAL_CALL SbaXGridControl::createPeer(const Reference< css::awt::XToolkit > & rToolkit, const Reference< css::awt::XWindowPeer > & rParentPeer)
{
    FmXGridControl::createPeer(rToolkit, rParentPeer);

    OSL_ENSURE(!mbCreatingPeer, "FmXGridControl::createPeer : recursion!");
        // see the base class' createPeer for a comment on this

    // TODO: why the hell this whole class does not use any mutex?

        Reference< css::frame::XDispatch >  xDisp(getPeer(), UNO_QUERY);
        for (   StatusMultiplexerArray::const_iterator aIter = m_aStatusMultiplexer.begin();
                aIter != m_aStatusMultiplexer.end();
                ++aIter)
        {
            if ((*aIter).second.is() && (*aIter).second->getLength())
                xDisp->addStatusListener((*aIter).second.get(), (*aIter).first);
        }
}

void SAL_CALL SbaXGridControl::dispatch(const css::util::URL& aURL, const Sequence< PropertyValue >& aArgs)
{
    Reference< css::frame::XDispatch >  xDisp(getPeer(), UNO_QUERY);
    if (xDisp.is())
        xDisp->dispatch(aURL, aArgs);
}

void SAL_CALL SbaXGridControl::addStatusListener( const Reference< XStatusListener > & _rxListener, const URL& _rURL )
{
    ::osl::MutexGuard aGuard( GetMutex() );
    if ( _rxListener.is() )
    {
        rtl::Reference<SbaXStatusMultiplexer>& xMultiplexer = m_aStatusMultiplexer[ _rURL ];
        if ( !xMultiplexer.is() )
        {
            xMultiplexer = new SbaXStatusMultiplexer( *this, GetMutex() );
        }

        xMultiplexer->addInterface( _rxListener );
        if ( getPeer().is() )
        {
            if ( 1 == xMultiplexer->getLength() )
            {   // the first external listener for this URL
                Reference< XDispatch >  xDisp( getPeer(), UNO_QUERY );
                xDisp->addStatusListener( xMultiplexer.get(), _rURL );
            }
            else
            {   // already have other listeners for this URL
                _rxListener->statusChanged( xMultiplexer->getLastEvent() );
            }
        }
    }
}

void SAL_CALL SbaXGridControl::removeStatusListener(const Reference< css::frame::XStatusListener > & _rxListener, const css::util::URL& _rURL)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    rtl::Reference<SbaXStatusMultiplexer>& xMultiplexer = m_aStatusMultiplexer[_rURL];
    if (!xMultiplexer.is())
    {
        xMultiplexer = new SbaXStatusMultiplexer(*this,GetMutex());
    }

    if (getPeer().is() && xMultiplexer->getLength() == 1)
    {
        Reference< css::frame::XDispatch >  xDisp(getPeer(), UNO_QUERY);
        xDisp->removeStatusListener(xMultiplexer.get(), _rURL);
    }
    xMultiplexer->removeInterface( _rxListener );
}

void SAL_CALL SbaXGridControl::dispose()
{
    SolarMutexGuard aGuard;

    EventObject aEvt;
    aEvt.Source = *this;

    for (   StatusMultiplexerArray::iterator aIter = m_aStatusMultiplexer.begin();
            aIter != m_aStatusMultiplexer.end();
            ++aIter)
    {
        if ((*aIter).second.is())
        {
            (*aIter).second->disposeAndClear(aEvt);
            (*aIter).second.clear();
        }
    }
    StatusMultiplexerArray().swap(m_aStatusMultiplexer);

    FmXGridControl::dispose();
}

// SbaXGridPeer
SbaXGridPeer::SbaXGridPeer(const Reference< XComponentContext >& _rM)
: FmXGridPeer(_rM)
,m_aStatusListeners(m_aMutex)
{
}

SbaXGridPeer::~SbaXGridPeer()
{
}

void SAL_CALL SbaXGridPeer::dispose()
{
    EventObject aEvt(*this);

    m_aStatusListeners.disposeAndClear(aEvt);

    FmXGridPeer::dispose();
}

void SbaXGridPeer::NotifyStatusChanged(const css::util::URL& _rUrl, const Reference< css::frame::XStatusListener > & xControl)
{
    VclPtr< SbaGridControl > pGrid = GetAs< SbaGridControl >();
    if (!pGrid)
        return;

    css::frame::FeatureStateEvent aEvt;
    aEvt.Source = *this;
    aEvt.IsEnabled = !pGrid->IsReadOnlyDB();
    aEvt.FeatureURL = _rUrl;

    MapDispatchToBool::const_iterator aURLStatePos = m_aDispatchStates.find( classifyDispatchURL( _rUrl ) );
    if ( m_aDispatchStates.end() != aURLStatePos )
        aEvt.State <<= aURLStatePos->second;
    else
        aEvt.State <<= false;

    if (xControl.is())
        xControl->statusChanged(aEvt);
    else
    {
        ::cppu::OInterfaceContainerHelper * pIter = m_aStatusListeners.getContainer(_rUrl);

        if (pIter)
        {
            ::cppu::OInterfaceIteratorHelper aListIter(*pIter);
            while (aListIter.hasMoreElements())
                static_cast< css::frame::XStatusListener*>(aListIter.next())->statusChanged(aEvt);
        }
    }
}

Any SAL_CALL SbaXGridPeer::queryInterface(const Type& _rType)
{
    Any aRet = ::cppu::queryInterface(_rType,static_cast<css::frame::XDispatch*>(this));
    if(aRet.hasValue())
        return aRet;
    return FmXGridPeer::queryInterface(_rType);
}

Reference< css::frame::XDispatch >  SAL_CALL SbaXGridPeer::queryDispatch(const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags)
{
    if  (   ( aURL.Complete == ".uno:GridSlots/BrowserAttribs" ) || ( aURL.Complete == ".uno:GridSlots/RowHeight" )
        ||  ( aURL.Complete == ".uno:GridSlots/ColumnAttribs" )  || ( aURL.Complete == ".uno:GridSlots/ColumnWidth" )
        )
    {
        return static_cast<css::frame::XDispatch*>(this);
    }

    return FmXGridPeer::queryDispatch(aURL, aTargetFrameName, nSearchFlags);
}

IMPL_LINK_NOARG( SbaXGridPeer, OnDispatchEvent, void*, void )
{
    VclPtr< SbaGridControl > pGrid = GetAs< SbaGridControl >();
    if ( pGrid )    // if this fails, we were disposing before arriving here
    {
        if ( Application::GetMainThreadIdentifier() != ::osl::Thread::getCurrentIdentifier() )
        {
            // still not in the main thread (see SbaXGridPeer::dispatch). post an event, again
            // without moving the special even to the back of the queue
            pGrid->PostUserEvent( LINK( this, SbaXGridPeer, OnDispatchEvent ) );
        }
        else
        {
            DispatchArgs aArgs = m_aDispatchArgs.front();
            m_aDispatchArgs.pop();

            SbaXGridPeer::dispatch( aArgs.aURL, aArgs.aArgs );
        }
    }
}

SbaXGridPeer::DispatchType SbaXGridPeer::classifyDispatchURL( const URL& _rURL )
{
    DispatchType eURLType = dtUnknown;
    if ( _rURL.Complete == ".uno:GridSlots/BrowserAttribs" )
        eURLType = dtBrowserAttribs;
    else if ( _rURL.Complete == ".uno:GridSlots/RowHeight" )
        eURLType = dtRowHeight;
    else if ( _rURL.Complete == ".uno:GridSlots/ColumnAttribs" )
        eURLType = dtColumnAttribs;
    else if ( _rURL.Complete == ".uno:GridSlots/ColumnWidth" )
        eURLType = dtColumnWidth;
    return eURLType;
}

void SAL_CALL SbaXGridPeer::dispatch(const URL& aURL, const Sequence< PropertyValue >& aArgs)
{
    VclPtr< SbaGridControl > pGrid = GetAs< SbaGridControl >();
    if (!pGrid)
        return;

    if ( Application::GetMainThreadIdentifier() != ::osl::Thread::getCurrentIdentifier() )
    {
        // we're not in the main thread. This is bad, as we want to raise windows here,
        // and VCL does not like windows to be opened in non-main threads (at least on Win32).
        // Okay, do this async. No problem with this, as XDispatch::dispatch is defined to be
        // a one-way method.

        // save the args
        DispatchArgs aDispatchArgs;
        aDispatchArgs.aURL = aURL;
        aDispatchArgs.aArgs = aArgs;
        m_aDispatchArgs.push( aDispatchArgs );

        // post an event
        // we use the Window::PostUserEvent here, instead of the application::PostUserEvent
        // this saves us from keeping track of these events - as soon as the window dies,
        // the events are deleted automatically. For the application way, we would need to
        // do this ourself.
        // As we use our grid as window, and the grid dies before we dy, this should be no problem.
        pGrid->PostUserEvent( LINK( this, SbaXGridPeer, OnDispatchEvent ) );
        return;
    }

    SolarMutexGuard aGuard;
    sal_Int16 nColId = -1;
    const PropertyValue* pArgs = aArgs.getConstArray();
    for (sal_Int32 i=0; i<aArgs.getLength(); ++i, ++pArgs)
    {
        if (pArgs->Name == "ColumnViewPos")
        {
            nColId = pGrid->GetColumnIdFromViewPos(::comphelper::getINT16(pArgs->Value));
            break;
        }
        if (pArgs->Name == "ColumnModelPos")
        {
            nColId = pGrid->GetColumnIdFromModelPos(::comphelper::getINT16(pArgs->Value));
            break;
        }
        if (pArgs->Name == "ColumnId")
        {
            nColId = ::comphelper::getINT16(pArgs->Value);
            break;
        }
    }

    DispatchType eURLType = classifyDispatchURL( aURL );

    if ( dtUnknown != eURLType )
    {
        // notify any status listeners that the dialog is now active (well, about to be active)
        MapDispatchToBool::const_iterator aThisURLState = m_aDispatchStates.insert( MapDispatchToBool::value_type( eURLType, true ) ).first;
        NotifyStatusChanged( aURL, nullptr );

        // execute the dialog
        switch ( eURLType )
        {
            case dtBrowserAttribs:
                pGrid->SetBrowserAttrs();
                break;

            case dtRowHeight:
                pGrid->SetRowHeight();
                break;

            case dtColumnAttribs:
            {
                OSL_ENSURE(nColId != -1, "SbaXGridPeer::dispatch : invalid parameter !");
                if (nColId != -1)
                    break;
                pGrid->SetColAttrs(nColId);
            }
            break;

            case dtColumnWidth:
            {
                OSL_ENSURE(nColId != -1, "SbaXGridPeer::dispatch : invalid parameter !");
                if (nColId != -1)
                    break;
                pGrid->SetColWidth(nColId);
            }
            break;

            case dtUnknown:
                break;
        }

        // notify any status listeners that the dialog vanished
        m_aDispatchStates.erase( aThisURLState );
        NotifyStatusChanged( aURL, nullptr );
    }
}

void SAL_CALL SbaXGridPeer::addStatusListener(const Reference< css::frame::XStatusListener > & xControl, const css::util::URL& aURL)
{
    ::cppu::OInterfaceContainerHelper* pCont = m_aStatusListeners.getContainer(aURL);
    if (!pCont)
        m_aStatusListeners.addInterface(aURL,xControl);
    else
        pCont->addInterface(xControl);
    NotifyStatusChanged(aURL, xControl);
}

void SAL_CALL SbaXGridPeer::removeStatusListener(const Reference< css::frame::XStatusListener > & xControl, const css::util::URL& aURL)
{
    ::cppu::OInterfaceContainerHelper* pCont = m_aStatusListeners.getContainer(aURL);
    if ( pCont )
        pCont->removeInterface(xControl);
}

namespace
{
    class theSbaXGridPeerUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSbaXGridPeerUnoTunnelId > {};
}

const Sequence< sal_Int8 > & SbaXGridPeer::getUnoTunnelId()
{
    return theSbaXGridPeerUnoTunnelId::get().getSeq();
}

Sequence< Type > SAL_CALL SbaXGridPeer::getTypes()
{
    Sequence< Type > aTypes = FmXGridPeer::getTypes();
    sal_Int32 nOldLen = aTypes.getLength();
    aTypes.realloc(nOldLen + 1);
    aTypes.getArray()[nOldLen] = cppu::UnoType<css::frame::XDispatch>::get();

    return aTypes;
}

// return implementation specific data
sal_Int64 SAL_CALL SbaXGridPeer::getSomething( const Sequence< sal_Int8 > & rId )
{
    if( rId.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(),  rId.getConstArray(), 16 ) )
        return reinterpret_cast< sal_Int64 >( this );

    return FmXGridPeer::getSomething(rId);
}

SbaXGridPeer* SbaXGridPeer::getImplementation(const Reference< XInterface >& _rxIFace)
{
    Reference< XUnoTunnel > xTunnel(
        _rxIFace, UNO_QUERY);
    if (xTunnel.is())
        return reinterpret_cast<SbaXGridPeer*>(xTunnel->getSomething(getUnoTunnelId()));
    return nullptr;
}

VclPtr<FmGridControl> SbaXGridPeer::imp_CreateControl(vcl::Window* pParent, WinBits nStyle)
{
    return VclPtr<SbaGridControl>::Create( m_xContext, pParent, this, nStyle);
}

// SbaGridHeader

SbaGridHeader::SbaGridHeader(BrowseBox* pParent)
    :FmGridHeader(pParent, WB_STDHEADERBAR | WB_DRAG)
    ,DragSourceHelper(this)
{
}

SbaGridHeader::~SbaGridHeader()
{
    disposeOnce();
}

void SbaGridHeader::dispose()
{
    DragSourceHelper::dispose();
    FmGridHeader::dispose();
}

void SbaGridHeader::StartDrag( sal_Int8 _nAction, const Point& _rPosPixel )
{
    SolarMutexGuard aGuard;
        // in the new DnD API, the solar mutex is not locked when StartDrag get's called

    ImplStartColumnDrag( _nAction, _rPosPixel );
}

void SbaGridHeader::MouseButtonDown( const MouseEvent& _rMEvt )
{
    if (_rMEvt.IsLeft())
        if (_rMEvt.GetClicks() != 2)
        {
            // the base class will start a column move here, which we don't want to allow
            // (at the moment. If we store relative positions with the columns, we can allow column moves ....)

        }

    FmGridHeader::MouseButtonDown(_rMEvt);
}

void SbaGridHeader::ImplStartColumnDrag(sal_Int8 _nAction, const Point& _rMousePos)
{
    sal_uInt16 nId = GetItemId(_rMousePos);
    bool bResizingCol = false;
    if (HEADERBAR_ITEM_NOTFOUND != nId)
    {
        Rectangle aColRect = GetItemRect(nId);
        aColRect.Left() += nId ? 3 : 0; // the handle col (nId == 0) does not have a left margin for resizing
        aColRect.Right() -= 3;
        bResizingCol = !aColRect.IsInside(_rMousePos);
    }
    if (!bResizingCol)
    {
        // force the base class to end its drag mode
        EndTracking(TrackingEventFlags::Cancel | TrackingEventFlags::End);

        // because we have 3d-buttons the select handler is called from MouseButtonUp, but StartDrag
        // occurs earlier (while the mouse button is down)
        // so for optical reasons we select the column before really starting the drag operation.
        notifyColumnSelect(nId);

        static_cast<SbaGridControl*>(GetParent())->StartDrag(_nAction,
                Point(
                    _rMousePos.X() + GetPosPixel().X(),     // we aren't left-justified with our parent, in contrast to the data window
                    _rMousePos.Y() - GetSizePixel().Height()
                )
            );
    }
}

void SbaGridHeader::PreExecuteColumnContextMenu(sal_uInt16 nColId, PopupMenu& rMenu)
{
    FmGridHeader::PreExecuteColumnContextMenu(nColId, rMenu);

    // some items are valid only if the db isn't readonly
    bool bDBIsReadOnly = static_cast<SbaGridControl*>(GetParent())->IsReadOnlyDB();

    if (bDBIsReadOnly)
    {
        rMenu.EnableItem(SID_FM_HIDECOL, false);
        PopupMenu* pShowColsMenu = rMenu.GetPopupMenu(SID_FM_SHOWCOLS);
        if (pShowColsMenu)
        {
            // at most 16 items which mean "show column <name>"
            for (sal_uInt16 i=1; i<16; ++i)
                pShowColsMenu->EnableItem(i, false);
            // "show cols/more..." and "show cols/all"
            pShowColsMenu->EnableItem(SID_FM_SHOWCOLS_MORE, false);
            pShowColsMenu->EnableItem(SID_FM_SHOWALLCOLS, false);
        }
    }

    // prepend some new items
    bool bColAttrs = (nColId != (sal_uInt16)-1) && (nColId != 0);
    if ( bColAttrs && !bDBIsReadOnly)
    {
        ScopedVclPtrInstance<PopupMenu> aNewItems(ModuleRes(RID_SBA_GRID_COLCTXMENU));
        sal_uInt16 nPos = 0;
        sal_uInt16 nModelPos = static_cast<SbaGridControl*>(GetParent())->GetModelColumnPos(nColId);
        Reference< XPropertySet >  xField = static_cast<SbaGridControl*>(GetParent())->getField(nModelPos);

        if ( xField.is() )
        {
            switch( ::comphelper::getINT32(xField->getPropertyValue(PROPERTY_TYPE)) )
            {
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            case DataType::SQLNULL:
            case DataType::OBJECT:
            case DataType::BLOB:
            case DataType::CLOB:
            case DataType::REF:
                break;
            default:
                rMenu.InsertItem(ID_BROWSER_COLATTRSET, aNewItems->GetItemText(ID_BROWSER_COLATTRSET), MenuItemBits::NONE, OString(), nPos++);
                rMenu.SetHelpId(ID_BROWSER_COLATTRSET, aNewItems->GetHelpId(ID_BROWSER_COLATTRSET));
                rMenu.InsertSeparator(OString(), nPos++);
            }
        }

        rMenu.InsertItem(ID_BROWSER_COLWIDTH, aNewItems->GetItemText(ID_BROWSER_COLWIDTH), MenuItemBits::NONE, OString(), nPos++);
        rMenu.SetHelpId(ID_BROWSER_COLWIDTH, aNewItems->GetHelpId(ID_BROWSER_COLWIDTH));
        rMenu.InsertSeparator(OString(), nPos++);
    }
}

void SbaGridHeader::PostExecuteColumnContextMenu(sal_uInt16 nColId, const PopupMenu& rMenu, sal_uInt16 nExecutionResult)
{
    switch (nExecutionResult)
    {
        case ID_BROWSER_COLWIDTH:
            static_cast<SbaGridControl*>(GetParent())->SetColWidth(nColId);
            break;

        case ID_BROWSER_COLATTRSET:
            static_cast<SbaGridControl*>(GetParent())->SetColAttrs(nColId);
            break;
        case ID_BROWSER_COLUMNINFO:
            {
                sal_uInt16 nModelPos = static_cast<SbaGridControl*>(GetParent())->GetModelColumnPos(nColId);
                Reference< XPropertySet >  xField = static_cast<SbaGridControl*>(GetParent())->getField(nModelPos);

                if(!xField.is())
                    break;
                ::std::vector< std::shared_ptr<OTableRow> > vClipboardList;
                // send it to the clipboard
                vClipboardList.push_back(std::make_shared<OTableRow>(xField));
                OTableRowExchange* pData = new OTableRowExchange(vClipboardList);
                Reference< css::datatransfer::XTransferable> xRef = pData;
                pData->CopyToClipboard(GetParent());
            }
            break;

        default: FmGridHeader::PostExecuteColumnContextMenu(nColId, rMenu, nExecutionResult);
    }
}

// SbaGridControl
SbaGridControl::SbaGridControl(Reference< XComponentContext > const & _rM,
                               vcl::Window* pParent, FmXGridPeer* _pPeer, WinBits nBits)
    :FmGridControl(_rM,pParent, _pPeer, nBits)
    ,m_pMasterListener(nullptr)
    ,m_nAsyncDropEvent(nullptr)
    ,m_bActivatingForDrop(false)
{
}

SbaGridControl::~SbaGridControl()
{
    disposeOnce();
}

void SbaGridControl::dispose()
{
    if (m_nAsyncDropEvent)
        Application::RemoveUserEvent(m_nAsyncDropEvent);
    FmGridControl::dispose();
}

VclPtr<BrowserHeader> SbaGridControl::imp_CreateHeaderBar(BrowseBox* pParent)
{
    return VclPtr<SbaGridHeader>::Create(pParent);
}

CellController* SbaGridControl::GetController(long nRow, sal_uInt16 nCol)
{
    if ( m_bActivatingForDrop )
        return nullptr;

    return FmGridControl::GetController(nRow, nCol);
}

void SbaGridControl::PreExecuteRowContextMenu(sal_uInt16 nRow, PopupMenu& rMenu)
{
    FmGridControl::PreExecuteRowContextMenu(nRow, rMenu);

    ScopedVclPtrInstance<PopupMenu> aNewItems(ModuleRes(RID_SBA_GRID_ROWCTXMENU));
    sal_uInt16 nPos = 0;

    if (!IsReadOnlyDB())
    {
        rMenu.InsertItem(ID_BROWSER_TABLEATTR, aNewItems->GetItemText(ID_BROWSER_TABLEATTR), MenuItemBits::NONE, OString(), nPos++);
        rMenu.SetHelpId(ID_BROWSER_TABLEATTR, aNewItems->GetHelpId(ID_BROWSER_TABLEATTR));

        rMenu.InsertItem(ID_BROWSER_ROWHEIGHT, aNewItems->GetItemText(ID_BROWSER_ROWHEIGHT), MenuItemBits::NONE, OString(), nPos++);
        rMenu.SetHelpId(ID_BROWSER_ROWHEIGHT, aNewItems->GetHelpId(ID_BROWSER_ROWHEIGHT));
        rMenu.InsertSeparator(OString(), nPos++);
    }

    if ( GetSelectRowCount() > 0 )
    {
        rMenu.InsertItem(ID_BROWSER_COPY, aNewItems->GetItemText(SID_COPY), MenuItemBits::NONE, OString(), nPos++);
        rMenu.SetHelpId(ID_BROWSER_COPY, aNewItems->GetHelpId(SID_COPY));

        rMenu.InsertSeparator(OString(), nPos++);
    }
}

SvNumberFormatter* SbaGridControl::GetDatasourceFormatter()
{
    Reference< css::util::XNumberFormatsSupplier >  xSupplier = ::dbtools::getNumberFormats(::dbtools::getConnection(Reference< XRowSet > (getDataSource(),UNO_QUERY)), true, getContext());

    SvNumberFormatsSupplierObj* pSupplierImpl = SvNumberFormatsSupplierObj::getImplementation( xSupplier );
    if ( !pSupplierImpl )
        return nullptr;

    SvNumberFormatter* pFormatter = pSupplierImpl->GetNumberFormatter();
    return pFormatter;
}

void SbaGridControl::SetColWidth(sal_uInt16 nColId)
{
    // get the (UNO) column model
    sal_uInt16 nModelPos = GetModelColumnPos(nColId);
    Reference< XIndexAccess >  xCols(GetPeer()->getColumns(), UNO_QUERY);
    Reference< XPropertySet >  xAffectedCol;
    if (xCols.is() && (nModelPos != (sal_uInt16)-1))
        xAffectedCol.set(xCols->getByIndex(nModelPos), css::uno::UNO_QUERY);

    if (xAffectedCol.is())
    {
        Any aWidth = xAffectedCol->getPropertyValue(PROPERTY_WIDTH);
        sal_Int32 nCurWidth = aWidth.hasValue() ? ::comphelper::getINT32(aWidth) : -1;

        ScopedVclPtrInstance< DlgSize > aDlgColWidth(this, nCurWidth, false);
        if (aDlgColWidth->Execute())
        {
            sal_Int32 nValue = aDlgColWidth->GetValue();
            Any aNewWidth;
            if (-1 == nValue)
            {   // set to default
                Reference< XPropertyState >  xPropState(xAffectedCol, UNO_QUERY);
                if (xPropState.is())
                {
                    try { aNewWidth = xPropState->getPropertyDefault(PROPERTY_WIDTH); } catch(Exception&) { } ;
                }
            }
            else
                aNewWidth <<= nValue;
            try {  xAffectedCol->setPropertyValue(PROPERTY_WIDTH, aNewWidth); } catch(Exception&) { } ;
        }
    }
}

void SbaGridControl::SetRowHeight()
{
    Reference< XPropertySet >  xCols(GetPeer()->getColumns(), UNO_QUERY);
    if (!xCols.is())
        return;

    Any aHeight = xCols->getPropertyValue(PROPERTY_ROW_HEIGHT);
    sal_Int32 nCurHeight = aHeight.hasValue() ? ::comphelper::getINT32(aHeight) : -1;

    ScopedVclPtrInstance< DlgSize > aDlgRowHeight(this, nCurHeight, true);
    if (aDlgRowHeight->Execute())
    {
        sal_Int32 nValue = aDlgRowHeight->GetValue();
        Any aNewHeight;
        if ((sal_Int16)-1 == nValue)
        {   // set to default
            Reference< XPropertyState >  xPropState(xCols, UNO_QUERY);
            if (xPropState.is())
            {
                try
                {
                    aNewHeight = xPropState->getPropertyDefault(PROPERTY_ROW_HEIGHT);
                }
                catch(Exception&)
                { }
            }
        }
        else
            aNewHeight <<= nValue;
        try
        {
            xCols->setPropertyValue(PROPERTY_ROW_HEIGHT, aNewHeight);
        }
        catch(Exception&)
        {
            OSL_FAIL("setPropertyValue: PROPERTY_ROW_HEIGHT throws a exception");
        }
    }
}

void SbaGridControl::SetColAttrs(sal_uInt16 nColId)
{
    SvNumberFormatter* pFormatter = GetDatasourceFormatter();
    if (!pFormatter)
        return;

    sal_uInt16 nModelPos = GetModelColumnPos(nColId);

    // get the (UNO) column model
    Reference< XIndexAccess >  xCols(GetPeer()->getColumns(), UNO_QUERY);
    Reference< XPropertySet >  xAffectedCol;
    if (xCols.is() && (nModelPos != (sal_uInt16)-1))
        xAffectedCol.set(xCols->getByIndex(nModelPos), css::uno::UNO_QUERY);

    // get the field the column is bound to
    Reference< XPropertySet >  xField = getField(nModelPos);
    ::dbaui::callColumnFormatDialog(xAffectedCol,xField,pFormatter,this);//(Window::GetSettings().GetLanguage());
}

void SbaGridControl::SetBrowserAttrs()
{
    Reference< XPropertySet >  xGridModel(GetPeer()->getColumns(), UNO_QUERY);
    if (!xGridModel.is())
        return;

    try
    {
        Reference< XComponentContext > xContext = getContext();
        Reference< XExecutableDialog > xExecute = ControlFontDialog::createWithGridModel( xContext, xGridModel);
        xExecute->execute();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void SbaGridControl::PostExecuteRowContextMenu(sal_uInt16 nRow, const PopupMenu& rMenu, sal_uInt16 nExecutionResult)
{
    switch (nExecutionResult)
    {
        case ID_BROWSER_TABLEATTR:
            SetBrowserAttrs();
            break;
        case ID_BROWSER_ROWHEIGHT:
            SetRowHeight();
            break;
        case ID_BROWSER_COPY:
            CopySelectedRowsToClipboard();
            break;

        default:
            FmGridControl::PostExecuteRowContextMenu(nRow, rMenu, nExecutionResult);
            break;
    }
}

void SbaGridControl::Select()
{
    // Some selection has changed ...
    FmGridControl::Select();

    if (m_pMasterListener)
        m_pMasterListener->SelectionChanged();
}

void SbaGridControl::ActivateCell(long nRow, sal_uInt16 nCol, bool bSetCellFocus /*= sal_True*/ )
{
    FmGridControl::ActivateCell(nRow, nCol, bSetCellFocus);
    if (m_pMasterListener)
        m_pMasterListener->CellActivated();
}

void SbaGridControl::DeactivateCell(bool bUpdate /*= sal_True*/)
{
    FmGridControl::DeactivateCell(bUpdate);
    if (m_pMasterListener)
        m_pMasterListener->CellDeactivated();
}

void SbaGridControl::onRowChange()
{
    if ( m_pMasterListener )
        m_pMasterListener->RowChanged();
}

void SbaGridControl::onColumnChange()
{
    if ( m_pMasterListener )
        m_pMasterListener->ColumnChanged();
}

Reference< XPropertySet >  SbaGridControl::getField(sal_uInt16 nModelPos)
{
    Reference< XPropertySet >  xEmptyReturn;
    try
    {
        // first get the name of the column
        Reference< XIndexAccess >  xCols(GetPeer()->getColumns(), UNO_QUERY);
        if ( xCols.is() && xCols->getCount() > nModelPos )
        {
            Reference< XPropertySet >  xCol(xCols->getByIndex(nModelPos),UNO_QUERY);
            if ( xCol.is() )
                xEmptyReturn.set(xCol->getPropertyValue(PROPERTY_BOUNDFIELD),UNO_QUERY);
        }
        else
            OSL_FAIL("SbaGridControl::getField getColumns returns NULL or ModelPos is > than count!");
    }
    catch (const Exception& e)
    {
        SAL_WARN("dbaccess", "SbaGridControl::getField Exception occurred: " << e.Message);
    }

    return xEmptyReturn;
}

bool SbaGridControl::IsReadOnlyDB() const
{
    // assume yes if anything fails
    bool bDBIsReadOnly = true;

    try
    {
        // the db is the implemented by the parent of the grid control's model ...
        Reference< XChild >  xColumns(GetPeer()->getColumns(), UNO_QUERY);
        if (xColumns.is())
        {
            Reference< XRowSet >  xDataSource(xColumns->getParent(), UNO_QUERY);
            ::dbtools::ensureRowSetConnection( xDataSource, getContext(), false ); // NOT SURE ABOUT FALSE
            Reference< XChild >  xConn(::dbtools::getConnection(xDataSource),UNO_QUERY);
            if (xConn.is())
            {
                // ... and the RO-flag simply is implemented by a property
                Reference< XPropertySet >  xDbProps(xConn->getParent(), UNO_QUERY);
                if (xDbProps.is())
                {
                    Reference< XPropertySetInfo >  xInfo = xDbProps->getPropertySetInfo();
                    if (xInfo->hasPropertyByName(PROPERTY_ISREADONLY))
                        bDBIsReadOnly = ::comphelper::getBOOL(xDbProps->getPropertyValue(PROPERTY_ISREADONLY));
                }
            }
        }
    }
    catch (const Exception& e)
    {
        SAL_WARN("dbaccess", "SbaGridControl::IsReadOnlyDB Exception occurred: " << e.Message);
    }

    return bDBIsReadOnly;
}

void SbaGridControl::MouseButtonDown( const BrowserMouseEvent& rMEvt)
{
    long nRow = GetRowAtYPosPixel(rMEvt.GetPosPixel().Y());
    sal_uInt16 nColPos = GetColumnAtXPosPixel(rMEvt.GetPosPixel().X());
    sal_uInt16 nViewPos = (nColPos == BROWSER_INVALIDID) ? (sal_uInt16)-1 : nColPos-1;
        // 'the handle column' and 'no valid column' will both result in a view position of -1 !

    bool bHitEmptySpace = (nRow > GetRowCount()) || (nViewPos == (sal_uInt16)-1);

    if (bHitEmptySpace && (rMEvt.GetClicks() == 2) && rMEvt.IsMod1())
        Control::MouseButtonDown(rMEvt);
    else
        FmGridControl::MouseButtonDown(rMEvt);
}

void SbaGridControl::StartDrag( sal_Int8 _nAction, const Point& _rPosPixel )
{
    SolarMutexGuard aGuard;
        // in the new DnD API, the solar mutex is not locked when StartDrag get's called

    bool bHandled = false;

    do
    {
        // determine if dragging is allowed
        // (Yes, this is controller (not view) functionality. But collecting and evaluating all the
        // information necessary via UNO would be quite difficult (if not impossible) so
        // my laziness says 'do it here'...)
        long nRow = GetRowAtYPosPixel(_rPosPixel.Y());
        sal_uInt16 nColPos = GetColumnAtXPosPixel(_rPosPixel.X());
        sal_uInt16 nViewPos = (nColPos == BROWSER_INVALIDID) ? (sal_uInt16)-1 : nColPos-1;
            // 'the handle column' and 'no valid column' will both result in a view position of -1 !

        bool bCurrentRowVirtual = IsCurrentAppending() && IsModified();
        // the current row doesn't really exist: the user's appending a new one and already has entered some data,
        // so the row contains data which has no counter part within the data source

        long nCorrectRowCount = GetRowCount();
        if (GetOptions() & DbGridControlOptions::Insert)
            --nCorrectRowCount; // there is a empty row for inserting records
        if (bCurrentRowVirtual)
            --nCorrectRowCount;

        if ((nColPos == BROWSER_INVALIDID) || (nRow >= nCorrectRowCount))
            break;

        bool bHitHandle = (nColPos == 0);

        // check which kind of dragging has to be initiated
        if  (   bHitHandle                          //  the handle column
                                                    // AND
            &&  (   GetSelectRowCount()             //  at least one row is selected
                                                    // OR
                ||  (   (nRow >= 0)                 //  a row below the header
                    &&  !bCurrentRowVirtual         //  we aren't appending a new record
                    &&  (nRow != GetCurrentPos())   //  a row which is not the current one
                    )                               // OR
                ||  (   (0 == GetSelectRowCount())  // no rows selected
                    &&  (-1 == nRow)                // hit the header
                    )
                )
            )
        {   // => start dragging the row
            if (GetDataWindow().IsMouseCaptured())
                GetDataWindow().ReleaseMouse();

            if (0 == GetSelectRowCount())
                // no rows selected, but here in this branch
                // -> the user started dragging the upper left corner, which symbolizes the whole table
                SelectAll();

            getMouseEvent().Clear();
            implTransferSelectedRows((sal_Int16)nRow, false);

            bHandled = true;
        }
        else if (   (nRow < 0)                      // the header
                &&  (!bHitHandle)                   // non-handle column
                &&  (nViewPos < GetViewColCount())  // valid (existing) column
                )
        {   // => start dragging the column
            if (GetDataWindow().IsMouseCaptured())
                GetDataWindow().ReleaseMouse();

            getMouseEvent().Clear();
            DoColumnDrag(nViewPos);

            bHandled = true;
        }
        else if (   !bHitHandle     // non-handle column
                &&  (nRow >= 0)     // non-header row
                )
        {   // => start dragging the field content
            if (GetDataWindow().IsMouseCaptured())
                GetDataWindow().ReleaseMouse();

            getMouseEvent().Clear();
            DoFieldDrag(nViewPos, (sal_Int16)nRow);

            bHandled = true;
        }
    }
    while (false);

    if (!bHandled)
        FmGridControl::StartDrag(_nAction, _rPosPixel);
}

void SbaGridControl::DoColumnDrag(sal_uInt16 nColumnPos)
{
    Reference< XPropertySet >  xDataSource(getDataSource(), UNO_QUERY);
    OSL_ENSURE(xDataSource.is(), "SbaGridControl::DoColumnDrag : invalid data source !");

    Reference< XPropertySet > xAffectedCol;
    Reference< XPropertySet > xAffectedField;
    Reference< XConnection > xActiveConnection;

    // determine the field to drag
    OUString sField;
    try
    {
        xActiveConnection = ::dbtools::getConnection(Reference< XRowSet >(getDataSource(),UNO_QUERY));

        sal_uInt16 nModelPos = GetModelColumnPos(GetColumnIdFromViewPos(nColumnPos));
        Reference< XIndexContainer >  xCols(GetPeer()->getColumns(), UNO_QUERY);
        xAffectedCol.set(xCols->getByIndex(nModelPos),UNO_QUERY);
        if (xAffectedCol.is())
        {
            xAffectedCol->getPropertyValue(PROPERTY_CONTROLSOURCE) >>= sField;
            xAffectedField.set(xAffectedCol->getPropertyValue(PROPERTY_BOUNDFIELD),UNO_QUERY);
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("SbaGridControl::DoColumnDrag : something went wrong while getting the column");
    }
    if (sField.isEmpty())
        return;

    OColumnTransferable* pDataTransfer = new OColumnTransferable(xDataSource, sField, xAffectedField, xActiveConnection, ColumnTransferFormatFlags::FIELD_DESCRIPTOR | ColumnTransferFormatFlags::COLUMN_DESCRIPTOR);
    Reference< XTransferable > xEnsureDelete = pDataTransfer;
    pDataTransfer->StartDrag(this, DND_ACTION_COPY | DND_ACTION_LINK);
}

void SbaGridControl::CopySelectedRowsToClipboard()
{
    OSL_ENSURE( GetSelectRowCount() > 0, "SbaGridControl::CopySelectedRowsToClipboard: invalid call!" );
    implTransferSelectedRows( (sal_Int16)FirstSelectedRow(), true );
}

void SbaGridControl::implTransferSelectedRows( sal_Int16 nRowPos, bool _bTrueIfClipboardFalseIfDrag )
{
    Reference< XPropertySet > xForm( getDataSource(), UNO_QUERY );
    OSL_ENSURE( xForm.is(), "SbaGridControl::implTransferSelectedRows: invalid form!" );

    // build the sequence of numbers of selected rows
    Sequence< Any > aSelectedRows;
    bool bSelectionBookmarks = true;

    // collect the affected rows
    if ((GetSelectRowCount() == 0) && (nRowPos >= 0))
    {
        aSelectedRows.realloc( 1 );
        aSelectedRows[0] <<= (sal_Int32)(nRowPos + 1);
        bSelectionBookmarks = false;
    }
    else if ( !IsAllSelected() && GetSelectRowCount() )
    {
        aSelectedRows = getSelectionBookmarks();
        bSelectionBookmarks = true;
    }

    Reference< XResultSet> xRowSetClone;
    try
    {
        ODataClipboard* pTransfer = new ODataClipboard( xForm, aSelectedRows, bSelectionBookmarks, getContext() );

        Reference< XTransferable > xEnsureDelete = pTransfer;
        if ( _bTrueIfClipboardFalseIfDrag )
            pTransfer->CopyToClipboard( this );
        else
            pTransfer->StartDrag(this, DND_ACTION_COPY | DND_ACTION_LINK);
    }
    catch(Exception&)
    {
    }
}

void SbaGridControl::DoFieldDrag(sal_uInt16 nColumnPos, sal_Int16 nRowPos)
{
    // the only thing to do here is dragging the pure cell text
    // the old implementation copied a SBA_FIELDDATAEXCHANGE_FORMAT, too, (which was rather expensive to obtain),
    // but we have no client for this DnD format anymore (the mail part of SO 5.2 was the only client)

    OUString sCellText;
    try
    {
        Reference< XGridFieldDataSupplier >  xFieldData(static_cast< XGridPeer* >(GetPeer()), UNO_QUERY);
        Sequence<sal_Bool> aSupportingText = xFieldData->queryFieldDataType(cppu::UnoType<decltype(sCellText)>::get());
        if (aSupportingText.getConstArray()[nColumnPos])
        {
            Sequence< Any> aCellContents = xFieldData->queryFieldData(nRowPos, cppu::UnoType<decltype(sCellText)>::get());
            sCellText = ::comphelper::getString(aCellContents.getConstArray()[nColumnPos]);
            ::svt::OStringTransfer::StartStringDrag(sCellText, this, DND_ACTION_COPY);
        }
    }
    catch(Exception&)
    {
        OSL_FAIL("SbaGridControl::DoFieldDrag : could not retrieve the cell's contents !");
        return;
    }

}

/// unary_function Functor object for class ZZ returntype is void
    struct SbaGridControlPrec : ::std::unary_function<DataFlavorExVector::value_type,bool>
    {
        inline bool operator()(const DataFlavorExVector::value_type& _aType)
        {
            switch (_aType.mnSotId)
            {
                case SotClipboardFormatId::DBACCESS_TABLE:   // table descriptor
                case SotClipboardFormatId::DBACCESS_QUERY:   // query descriptor
                case SotClipboardFormatId::DBACCESS_COMMAND: // SQL command
                    return true;
                default: break;
            }
            return false;
        }
    };
sal_Int8 SbaGridControl::AcceptDrop( const BrowserAcceptDropEvent& rEvt )
{
    sal_Int8 nAction = DND_ACTION_NONE;

    // we need a valid connection
    if (!::dbtools::getConnection(Reference< XRowSet > (getDataSource(),UNO_QUERY)).is())
        return nAction;

    if ( IsDropFormatSupported( SotClipboardFormatId::STRING ) ) do
    {   // odd construction, but spares us a lot of (explicit ;) goto's

        if (!GetEmptyRow().is())
            // without an empty row we're not in update mode
            break;

        long    nRow = GetRowAtYPosPixel(rEvt.maPosPixel.Y(), false);
        sal_uInt16  nCol = GetColumnAtXPosPixel(rEvt.maPosPixel.X(), false);

        long nCorrectRowCount = GetRowCount();
        if (GetOptions() & DbGridControlOptions::Insert)
            --nCorrectRowCount; // there is a empty row for inserting records
        if (IsCurrentAppending())
            --nCorrectRowCount; // the current data record doesn't really exist, we are appending a new one

        if ((nCol == BROWSER_INVALIDID) || (nRow >= nCorrectRowCount) || GetColumnId(nCol) == 0  || GetColumnId(nCol) == BROWSER_INVALIDID )
            // no valid cell under the mouse cursor
            break;

        Rectangle aRect = GetCellRect(nRow, nCol, false);
        if (!aRect.IsInside(rEvt.maPosPixel))
            // not dropped within a cell (a cell isn't as wide as the column - the are small spaces)
            break;

        if ((IsModified() || (GetCurrentRow().is() && GetCurrentRow()->IsModified())) && (GetCurrentPos() != nRow))
            // there is a current and modified row or cell and he text is to be dropped into another one
            break;

        CellControllerRef xCurrentController = Controller();
        if (xCurrentController.is() && xCurrentController->IsModified() && ((nRow != GetCurRow()) || (nCol != GetCurColumnId())))
            // the current controller is modified and the user wants to drop in another cell -> no chance
            // (when leaving the modified cell a error may occur - this is deadly while dragging)
            break;

        Reference< XPropertySet >  xField = getField(GetModelColumnPos(nCol));
        if (!xField.is())
            // the column is not valid bound (for instance a binary field)
            break;

        try
        {
            if (::comphelper::getBOOL(xField->getPropertyValue(PROPERTY_ISREADONLY)))
                break;
        }
        catch (const Exception& e )
        {
            (void)e; // make compiler happy
            // assume RO
            break;
        }

        try
        {
            // assume that text can be dropped into a field if the column has a css::awt::XTextComponent interface
            Reference< XIndexAccess >  xColumnControls(static_cast<css::form::XGridPeer*>(GetPeer()), UNO_QUERY);
            if (xColumnControls.is())
            {
                Reference< css::awt::XTextComponent >  xColControl(
                    xColumnControls->getByIndex(GetViewColumnPos(nCol)),
                    css::uno::UNO_QUERY);
                if (xColControl.is())
                {
                    m_bActivatingForDrop = true;
                    GoToRowColumnId(nRow, nCol);
                    m_bActivatingForDrop = false;

                    nAction = DND_ACTION_COPY;
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

    } while (false);

    if(nAction != DND_ACTION_COPY && GetEmptyRow().is())
    {
        const DataFlavorExVector& _rFlavors = GetDataFlavors();
        if(::std::any_of(_rFlavors.begin(),_rFlavors.end(),SbaGridControlPrec()))
            nAction = DND_ACTION_COPY;
    }

    return (DND_ACTION_NONE != nAction) ? nAction : FmGridControl::AcceptDrop(rEvt);
}

sal_Int8 SbaGridControl::ExecuteDrop( const BrowserExecuteDropEvent& rEvt )
{
    // we need some properties of our data source
    Reference< XPropertySet >  xDataSource = getDataSource();
    if (!xDataSource.is())
        return DND_ACTION_NONE;

    // we need a valid connection
    if (!::dbtools::getConnection(Reference< XRowSet > (xDataSource,UNO_QUERY)).is())
        return DND_ACTION_NONE;

    if ( IsDropFormatSupported( SotClipboardFormatId::STRING ) )
    {
        long    nRow = GetRowAtYPosPixel(rEvt.maPosPixel.Y(), false);
        sal_uInt16  nCol = GetColumnAtXPosPixel(rEvt.maPosPixel.X(), false);

        long nCorrectRowCount = GetRowCount();
        if (GetOptions() & DbGridControlOptions::Insert)
            --nCorrectRowCount; // there is a empty row for inserting records
        if (IsCurrentAppending())
            --nCorrectRowCount; // the current data record doesn't really exist, we are appending a new one

        OSL_ENSURE((nCol != BROWSER_INVALIDID) && (nRow < nCorrectRowCount), "SbaGridControl::Drop : dropped on an invalid position !");
            // AcceptDrop should have caught this

        // from now we work with ids instead of positions
        nCol = GetColumnId(nCol);

        GoToRowColumnId(nRow, nCol);
        if (!IsEditing())
            ActivateCell();

        CellControllerRef xCurrentController = Controller();
        if (!xCurrentController.is() || nullptr == dynamic_cast< const EditCellController* >(xCurrentController.get()))
            return DND_ACTION_NONE;
        Edit& rEdit = static_cast<Edit&>(xCurrentController->GetWindow());

        // get the dropped string
        TransferableDataHelper aDropped( rEvt.maDropEvent.Transferable );
        OUString sDropped;
        if ( !aDropped.GetString( SotClipboardFormatId::STRING, sDropped ) )
            return DND_ACTION_NONE;

        rEdit.SetText( sDropped );
        xCurrentController->SetModified();
        rEdit.Modify();
            // SetText itself doesn't call a Modify as it isn't a user interaction

        return DND_ACTION_COPY;
    }

    if(GetEmptyRow().is())
    {
        const DataFlavorExVector& _rFlavors = GetDataFlavors();
        if( ::std::any_of(_rFlavors.begin(),_rFlavors.end(), SbaGridControlPrec()) )
        {
            TransferableDataHelper aDropped( rEvt.maDropEvent.Transferable );
            m_aDataDescriptor = ODataAccessObjectTransferable::extractObjectDescriptor(aDropped);
            if (m_nAsyncDropEvent)
                Application::RemoveUserEvent(m_nAsyncDropEvent);
            m_nAsyncDropEvent = Application::PostUserEvent(LINK(this, SbaGridControl, AsynchDropEvent), nullptr, true);
            return DND_ACTION_COPY;
        }
    }

    return DND_ACTION_NONE;
}

Reference< XPropertySet >  SbaGridControl::getDataSource() const
{
    Reference< XPropertySet >  xReturn;

    Reference< XChild >  xColumns(GetPeer()->getColumns(), UNO_QUERY);
    Reference< XPropertySet >  xDataSource;
    if (xColumns.is())
        xReturn.set(xColumns->getParent(), UNO_QUERY);

    return xReturn;
}

IMPL_LINK_NOARG(SbaGridControl, AsynchDropEvent, void*, void)
{
    m_nAsyncDropEvent = nullptr;

    Reference< XPropertySet >  xDataSource = getDataSource();
    if ( xDataSource.is() )
    {
        bool bCountFinal = false;
        xDataSource->getPropertyValue(PROPERTY_ISROWCOUNTFINAL) >>= bCountFinal;
        if ( !bCountFinal )
            setDataSource(nullptr); // detach from grid control
        Reference< XResultSetUpdate > xResultSetUpdate(xDataSource,UNO_QUERY);
        ODatabaseImportExport* pImExport = new ORowSetImportExport(this,xResultSetUpdate,m_aDataDescriptor, getContext());
        Reference<XEventListener> xHolder = pImExport;
        Hide();
        try
        {
            pImExport->initialize(m_aDataDescriptor);
            if (m_pMasterListener)
                m_pMasterListener->BeforeDrop();
            if(!pImExport->Read())
            {
                OUString sError = OUString(ModuleRes(STR_NO_COLUMNNAME_MATCHING));
                throwGenericSQLException(sError,nullptr);
            }
            if (m_pMasterListener)
                m_pMasterListener->AfterDrop();
            Show();
        }
        catch(const SQLException& e)
        {
            if (m_pMasterListener)
                m_pMasterListener->AfterDrop();
            Show();
            ::dbaui::showError( ::dbtools::SQLExceptionInfo(e), this, getContext() );
        }
        catch(const Exception& )
        {
            if (m_pMasterListener)
                m_pMasterListener->AfterDrop();
            Show();
            DBG_UNHANDLED_EXCEPTION();
        }
        if ( !bCountFinal )
            setDataSource(Reference< XRowSet >(xDataSource,UNO_QUERY));
    }
    m_aDataDescriptor.clear();
}

OUString SbaGridControl::GetAccessibleObjectDescription( ::svt::AccessibleBrowseBoxObjType eObjType,sal_Int32 _nPosition) const
{
    OUString sRet;
    if ( ::svt::BBTYPE_BROWSEBOX == eObjType )
    {
        SolarMutexGuard aGuard;
        sRet = OUString(ModuleRes(STR_DATASOURCE_GRIDCONTROL_DESC));
    }
    else
        sRet = FmGridControl::GetAccessibleObjectDescription( eObjType,_nPosition);
    return sRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
