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

#include <brwview.hxx>
#include <sbagrid.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/types.hxx>
#include <vcl/split.hxx>
#include <strings.hxx>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <osl/diagnose.h>

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

namespace
{
    bool isGrabVclControlFocusAllowed(const UnoDataBrowserView* _pView)
    {
        bool bGrabFocus = false;
        SbaGridControl* pVclControl = _pView->getVclControl();
        const rtl::Reference< SbaXGridControl > xGrid = _pView->getGridControl();
        if (pVclControl && xGrid.is())
        {
            bGrabFocus = true;
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

UnoDataBrowserView::UnoDataBrowserView( vcl::Window* pParent,
                                        IController& _rController,
                                        const Reference< css::uno::XComponentContext >& _rxContext)
    :ODataView(pParent,_rController,_rxContext)
    ,m_pTreeView(nullptr)
    ,m_pSplitter(nullptr)
    ,m_pVclControl(nullptr)
{

}

void UnoDataBrowserView::Construct(const Reference< css::awt::XControlModel >& xModel)
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
        m_xGrid->setDesignMode(true);
        m_xGrid->setVisible(true);
        m_xGrid->setEnable(true);

        // introduce the model to the grid
        m_xGrid->setModel(xModel);
        // introduce the container (me) to the grid
        Reference< css::beans::XPropertySet >  xModelSet(xModel, UNO_QUERY);
        getContainer()->addControl(::comphelper::getString(xModelSet->getPropertyValue(PROPERTY_NAME)), m_xGrid);

        // get the VCL-control
        m_pVclControl = nullptr;
        getVclControl();

        OSL_ENSURE(m_pVclControl != nullptr, "UnoDataBrowserView::Construct : no real grid control !");
    }
    catch(const Exception&)
    {
        ::comphelper::disposeComponent(m_xGrid);
        throw;
    }
}

UnoDataBrowserView::~UnoDataBrowserView()
{
    disposeOnce();
}

void UnoDataBrowserView::dispose()
{
    m_pSplitter.disposeAndClear();
    setTreeView(nullptr);

    try
    {
        ::comphelper::disposeComponent(m_xGrid);
        ::comphelper::disposeComponent(m_xMe);
    }
    catch(const Exception&)
    {}
    m_pTreeView.reset();
    m_pVclControl.reset();
    ODataView::dispose();
}

IMPL_LINK_NOARG( UnoDataBrowserView, SplitHdl, Splitter*, void )
{
    tools::Long nYPos = m_pSplitter->GetPosPixel().Y();
    m_pSplitter->SetPosPixel( Point( m_pSplitter->GetSplitPosPixel(), nYPos ) );
    Resize();
}

void UnoDataBrowserView::setSplitter(Splitter* _pSplitter)
{
    m_pSplitter = _pSplitter;
    m_pSplitter->SetSplitHdl( LINK( this, UnoDataBrowserView, SplitHdl ) );
    LINK( this, UnoDataBrowserView, SplitHdl ).Call(m_pSplitter);
}

void UnoDataBrowserView::setTreeView(InterimDBTreeListBox* pTreeView)
{
    if (m_pTreeView.get() != pTreeView)
    {
        m_pTreeView.disposeAndClear();
        m_pTreeView = pTreeView;
    }
}

void UnoDataBrowserView::showStatus( const OUString& _rStatus )
{
    if (_rStatus.isEmpty())
        hideStatus();
    else
    {
        if (!m_pTreeView)
            return;
        weld::Label& rLabel = m_pTreeView->GetStatusBar();
        rLabel.set_label(_rStatus);
        rLabel.show();
        Resize();
        PaintImmediately();
    }
}

void UnoDataBrowserView::hideStatus()
{
    if (!m_pTreeView)
        return;
    weld::Label& rLabel = m_pTreeView->GetStatusBar();
    if (!rLabel.get_visible())
    {
        // nothing to do
        return;
    }
    rLabel.hide();
    Resize();
    PaintImmediately();
}

void UnoDataBrowserView::resizeDocumentView(tools::Rectangle& _rPlayground)
{
    Point   aSplitPos;
    Size    aSplitSize;
    Point   aPlaygroundPos( _rPlayground.TopLeft() );
    Size    aPlaygroundSize( _rPlayground.GetSize() );

    if (m_pTreeView && m_pTreeView->IsVisible() && m_pSplitter)
    {
        // calculate the splitter pos and size
        aSplitPos   = m_pSplitter->GetPosPixel();
        aSplitPos.setY( aPlaygroundPos.Y() );
        aSplitSize  = m_pSplitter->GetOutputSizePixel();
        aSplitSize.setHeight( aPlaygroundSize.Height() );

        if( ( aSplitPos.X() + aSplitSize.Width() ) > ( aPlaygroundSize.Width() ))
            aSplitPos.setX( aPlaygroundSize.Width() - aSplitSize.Width() );

        if( aSplitPos.X() <= aPlaygroundPos.X() )
            aSplitPos.setX( aPlaygroundPos.X() + sal_Int32(aPlaygroundSize.Width() * 0.2) );

        // the tree pos and size
        Point   aTreeViewPos( aPlaygroundPos );
        Size    aTreeViewSize( aSplitPos.X(), aPlaygroundSize.Height() );

        // set the size of treelistbox
        m_pTreeView->SetPosSizePixel( aTreeViewPos, aTreeViewSize );
        // Call this to trigger InterimItemWindow::Layout immediately, and
        // not later on idle so the statusbar will be shown to explain
        // a long delay on opening databases
        m_pTreeView->Resize();

        //set the size of the splitter
        m_pSplitter->SetPosSizePixel( aSplitPos, Size( aSplitSize.Width(), aPlaygroundSize.Height() ) );
        m_pSplitter->SetDragRectPixel( _rPlayground );
    }

    // set the size of grid control
    if (m_xGrid.is())
        m_xGrid->setPosSize( aSplitPos.X() + aSplitSize.Width(), aPlaygroundPos.Y(),
                                   aPlaygroundSize.Width() - aSplitSize.Width() - aSplitPos.X(), aPlaygroundSize.Height(), css::awt::PosSize::POSSIZE);

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
            Reference< css::awt::XWindowPeer >  xPeer = m_xGrid->getPeer();
            if ( xPeer.is() )
            {
                SbaXGridPeer* pPeer = dynamic_cast<SbaXGridPeer*>(xPeer.get());
                UnoDataBrowserView* pTHIS = const_cast<UnoDataBrowserView*>(this);
                if ( pPeer )
                {
                    m_pVclControl = static_cast<SbaGridControl*>(pPeer->GetWindow());
                    pTHIS->startComponentListening(VCLUnoHelper::GetInterface(m_pVclControl));
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
        bool bGrabFocus = false;
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

void UnoDataBrowserView::_disposing( const css::lang::EventObject& /*_rSource*/ )
{
    stopComponentListening(VCLUnoHelper::GetInterface(m_pVclControl));
    m_pVclControl = nullptr;
}

bool UnoDataBrowserView::PreNotify( NotifyEvent& rNEvt )
{
    bool bDone = false;
    if(rNEvt.GetType() == NotifyEventType::KEYINPUT)
    {
        bool bGrabAllowed = isGrabVclControlFocusAllowed(this);
        if ( bGrabAllowed )
        {
            const KeyEvent* pKeyEvt = rNEvt.GetKeyEvent();
            const vcl::KeyCode& rKeyCode = pKeyEvt->GetKeyCode();
            if (  ( rKeyCode == vcl::KeyCode( KEY_E, true, true, false, false ) )
               || ( rKeyCode == vcl::KeyCode( KEY_TAB, true, false, false, false ) )
               )
            {
                if ( m_pTreeView && m_pVclControl && m_pTreeView->HasChildPathFocus() )
                    m_pVclControl->GrabFocus();
                else if ( m_pTreeView && m_pVclControl && m_pVclControl->HasChildPathFocus() )
                    m_pTreeView->GrabFocus();

                bDone = true;
            }
        }
    }
    return bDone || ODataView::PreNotify(rNEvt);
}

BrowserViewStatusDisplay::BrowserViewStatusDisplay( UnoDataBrowserView* _pView, const OUString& _rStatus )
    :m_pView(_pView)
{

    if (m_pView)
        m_pView->showStatus(_rStatus);
}

BrowserViewStatusDisplay::~BrowserViewStatusDisplay( )
{
    if (m_pView)
        m_pView->showStatus(OUString());

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
