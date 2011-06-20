/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_rptui.hxx"
#include "PageNumber.hxx"
#include "PageNumber.hrc"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <tools/debug.hxx>
#include "RptResId.hrc"
#include "rptui_slotid.hrc"
#include "ModuleHelper.hxx"
#include "RptDef.hxx"
#include "helpids.hrc"
#include <vcl/msgbox.hxx>
#include <svx/globlmn.hrc>
#include <svx/svxids.hrc>
#include "UITools.hxx"
#include "uistrings.hrc"
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
