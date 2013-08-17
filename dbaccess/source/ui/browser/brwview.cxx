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

#include "brwctrlr.hxx"
#include "brwview.hxx"
#include "sbagrid.hxx"
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/types.hxx>
#include <vcl/split.hxx>
#include "dbtreeview.hxx"
#include "dbustrings.hrc"
#include "dbu_brw.hrc"
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include "UITools.hxx"
#include <osl/diagnose.h>

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

namespace
{
    sal_Bool isGrabVclControlFocusAllowed(const UnoDataBrowserView* _pView)
    {
        sal_Bool bGrabFocus = sal_False;
        SbaGridControl* pVclControl = _pView->getVclControl();
        Reference< ::com::sun::star::awt::XControl > xGrid = _pView->getGridControl();
        if (pVclControl && xGrid.is())
        {
            bGrabFocus = sal_True;
            if(!pVclControl->HasChildPathFocus())
            {
                Reference<XChild> xChild(xGrid->getModel(),UNO_QUERY);
                Reference<XLoadable> xLoad;
                if(xChild.is())
                    xLoad.set(xChild->getParent(),UNO_QUERY);
                bGrabFocus = xLoad.is() && xLoad->isLoaded();
            }
        }
        return bGrabFocus;
    }
}

// UnoDataBrowserView

DBG_NAME(UnoDataBrowserView)
UnoDataBrowserView::UnoDataBrowserView( Window* pParent,
                                        IController& _rController,
                                        const Reference< ::com::sun::star::uno::XComponentContext >& _rxContext)
    :ODataView(pParent,_rController,_rxContext)
    ,m_pTreeView(NULL)
    ,m_pSplitter(NULL)
    ,m_pVclControl(NULL)
    ,m_pStatus(NULL)
{
    DBG_CTOR(UnoDataBrowserView,NULL);

}

void UnoDataBrowserView::Construct(const Reference< ::com::sun::star::awt::XControlModel >& xModel)
{
    try
    {
        ODataView::Construct();

        // our UNO representation
        m_xMe = VCLUnoHelper::CreateControlContainer(this);

        // create the (UNO-) control
        m_xGrid = new SbaXGridControl( getORB() );
        OSL_ENSURE(m_xGrid.is(), "UnoDataBrowserView::Construct : could not create a grid control !");
        // in design mode (for the moment)
        m_xGrid->setDesignMode(sal_True);

        Reference< ::com::sun::star::awt::XWindow >  xGridWindow(m_xGrid, UNO_QUERY);
        xGridWindow->setVisible(sal_True);
        xGridWindow->setEnable(sal_True);

        // introduce the model to the grid
        m_xGrid->setModel(xModel);
        // introduce the container (me) to the grid
        Reference< ::com::sun::star::beans::XPropertySet >  xModelSet(xModel, UNO_QUERY);
        getContainer()->addControl(::comphelper::getString(xModelSet->getPropertyValue(PROPERTY_NAME)), m_xGrid);

        // get the VCL-control
        m_pVclControl = NULL;
        getVclControl();

        OSL_ENSURE(m_pVclControl != NULL, "UnoDataBrowserView::Construct : no real grid control !");
    }
    catch(const Exception&)
    {
        ::comphelper::disposeComponent(m_xGrid);
        throw;
    }
}

UnoDataBrowserView::~UnoDataBrowserView()
{
    {
        ::std::auto_ptr<Splitter> aTemp(m_pSplitter);
        m_pSplitter = NULL;
    }
    setTreeView(NULL);

    if ( m_pStatus )
    {
        delete m_pStatus;
        m_pStatus = NULL;
    }

    try
    {
        ::comphelper::disposeComponent(m_xGrid);
        ::comphelper::disposeComponent(m_xMe);
    }
    catch(const Exception&)
    {}

    DBG_DTOR(UnoDataBrowserView,NULL);
}

IMPL_LINK( UnoDataBrowserView, SplitHdl, void*, /*NOINTERESTEDIN*/ )
{
    long nYPos = m_pSplitter->GetPosPixel().Y();
    m_pSplitter->SetPosPixel( Point( m_pSplitter->GetSplitPosPixel(), nYPos ) );
    Resize();

    return 0L;
}

