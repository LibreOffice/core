/*************************************************************************
 *
 *  $RCSfile: brwview.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-19 12:39:46 $
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
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif
#ifndef _SBA_BWRCTRLR_HXX
#include "brwctrlr.hxx"
#endif


using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::form;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

//==================================================================
//= UnoDataBrowserView
//==================================================================

// -------------------------------------------------------------------------
UnoDataBrowserView::UnoDataBrowserView(Window* pParent, const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rFactory)
    :ODataView(pParent,_rFactory)
    ,m_pVclControl(NULL)
    ,m_pSplitter(NULL)
    ,m_pTreeView(NULL)
{
}
// -------------------------------------------------------------------------
void UnoDataBrowserView::Construct(const Reference< ::com::sun::star::awt::XControlModel >& xModel)
{
    try
    {
        ODataView::Construct(xModel);
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
        Reference< ::com::sun::star::awt::XWindowPeer >  xPeer = m_xGrid->getPeer();
        if (xPeer.is())
        {
            SbaXGridPeer* pPeer = SbaXGridPeer::getImplementation(xPeer);
            if (pPeer)
                m_pVclControl = static_cast<SbaGridControl*>(pPeer->GetWindow());
        }

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
    m_pVclControl = NULL;

    delete m_pSplitter;
    m_pSplitter = NULL;
    if(m_pTreeView)
    {
        delete m_pTreeView;
        m_pTreeView = NULL;
    }
    ::comphelper::disposeComponent(m_xGrid);
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
            delete m_pTreeView;
        m_pTreeView = _pTreeView;
    }
}
// -------------------------------------------------------------------------
void UnoDataBrowserView::resizeControl(Rectangle& _rRect)
{
    Point   aSplitPos(0,0);
    Size    aSplitSize(0,0);
    Size    aNewSize( _rRect.GetSize() );

    Size aToolBoxSize;
    ToolBox* pToolBox = getToolBox();
    if(pToolBox)
    {
        aToolBoxSize = pToolBox->GetOutputSizePixel();
        pToolBox->SetSizePixel(aToolBoxSize);
    }

    if (m_pTreeView && m_pTreeView->IsVisible() && m_pSplitter)
    {

        aSplitPos   = m_pSplitter->GetPosPixel();
        aSplitPos.Y() = aToolBoxSize.Height();
        aSplitSize  = m_pSplitter->GetOutputSizePixel();
        aSplitSize.Height() = aNewSize.Height() - aToolBoxSize.Height();

        if( ( aSplitPos.X() + aSplitSize.Width() ) > ( aNewSize.Width() ))
            aSplitPos.X() = aNewSize.Width() - aSplitSize.Width();

        if( aSplitPos.X() <= 0)
            aSplitPos.X() = LogicToPixel( Size(sal_Int32(aNewSize.Width() * 0.2), 0 ), MAP_APPFONT ).Width();

        // set the size of treelistbox
        m_pTreeView->SetPosSizePixel(   Point( 0, aToolBoxSize.Height() ),
                                        Size( aSplitPos.X(), aNewSize.Height() - aToolBoxSize.Height() ) );
        //set the size of the splitter
        m_pSplitter->SetPosSizePixel( aSplitPos, Size( aSplitSize.Width(), aNewSize.Height() - aToolBoxSize.Height() ) );
        m_pSplitter->SetDragRectPixel(  Rectangle( Point( 0, aToolBoxSize.Height() ),
                                        Size( aNewSize.Width(), aNewSize.Height() - aToolBoxSize.Height() ) ) );
    }

    // set the size of grid control
    Reference< ::com::sun::star::awt::XWindow >  xGridAsWindow(m_xGrid, UNO_QUERY);
    if (xGridAsWindow.is())
        xGridAsWindow->setPosSize( aSplitPos.X() + aSplitSize.Width(), aToolBoxSize.Height() ,
                                   aNewSize.Width() - aSplitSize.Width() - aSplitPos.X(),_rRect.GetHeight() - aToolBoxSize.Height(), ::com::sun::star::awt::PosSize::POSSIZE);

    // set the rect for the baseclass
    _rRect.SetPos(Point(0, 0));
    _rRect.SetSize(Size(aNewSize.Width(),aToolBoxSize.Height()));
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

//------------------------------------------------------------------
void UnoDataBrowserView::GetFocus()
{
    Window::GetFocus();
    if (m_pVclControl && m_xGrid.is())
    {
        sal_Bool bGrabFocus = sal_True;
        if(!m_pVclControl->HasChildPathFocus())
        {
            Reference<XChild> xChild(m_xGrid->getModel(),UNO_QUERY);
            Reference<XLoadable> xLoad;
            if(xChild.is())
                xLoad = Reference<XLoadable>(xChild->getParent(),UNO_QUERY);
            bGrabFocus = !(xLoad.is() && xLoad->isLoaded());
            if(!bGrabFocus)
                m_pVclControl->GrabFocus();
        }
        if(bGrabFocus && m_pTreeView)
            m_pTreeView->GrabFocus();
    }
    else if(m_pTreeView)
        m_pTreeView->GrabFocus();
}
// -------------------------------------------------------------------------
long UnoDataBrowserView::PreNotify( NotifyEvent& rNEvt )
{
    long nDone = 0L;
    if(rNEvt.GetType() == EVENT_KEYINPUT)
    {
        const KeyEvent* pKeyEvt = rNEvt.GetKeyEvent();
        const KeyCode& rKeyCode = pKeyEvt->GetKeyCode();
        if(rKeyCode  == KeyCode(KEY_E,TRUE,TRUE,FALSE))
        {
            if(m_pTreeView && m_pVclControl->HasChildPathFocus())
                m_pTreeView->GrabFocus();
            else
                m_pVclControl->GrabFocus();
            nDone = 1L;
        }
    }
    return nDone ? nDone : Window::PreNotify(rNEvt);
}
// -----------------------------------------------------------------------------




