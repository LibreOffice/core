/*************************************************************************
 *
 *  $RCSfile: QueryViewSwitch.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-14 10:35:11 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBAUI_QUERYVIEWSWITCH_HXX
#include "QueryViewSwitch.hxx"
#endif
#ifndef DBAUI_QUERYDESIGNVIEW_HXX
#include "QueryDesignView.hxx"
#endif
#ifndef DBAUI_QUERYVIEW_TEXT_HXX
#include "QueryTextView.hxx"
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef DBAUI_QYDLGTAB_HXX
#include "adtabdlg.hxx"
#endif
#ifndef DBAUI_QUERYCONTROLLER_HXX
#include "querycontroller.hxx"
#endif


using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;


OQueryViewSwitch::OQueryViewSwitch(Window* _pParent, OQueryController* _pController,const Reference< XMultiServiceFactory >& _rFactory)
    //  :OQueryView(_pParent,_pController,_rFactory)
{
    ToolBox* pToolBox = new ToolBox(_pParent, ModuleRes(RID_BRW_QUERYDESIGN_TOOLBOX));

    m_pTextView     = new OQueryTextView(_pParent,pToolBox);
    m_pDesignView   = new OQueryDesignView(_pParent,_pController,_rFactory);

    m_pDesignView->setToolBox(pToolBox);
    m_pTextView->Show();
    pToolBox->SetParent(m_pTextView); // change owner ship
    pToolBox->Show();

    if(pToolBox && m_pTextView->IsVisible())
    {
        pToolBox->HideItem(ID_BROWSER_QUERY_DISTINCT_VALUES);
        pToolBox->HideItem(ID_BROWSER_QUERY_VIEW_ALIASES);
        pToolBox->HideItem(ID_BROWSER_QUERY_VIEW_TABLES);
        pToolBox->HideItem(ID_BROWSER_QUERY_VIEW_FUNCTIONS);
        pToolBox->HideItem(ID_BROWSER_ADDTABLE);
        pToolBox->HideItem(ID_QUERY_ZOOM_IN);
        pToolBox->HideItem(ID_QUERY_ZOOM_OUT);
        //  ToolBoxItemType eType = pToolBox->GetItemType(pToolBox->GetItemPos(ID_BROWSER_SQL)+1);
        //  pToolBox->HideItem(pToolBox->GetItemId(pToolBox->GetItemPos(ID_BROWSER_SQL))+1); // hide the separator
    }
}
// -----------------------------------------------------------------------------
OQueryViewSwitch::~OQueryViewSwitch()
{
    ToolBox* pToolBox = m_pDesignView->getToolBox();
    pToolBox->SetParent(m_pDesignView); // change owner ship again to real owner

    delete m_pTextView;
    //  delete m_pDesignView; // will be deleted by XFrame
}
// -------------------------------------------------------------------------
void OQueryViewSwitch::Construct(const Reference< ::com::sun::star::awt::XControlModel >& xModel)
{
    m_pDesignView->Construct(xModel); // initialize the table view
    //  OQueryView::Construct(xModel); // initialize m_xMe
    //  m_pTextView->Construct(xModel);

}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::initialize()
{
    m_pDesignView->initialize();
    if(static_cast<OQueryController*>(m_pDesignView->getController())->isDesignMode())
        switchView();
}
// -------------------------------------------------------------------------
void OQueryViewSwitch::resizeControl(Rectangle& _rRect)
{
    Size aToolBoxSize;
    ToolBox* pToolBox = m_pDesignView->getToolBox();
    if(pToolBox)
        aToolBoxSize = pToolBox->GetOutputSizePixel();

    Point aStart(_rRect.TopLeft());
    aStart.Y() += aToolBoxSize.Height();

    m_pTextView->SetPosSizePixel(aStart,Size(_rRect.GetSize().Width(),_rRect.GetSize().Height()-aToolBoxSize.Height()));
    m_pDesignView->SetPosSizePixel(aStart,Size(_rRect.GetSize().Width(),_rRect.GetSize().Height()-aToolBoxSize.Height()));

    aToolBoxSize.Width() += _rRect.getWidth();
    _rRect.SetSize(aToolBoxSize);
}
// -----------------------------------------------------------------------------
::rtl::OUString OQueryViewSwitch::getStatement()
{
    if(m_pTextView->IsVisible())
        return m_pTextView->getStatement();
    return m_pDesignView->getStatement();
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::setReadOnly(sal_Bool _bReadOnly)
{
    if(m_pTextView->IsVisible())
        m_pTextView->setReadOnly(_bReadOnly);
    else
        m_pDesignView->setReadOnly(_bReadOnly);
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::clear()
{
    if(m_pTextView->IsVisible())
        m_pTextView->clear();
    else
        m_pDesignView->clear();
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::setStatement(const ::rtl::OUString& _rsStatement)
{
    if(m_pTextView->IsVisible())
        m_pTextView->setStatement(_rsStatement);
    else
        m_pDesignView->setStatement(_rsStatement);
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::copy()
{
    if(m_pTextView->IsVisible())
        m_pTextView->copy();
    else
        m_pDesignView->copy();
}
// -----------------------------------------------------------------------------
sal_Bool OQueryViewSwitch::isCutAllowed()
{
    if(m_pTextView->IsVisible())
        return m_pTextView->isCutAllowed();
    return m_pDesignView->isCutAllowed();
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::cut()
{
    if(m_pTextView->IsVisible())
        m_pTextView->cut();
    else
        m_pDesignView->cut();
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::paste()
{
    if(m_pTextView->IsVisible())
        m_pTextView->paste();
    else
        m_pDesignView->paste();
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::switchView()
{
    m_pTextView->Show(!static_cast<OQueryController*>(m_pDesignView->getController())->isDesignMode());

    ToolBox* pToolBox = m_pDesignView->getToolBox();
    if(pToolBox && m_pTextView->IsVisible())
    {
        pToolBox->SetParent(m_pTextView); // change owner ship
        m_pDesignView->Show(FALSE);
        pToolBox->HideItem(ID_BROWSER_QUERY_DISTINCT_VALUES);
        pToolBox->HideItem(ID_BROWSER_QUERY_VIEW_ALIASES);
        pToolBox->HideItem(ID_BROWSER_QUERY_VIEW_TABLES);
        pToolBox->HideItem(ID_BROWSER_QUERY_VIEW_FUNCTIONS);
        pToolBox->HideItem(ID_BROWSER_ADDTABLE);
        pToolBox->HideItem(ID_QUERY_ZOOM_IN);
        pToolBox->HideItem(ID_QUERY_ZOOM_OUT);
        pToolBox->ShowItem(ID_BROWSER_ESACPEPROCESSING);
        //  ToolBoxItemType eType = pToolBox->GetItemType(pToolBox->GetItemPos(ID_BROWSER_SQL)+1);
        //  pToolBox->HideItem(pToolBox->GetItemId(pToolBox->GetItemPos(ID_BROWSER_SQL))+1); // hide the separator
        m_pTextView->clear();
        m_pTextView->setStatement(static_cast<OQueryController*>(m_pDesignView->getController())->getStatement());
    }
    else if(pToolBox)
    {
        pToolBox->SetParent(m_pDesignView); // change owner ship
        //  pToolBox->ShowItem(pToolBox->GetItemId(pToolBox->GetItemPos(ID_BROWSER_ADDTABLE)-1)); // hide the separator
        pToolBox->HideItem(ID_BROWSER_ESACPEPROCESSING);
        pToolBox->ShowItem(ID_BROWSER_ADDTABLE);
        pToolBox->ShowItem(ID_BROWSER_QUERY_VIEW_FUNCTIONS);
        pToolBox->ShowItem(ID_BROWSER_QUERY_VIEW_TABLES);
        pToolBox->ShowItem(ID_BROWSER_QUERY_VIEW_ALIASES);
        pToolBox->ShowItem(ID_BROWSER_QUERY_DISTINCT_VALUES);
        pToolBox->ShowItem(ID_QUERY_ZOOM_IN);
        pToolBox->ShowItem(ID_QUERY_ZOOM_OUT);

        //  m_pDesignView->clear();
        getAddTableDialog()->Update();
        m_pDesignView->InitFromParseNode();
        // only show the view when the data is inserted
        m_pDesignView->Show(static_cast<OQueryController*>(m_pDesignView->getController())->isDesignMode());
    }
    m_pDesignView->Resize();
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::clearDesignView()
{
    m_pDesignView->clear();
}
// -----------------------------------------------------------------------------
OAddTableDlg* OQueryViewSwitch::getAddTableDialog()
{
    return m_pDesignView->getAddTableDialog();
}
// -----------------------------------------------------------------------------
BOOL OQueryViewSwitch::IsAddAllowed()
{
    return m_pDesignView->IsAddAllowed();
}
// -----------------------------------------------------------------------------
sal_Bool OQueryViewSwitch::isSlotEnabled(sal_Int32 _nSlotId)
{
    return m_pDesignView->isSlotEnabled(_nSlotId);
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::setSlotEnabled(sal_Int32 _nSlotId,sal_Bool _bEnable)
{
    m_pDesignView->setSlotEnabled(_nSlotId,_bEnable);
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::zoomTableView(const Fraction& _rFraction)
{
    m_pDesignView->zoomTableView(_rFraction);
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::SaveUIConfig()
{
    if(m_pDesignView->IsVisible())
        m_pDesignView->SaveUIConfig();
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::SetPosSizePixel( Point _rPt,Size _rSize)
{
    m_pDesignView->SetPosSizePixel( _rPt,_rSize);
    m_pTextView->SetPosSizePixel( _rPt,_rSize);
}
// -----------------------------------------------------------------------------
Reference< XMultiServiceFactory > OQueryViewSwitch::getORB() const
{
    return m_pDesignView->getORB();
}
// -----------------------------------------------------------------------------