void UnoDataBrowserView::setSplitter(Splitter* _pSplitter)
{
    m_pSplitter = _pSplitter;
    m_pSplitter->SetSplitHdl( LINK( this, UnoDataBrowserView, SplitHdl ) );
    LINK( this, UnoDataBrowserView, SplitHdl ).Call(m_pSplitter);
}

void UnoDataBrowserView::setTreeView(DBTreeView* _pTreeView)
{
    if (m_pTreeView != _pTreeView)
    {
        if (m_pTreeView)
        {
            ::std::auto_ptr<Window> aTemp(m_pTreeView);
            m_pTreeView = NULL;
        }
        m_pTreeView = _pTreeView;
    }
}

void UnoDataBrowserView::showStatus( const OUString& _rStatus )
{
    if (_rStatus.isEmpty())
        hideStatus();
    else
    {
        if (!m_pStatus)
            m_pStatus = new FixedText(this);
        m_pStatus->SetText(_rStatus);
        m_pStatus->Show();
        Resize();
        Update();
    }
}

void UnoDataBrowserView::hideStatus()
{
    if (!m_pStatus || !m_pStatus->IsVisible())
        // nothing to do
        return;
    m_pStatus->Hide();
    Resize();
    Update();
}

void UnoDataBrowserView::resizeDocumentView(Rectangle& _rPlayground)
{
    Point   aSplitPos;
    Size    aSplitSize;
    Point   aPlaygroundPos( _rPlayground.TopLeft() );
    Size    aPlaygroundSize( _rPlayground.GetSize() );

    if (m_pTreeView && m_pTreeView->IsVisible() && m_pSplitter)
    {
        // calculate the splitter pos and size
        aSplitPos   = m_pSplitter->GetPosPixel();
        aSplitPos.Y() = aPlaygroundPos.Y();
        aSplitSize  = m_pSplitter->GetOutputSizePixel();
        aSplitSize.Height() = aPlaygroundSize.Height();

        if( ( aSplitPos.X() + aSplitSize.Width() ) > ( aPlaygroundSize.Width() ))
            aSplitPos.X() = aPlaygroundSize.Width() - aSplitSize.Width();

        if( aSplitPos.X() <= aPlaygroundPos.X() )
            aSplitPos.X() = aPlaygroundPos.X() + sal_Int32(aPlaygroundSize.Width() * 0.2);

        // the tree pos and size
        Point   aTreeViewPos( aPlaygroundPos );
        Size    aTreeViewSize( aSplitPos.X(), aPlaygroundSize.Height() );

        // the status pos and size
        if (m_pStatus && m_pStatus->IsVisible())
        {
            Size aStatusSize(aPlaygroundPos.X(), GetTextHeight() + 2);
            aStatusSize = LogicToPixel(aStatusSize, MAP_APPFONT);
            aStatusSize.Width() = aTreeViewSize.Width() - 2 - 2;

            Point aStatusPos( aPlaygroundPos.X() + 2, aTreeViewPos.Y() + aTreeViewSize.Height() - aStatusSize.Height() );
            m_pStatus->SetPosSizePixel( aStatusPos, aStatusSize );
            aTreeViewSize.Height() -= aStatusSize.Height();
        }

        // set the size of treelistbox
        m_pTreeView->SetPosSizePixel( aTreeViewPos, aTreeViewSize );

        //set the size of the splitter
        m_pSplitter->SetPosSizePixel( aSplitPos, Size( aSplitSize.Width(), aPlaygroundSize.Height() ) );
        m_pSplitter->SetDragRectPixel( _rPlayground );
    }

    // set the size of grid control
    Reference< ::com::sun::star::awt::XWindow >  xGridAsWindow(m_xGrid, UNO_QUERY);
    if (xGridAsWindow.is())
        xGridAsWindow->setPosSize( aSplitPos.X() + aSplitSize.Width(), aPlaygroundPos.Y(),
                                   aPlaygroundSize.Width() - aSplitSize.Width() - aSplitPos.X(), aPlaygroundSize.Height(), ::com::sun::star::awt::PosSize::POSSIZE);

    // just for completeness: there is no space left, we occupied it all ...
    _rPlayground.SetPos( _rPlayground.BottomRight() );
    _rPlayground.SetSize( Size( 0, 0 ) );
}

