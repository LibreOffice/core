/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RptModel.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:58:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#ifndef REPORT_RPTMODEL_HXX
#include "RptModel.hxx"
#endif

#ifndef _REPORT_RPTUIPAGE_HXX
#include "RptPage.hxx"
#endif
#ifndef DBAUI_SINGLEDOCCONTROLLER_HXX
#include <dbaccess/singledoccontroller.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef RPTUI_UNDOACTIONS_HXX
#include "UndoActions.hxx"
#endif
#include "UndoEnv.hxx"
#include "ReportUndoFactory.hxx"
#include "ReportDefinition.hxx"
#define ITEMID_COLOR        1
#define ITEMID_BRUSH        2
#define ITEMID_FONT         3
#define ITEMID_FONTHEIGHT   4

#include <svx/tbcontrl.hxx>
#include "rptui_slotid.hrc"
#include "RptDef.hxx"
#include "corestrings.hrc"

namespace rptui
{

using namespace com::sun::star;
DBG_NAME( rpt_OReportModel )
TYPEINIT1(OReportModel,SdrModel);

//----------------------------------------------------------------------------

OReportModel::OReportModel(::reportdesign::OReportDefinition* _pReportDefinition) :
    SdrModel(NULL,_pReportDefinition)
    ,m_pController(NULL)
    ,m_pReportDefinition(_pReportDefinition)
{
    DBG_CTOR( rpt_OReportModel,0);
    SetAllowShapePropertyChangeListener(true);
    m_pUndoEnv = new OXUndoEnvironment(*this);
    m_pUndoEnv->acquire();
    SetSdrUndoFactory(new OReportUndoFactory);

 //   SvxFontNameToolBoxControl::RegisterControl(SID_ATTR_CHAR_FONT);
    //SvxFontHeightToolBoxControl::RegisterControl(SID_ATTR_CHAR_FONTHEIGHT);
    //SvxFontColorToolBoxControl::RegisterControl(SID_ATTR_CHAR_COLOR);
    //SvxFontColorExtToolBoxControl::RegisterControl(SID_ATTR_CHAR_COLOR2);
    //SvxFontColorExtToolBoxControl::RegisterControl(SID_ATTR_CHAR_COLOR_BACKGROUND);
    //SvxColorToolBoxControl::RegisterControl(SID_BACKGROUND_COLOR);
}

//----------------------------------------------------------------------------
OReportModel::~OReportModel()
{
    DBG_DTOR( rpt_OReportModel,0);
    detachController();
    m_pUndoEnv->release();
}
// -----------------------------------------------------------------------------
void OReportModel::detachController()
{
    m_pReportDefinition = NULL;
    m_pController = NULL;
    m_pUndoEnv->EndListening( *this );
    ClearUndoBuffer();
    m_pUndoEnv->Clear(OXUndoEnvironment::Accessor());
}
//----------------------------------------------------------------------------
SdrPage* OReportModel::AllocPage(FASTBOOL /*bMasterPage*/)
{
    DBG_CHKTHIS( rpt_OReportModel, 0);
    OSL_ENSURE(0,"Who called me!");
    return NULL;
}

//----------------------------------------------------------------------------

void OReportModel::SetChanged( sal_Bool bChanged )
{
    SdrModel::SetChanged( bChanged );
    SetModified( bChanged );
}

//----------------------------------------------------------------------------

Window* OReportModel::GetCurDocViewWin()
{
    return 0;
}

//----------------------------------------------------------------------------
OXUndoEnvironment&  OReportModel::GetUndoEnv()
{
    return *m_pUndoEnv;
}
//----------------------------------------------------------------------------
void OReportModel::SetModified(sal_Bool _bModified)
{
    if ( m_pController )
        m_pController->setModified(_bModified);
}
// -----------------------------------------------------------------------------
SdrPage* OReportModel::RemovePage(USHORT nPgNum)
{
    OReportPage* pPage = dynamic_cast<OReportPage*>(SdrModel::RemovePage(nPgNum));
    //if ( pPage )
    //{
    //    m_pUndoEnv->RemoveSection(pPage);
    //}
    return pPage;
}
// -----------------------------------------------------------------------------
OReportPage* OReportModel::createNewPage(const uno::Reference< report::XSection >& _xSection)
{
    OReportPage* pPage = new OReportPage( *this ,_xSection);
    InsertPage(pPage);
    m_pUndoEnv->AddSection(_xSection);
    return pPage;
}
// -----------------------------------------------------------------------------
OReportPage* OReportModel::getPage(const uno::Reference< report::XSection >& _xSection)
{
    OReportPage* pPage = NULL;
    USHORT nCount = GetPageCount();
    for (USHORT i = 0; i < nCount && !pPage ; ++i)
    {
        OReportPage* pRptPage = PTR_CAST( OReportPage, GetPage(i) );
        if ( pRptPage && pRptPage->getSection() == _xSection )
            pPage = pRptPage;
    }
    return pPage;
}
// -----------------------------------------------------------------------------
SvxNumType OReportModel::GetPageNumType() const
{
    uno::Reference< report::XReportDefinition > xReportDefinition( getReportDefinition() );
    if ( xReportDefinition.is() )
        return (SvxNumType)getStyleProperty<sal_Int16>(xReportDefinition,reportdesign::PROPERTY_NUMBERINGTYPE);
    return SVX_ARABIC;
}

// -----------------------------------------------------------------------------
uno::Reference< report::XReportDefinition > OReportModel::getReportDefinition() const
{
    uno::Reference< report::XReportDefinition > xReportDefinition = m_pReportDefinition;
    OSL_ENSURE( xReportDefinition.is(), "OReportModel::getReportDefinition: invalid model at our controller!" );
    return xReportDefinition;
}
// -----------------------------------------------------------------------------
uno::Reference< uno::XInterface > OReportModel::createUnoModel()
{
    return uno::Reference< uno::XInterface >(getReportDefinition(),uno::UNO_QUERY);
}
//==================================================================
}   //rptui
//==================================================================
