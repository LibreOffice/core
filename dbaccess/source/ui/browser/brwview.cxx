/*************************************************************************
 *
 *  $RCSfile: brwview.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-06 17:41:04 $
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
#ifndef _SFXAPP_HXX //autogen wg. SFX_APP
#include <sfx2/app.hxx>
#endif
#ifndef _SFXIMGMGR_HXX //autogen wg. SFX_IMAGEMANAGER
#include <sfx2/imgmgr.hxx>
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


using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

//==================================================================
//= UnoDataBrowserView
//==================================================================

// -------------------------------------------------------------------------
UnoDataBrowserView::UnoDataBrowserView(Window* pParent, const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rFactory)
:Window(pParent,WB_BORDER)
    ,m_pToolBox(NULL)
    ,m_pVclControl(NULL)
    ,m_xServiceFactory(_rFactory)
    ,m_pSplitter(NULL)
    ,m_pTreeView(NULL)
{
}
// -------------------------------------------------------------------------
void UnoDataBrowserView::Construct(const Reference< ::com::sun::star::awt::XControlModel >& xModel)
{
    try
    {
        // our UNO representation
        m_xMe = VCLUnoHelper::CreateControlContainer(this);
        DBG_ASSERT(m_xMe.is(), "UnoDataBrowserView::Construct : no UNO representation");

        // create the (UNO-) control
        m_xGrid = new SbaXGridControl(m_xServiceFactory);
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
        m_xMe->addControl(::comphelper::getString(xModelSet->getPropertyValue(PROPERTY_NAME)), m_xGrid);

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
    catch(...)
    {
        ::comphelper::disposeComponent(m_xGrid);
        ::comphelper::disposeComponent(m_xMe);
        throw;
    }
}
// -------------------------------------------------------------------------
UnoDataBrowserView::~UnoDataBrowserView()
{
    setToolBox(NULL);

    m_pVclControl = NULL;

    delete m_pSplitter;
    m_pSplitter = NULL;
    if(m_pTreeView)
    {
        delete m_pTreeView;
        m_pTreeView = NULL;
    }
    ::comphelper::disposeComponent(m_xMe);
    ::comphelper::disposeComponent(m_xGrid);
}
// -----------------------------------------------------------------------------
IMPL_LINK( UnoDataBrowserView, SplitHdl, void*, p )
{
    m_pSplitter->SetPosPixel( Point( m_pSplitter->GetSplitPosPixel(), m_pSplitter->GetPosPixel().Y() ) );
    Resize();

    return 0L;
}
// -------------------------------------------------------------------------
void UnoDataBrowserView::setToolBox(ToolBox* pTB)
{
    if (pTB == m_pToolBox)
        return;

    if (m_pToolBox)
    {
        SFX_IMAGEMANAGER()->ReleaseToolBox(m_pToolBox);
        delete m_pToolBox;
    }

    m_pToolBox = pTB;
    if (m_pToolBox)
    {
        SFX_IMAGEMANAGER()->RegisterToolBox(m_pToolBox, SFX_TOOLBOX_CHANGEOUTSTYLE);

        m_pToolBox->SetParent(this);
        m_pToolBox->Show();
    }

    // rearrange the grid and the TB
    Resize();
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
    if (m_pTreeView)
        delete m_pTreeView;
    m_pTreeView = _pTreeView;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void UnoDataBrowserView::Resize()
{
    Window::Resize();
    Point   aSplitPos(0,0);
    Size    aSplitSize(0,0);
    Size    aNewSize( GetOutputSizePixel() );

    if(m_pTreeView && m_pSplitter)
    {

        aSplitPos   = m_pSplitter->GetPosPixel();
        aSplitSize  = m_pSplitter->GetOutputSizePixel();

        if( ( aSplitPos.X() + aSplitSize.Width() ) > ( aNewSize.Width() ))
            aSplitPos.X() = aNewSize.Width() - aSplitSize.Width();

        if( aSplitPos.X() <= 0)
            aSplitPos.X() = LogicToPixel( Size(sal_Int32(aNewSize.Width() * 0.2), 0 ), MAP_APPFONT ).Width();
        // set the size of treelistbox
        m_pTreeView->SetPosSizePixel(   Point( 0, 0 ),
                                        Size( aSplitPos.X(), aNewSize.Height() ) );
        //set the size of the splitter
        m_pSplitter->SetPosSizePixel( aSplitPos, Size( aSplitSize.Width(), aNewSize.Height() ) );
        m_pSplitter->SetDragRectPixel(  Rectangle( Point( 0, 0 ),
                                        Size( aNewSize.Width(), aNewSize.Height() ) ) );
    }

    // set the size of the toolbox
    Size aToolBoxSize(0,0);
    if (m_pToolBox)
    {
        aToolBoxSize = m_pToolBox->GetSizePixel();
        m_pToolBox->SetPosSizePixel(Point(aSplitPos.X() + aSplitSize.Width(), 0),
            Size(aNewSize.Width() - aSplitSize.Width() - aSplitPos.X(), aToolBoxSize.Height()));
    }
    // set the size of grid control
    Reference< ::com::sun::star::awt::XWindow >  xGridAsWindow(m_xGrid, UNO_QUERY);
    if (xGridAsWindow.is())
        xGridAsWindow->setPosSize( aSplitPos.X() + aSplitSize.Width(), aToolBoxSize.Height() ,
                                   aNewSize.Width() - aSplitSize.Width() - aSplitPos.X(), aNewSize.Height() - aToolBoxSize.Height(), ::com::sun::star::awt::PosSize::POSSIZE);


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
    if (m_pVclControl)
        m_pVclControl->GrabFocus();
}
// -------------------------------------------------------------------------



