/*************************************************************************
 *
 *  $RCSfile: brwview.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:32:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SBX_BRWVIEW_HXX
#include "brwview.hxx"
#endif
#ifndef _SBA_GRID_HXX
#include "sbagrid.hxx"
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif
#ifndef DBACCESS_UI_DBTREEVIEW_HXX
#include "dbtreeview.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_BRW_HRC_
#include "dbu_brw.hrc"
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif
#ifndef _SBA_BWRCTRLR_HXX
#include "brwctrlr.hxx"
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif


using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::form;
//  using namespace ::com::sun::star::sdbcx;
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
//==================================================================
//= UnoDataBrowserView
//==================================================================

// -------------------------------------------------------------------------
UnoDataBrowserView::UnoDataBrowserView( Window* pParent,
                                        IController* _pController,
                                        const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rFactory)
    :ODataView(pParent,_pController,_rFactory)
    ,m_pVclControl(NULL)
    ,m_pSplitter(NULL)
    ,m_pTreeView(NULL)
    ,m_pStatus(NULL)
{
}
// -------------------------------------------------------------------------
void UnoDataBrowserView::Construct(const Reference< ::com::sun::star::awt::XControlModel >& xModel)
{
    try
    {
        ODataView::Construct();

        // our UNO representation
        m_xMe = VCLUnoHelper::CreateControlContainer(this);

        // create the (UNO-) control
        m_xGrid = new SbaXGridControl(getORB());
        DBG_ASSERT(m_xGrid.is(), "UnoDataBrowserView::Construct : could not create a grid control !");
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

        DBG_ASSERT(m_pVclControl != NULL, "UnoDataBrowserView::Construct : no real grid control !");
    }
    catch(Exception&)
    {
        ::comphelper::disposeComponent(m_xGrid);
        throw;
    }
}
// -------------------------------------------------------------------------
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
    catch(Exception)
    {}
}
// -----------------------------------------------------------------------------
IMPL_LINK( UnoDataBrowserView, SplitHdl, void*, p )
{
    long nTest = m_pSplitter->GetPosPixel().Y();
    m_pSplitter->SetPosPixel( Point( m_pSplitter->GetSplitPosPixel(), m_pSplitter->GetPosPixel().Y() ) );
    Resize();

    return 0L;
}
// -------------------------------------------------------------------------
void UnoDataBrowserView::setSplitter(Splitter* _pSplitter)
{
    m_pSplitter = _pSplitter;
    m_pSplitter->SetSplitHdl( LINK( this, UnoDataBrowserView, SplitHdl ) );
    LINK( this, UnoDataBrowserView, SplitHdl ).Call(m_pSplitter);
}
// -------------------------------------------------------------------------
void UnoDataBrowserView::setTreeView(DBTreeView* _pTreeView)
{
    if (m_pTreeView != _pTreeView)
    {
        if (m_pTreeView)
        {
            ::dbaui::notifySystemWindow(this,m_pTreeView,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
            ::std::auto_ptr<Window> aTemp(m_pTreeView);
            m_pTreeView = NULL;
        }
        m_pTreeView = _pTreeView;
        if ( m_pTreeView )
            ::dbaui::notifySystemWindow(this,m_pTreeView,::comphelper::mem_fun(&TaskPaneList::AddWindow));
    }
}
// -------------------------------------------------------------------------
void UnoDataBrowserView::showStatus( const String& _rStatus )
{
    if (0 == _rStatus.Len())
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

// -------------------------------------------------------------------------
void UnoDataBrowserView::hideStatus()
{
    if (!m_pStatus || !m_pStatus->IsVisible())
        // nothing to do
        return;
    m_pStatus->Hide();
    Resize();
    Update();
}

// -------------------------------------------------------------------------
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

//------------------------------------------------------------------
sal_uInt16 UnoDataBrowserView::Model2ViewPos(sal_uInt16 nPos) const
{
    return m_pVclControl ? m_pVclControl->GetViewColumnPos(m_pVclControl->GetColumnIdFromModelPos(nPos)) : -1;
}

//------------------------------------------------------------------
sal_uInt16 UnoDataBrowserView::View2ModelPos(sal_uInt16 nPos) const
{
    return m_pVclControl ? m_pVclControl->GetModelColumnPos(m_pVclControl->GetColumnIdFromViewPos(nPos)) : -1;
}

//------------------------------------------------------------------
sal_uInt16 UnoDataBrowserView::ViewColumnCount() const
{
    return m_pVclControl ? m_pVclControl->GetViewColCount() : 0;
}
// -----------------------------------------------------------------------------
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

                ::dbaui::notifySystemWindow(pTHIS,m_pVclControl,::comphelper::mem_fun(&TaskPaneList::AddWindow));
            }
        }
    }
    return m_pVclControl;
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
void UnoDataBrowserView::_disposing( const ::com::sun::star::lang::EventObject& _rSource )
{
    stopComponentListening(Reference<XComponent>(VCLUnoHelper::GetInterface(m_pVclControl),UNO_QUERY));
    ::dbaui::notifySystemWindow(this,m_pVclControl,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
    m_pVclControl = NULL;
}
// -------------------------------------------------------------------------
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
            if( rKeyCode == KeyCode(KEY_E,TRUE,TRUE,FALSE) )
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

// -----------------------------------------------------------------------------
BrowserViewStatusDisplay::BrowserViewStatusDisplay( UnoDataBrowserView* _pView, const String& _rStatus )
    :m_pView(_pView)
{
    if (m_pView)
        m_pView->showStatus(_rStatus);
}

// -----------------------------------------------------------------------------
BrowserViewStatusDisplay::~BrowserViewStatusDisplay( )
{
    if (m_pView)
        m_pView->showStatus(String());
}
// -----------------------------------------------------------------------------
