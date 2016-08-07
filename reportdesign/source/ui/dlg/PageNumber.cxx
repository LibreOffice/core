/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#include "precompiled_rptui.hxx"
#include "PageNumber.hxx"
#ifndef RPTUI_PAGENUMBER_HRC
#include "PageNumber.hrc"
#endif
#include <com/sun/star/beans/XPropertySet.hpp>
#include <tools/debug.hxx>
#ifndef _RPTUI_DLGRESID_HRC
#include "RptResId.hrc"
#endif
#ifndef _RPTUI_SLOTID_HRC_
#include "rptui_slotid.hrc"
#endif
#ifndef _RPTUI_MODULE_HELPER_DBU_HXX_
#include "ModuleHelper.hxx"
#endif
#include "RptDef.hxx"
#ifndef RTPUI_REPORTDESIGN_HELPID_HRC
#include "helpids.hrc"
#endif
#include <vcl/msgbox.hxx>
#ifndef _GLOBLMN_HRC
#include <svx/globlmn.hrc>
#endif
#ifndef _SBASLTID_HRC
#include <svx/svxids.hrc>
#endif
#ifndef RPTUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef REPORTDESIGN_SHARED_UISTRINGS_HRC
#include "uistrings.hrc"
#endif
#include "ReportController.hxx"
#include <com/sun/star/report/XFixedText.hpp>
#include <algorithm>

namespace rptui
{
using namespace ::com::sun::star;
using namespace ::comphelper;

DBG_NAME( rpt_OPageNumberDialog )
//========================================================================
// class OPageNumberDialog
//========================================================================
OPageNumberDialog::OPageNumberDialog( Window* _pParent
                                           ,const uno::Reference< report::XReportDefinition >& _xHoldAlive
                                           ,OReportController* _pController)
    : ModalDialog( _pParent, ModuleRes(RID_PAGENUMBERS) )
    ,m_aFormat(this, ModuleRes(FL_FORMAT) )
    ,m_aPageN(this, ModuleRes(RB_PAGE_N) )
    ,m_aPageNofM(this, ModuleRes(RB_PAGE_N_OF_M) )
    ,m_aPosition(this, ModuleRes(FL_POSITION) )
    ,m_aTopPage(this, ModuleRes(RB_PAGE_TOPPAGE) )
    ,m_aBottomPage(this, ModuleRes(RB_PAGE_BOTTOMPAGE) )
    ,m_aMisc(this, ModuleRes(FL_MISC) )
    ,m_aAlignment(this, ModuleRes(FL_ALIGNMENT) )
    ,m_aAlignmentLst(this, ModuleRes(LST_ALIGNMENT) )
    ,m_aShowNumberOnFirstPage(this, ModuleRes(CB_SHOWNUMBERONFIRSTPAGE) )
    ,m_aFl1(this,       ModuleRes(FL_SEPARATOR1))
    ,m_aPB_OK(this,     ModuleRes(PB_OK))
    ,m_aPB_CANCEL(this, ModuleRes(PB_CANCEL))
    ,m_aPB_Help(this,   ModuleRes(PB_HELP))
    ,m_pController(_pController)
    ,m_xHoldAlive(_xHoldAlive)
{
    DBG_CTOR( rpt_OPageNumberDialog,NULL);

    m_aShowNumberOnFirstPage.Hide();

    FreeResource();
}

//------------------------------------------------------------------------
OPageNumberDialog::~OPageNumberDialog()
{
    DBG_DTOR( rpt_OPageNumberDialog,NULL);
}
// -----------------------------------------------------------------------------
short OPageNumberDialog::Execute()
{
    short nRet = ModalDialog::Execute();
    if ( nRet == RET_OK )
    {
        try
        {
            sal_Int32 nControlMaxSize = 3000;
            sal_Int32 nPosX = 0;
            sal_Int32 nPos2X = 0;
            awt::Size aRptSize = getStyleProperty<awt::Size>(m_xHoldAlive,PROPERTY_PAPERSIZE);
            switch ( m_aAlignmentLst.GetSelectEntryPos() )
            {
                case 0: // left
                    nPosX = getStyleProperty<sal_Int32>(m_xHoldAlive,PROPERTY_LEFTMARGIN);
                    break;
                case 1: // middle
                    nPosX = getStyleProperty<sal_Int32>(m_xHoldAlive,PROPERTY_LEFTMARGIN) + (aRptSize.Width - getStyleProperty<sal_Int32>(m_xHoldAlive,PROPERTY_LEFTMARGIN) - getStyleProperty<sal_Int32>(m_xHoldAlive,PROPERTY_RIGHTMARGIN) - nControlMaxSize) / 2;
                    break;
                case 2: // right
                    nPosX = (aRptSize.Width - getStyleProperty<sal_Int32>(m_xHoldAlive,PROPERTY_RIGHTMARGIN) - nControlMaxSize);
                    break;
                case 3: // inner
                case 4: // outer
                    nPosX = getStyleProperty<sal_Int32>(m_xHoldAlive,PROPERTY_LEFTMARGIN);
                    nPos2X = (aRptSize.Width - getStyleProperty<sal_Int32>(m_xHoldAlive,PROPERTY_RIGHTMARGIN) - nControlMaxSize);
                    break;
                default:
                    break;
            }
            if ( m_aAlignmentLst.GetSelectEntryPos() > 2 )
                nPosX = nPos2X;

            sal_Int32 nLength = 0;
            uno::Sequence<beans::PropertyValue> aValues( 3 );
            aValues[nLength].Name = PROPERTY_POSITION;
            aValues[nLength++].Value <<= awt::Point(nPosX,0);

            aValues[nLength].Name = PROPERTY_PAGEHEADERON;
            aValues[nLength++].Value <<= m_aTopPage.IsChecked();

            aValues[nLength].Name = PROPERTY_STATE;
            aValues[nLength++].Value <<= m_aPageNofM.IsChecked();

            m_pController->executeChecked(SID_INSERT_FLD_PGNUMBER,aValues);
        }
        catch(uno::Exception&)
        {
            nRet = RET_NO;
        }
    }
    return nRet;
}
// =============================================================================
} // rptui
// =============================================================================
