/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: RptModel.cxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "RptModel.hxx"
#include "RptPage.hxx"
#include <dbaccess/singledoccontroller.hxx>
#include <tools/debug.hxx>
#include <svtools/pathoptions.hxx>

#include "UndoActions.hxx"
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
    SdrModel(SvtPathOptions().GetPalettePath(),NULL,_pReportDefinition)
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