sal_uInt16 UnoDataBrowserView::View2ModelPos(sal_uInt16 nPos) const
{
    return m_pVclControl ? m_pVclControl->GetModelColumnPos(m_pVclControl->GetColumnIdFromViewPos(nPos)) : -1;
}

SbaGridControl* UnoDataBrowserView::getVclControl() const
{
    if ( !m_pVclControl )
    {
        OSL_ENSURE(m_xGrid.is(),"Grid not set!");
        if ( m_xGrid.is() )
        {
            Reference< ::com::sun::star::awt::XWindowPeer >  xPeer = m_xGrid->getPeer();
            if ( xPeer.is() )
            {
                SbaXGridPeer* pPeer = SbaXGridPeer::getImplementation(xPeer);
                UnoDataBrowserView* pTHIS = const_cast<UnoDataBrowserView*>(this);
                if ( pPeer )
                {
                    m_pVclControl = static_cast<SbaGridControl*>(pPeer->GetWindow());
                    pTHIS->startComponentListening(Reference<XComponent>(VCLUnoHelper::GetInterface(m_pVclControl),UNO_QUERY));
                }
            }
        }
    }
    return m_pVclControl;
}

void UnoDataBrowserView::GetFocus()
{
    ODataView::GetFocus();
    if( m_pTreeView && m_pTreeView->IsVisible() && !m_pTreeView->HasChildPathFocus())
        m_pTreeView->GrabFocus();
    else if (m_pVclControl && m_xGrid.is())
    {
        sal_Bool bGrabFocus = sal_False;
        if(!m_pVclControl->HasChildPathFocus())
        {
            bGrabFocus = isGrabVclControlFocusAllowed(this);
            if( bGrabFocus )
                m_pVclControl->GrabFocus();
        }
        if(!bGrabFocus && m_pTreeView && m_pTreeView->IsVisible() )
            m_pTreeView->GrabFocus();
    }
}

void UnoDataBrowserView::_disposing( const ::com::sun::star::lang::EventObject& /*_rSource*/ )
{
    stopComponentListening(Reference<XComponent>(VCLUnoHelper::GetInterface(m_pVclControl),UNO_QUERY));
    m_pVclControl = NULL;
}

long UnoDataBrowserView::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0L;
    if(rNEvt.GetType() == EVENT_KEYINPUT)
    {
        sal_Bool bGrabAllowed = isGrabVclControlFocusAllowed(this);
        if ( bGrabAllowed )
        {
            const KeyEvent* pKeyEvt = rNEvt.GetKeyEvent();
            const KeyCode& rKeyCode = pKeyEvt->GetKeyCode();
            if (  ( rKeyCode == KeyCode( KEY_E, sal_True, sal_True, sal_False, sal_False ) )
               || ( rKeyCode == KeyCode( KEY_TAB, sal_True, sal_False, sal_False, sal_False ) )
               )
            {
                if ( m_pTreeView && m_pVclControl && m_pTreeView->HasChildPathFocus() )
                    m_pVclControl->GrabFocus();
                else if ( m_pTreeView && m_pVclControl && m_pVclControl->HasChildPathFocus() )
                    m_pTreeView->GrabFocus();

                nDone = 1L;
            }
        }
    }
    return nDone ? nDone : ODataView::PreNotify(rNEvt);
}

DBG_NAME(BrowserViewStatusDisplay)
BrowserViewStatusDisplay::BrowserViewStatusDisplay( UnoDataBrowserView* _pView, const OUString& _rStatus )
    :m_pView(_pView)
{
    DBG_CTOR(BrowserViewStatusDisplay,NULL);

    if (m_pView)
        m_pView->showStatus(_rStatus);
}

BrowserViewStatusDisplay::~BrowserViewStatusDisplay( )
{
    if (m_pView)
        m_pView->showStatus(OUString());

    DBG_DTOR(BrowserViewStatusDisplay,NULL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
