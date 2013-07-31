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
#include "PageNumber.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <tools/debug.hxx>
#include "RptResId.hrc"
#include "rptui_slotid.hrc"
#include "ModuleHelper.hxx"
#include "RptDef.hxx"
#include "helpids.hrc"
#include <vcl/msgbox.hxx>
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
    : ModalDialog( _pParent, "PageNumberDialog" , "modules/dbreport/ui/pagenumberdialog.ui" )
    ,m_pController(_pController)
    ,m_xHoldAlive(_xHoldAlive)
{
    get(m_pPageN,"pagen");
    get(m_pPageNofM,"pagenofm");
    get(m_pTopPage,"toppage");
    get(m_pBottomPage,"bottompage");
    get(m_pAlignmentLst,"alignment");
    get(m_pShowNumberOnFirstPage,"shownumberonfirstpage");

    DBG_CTOR( rpt_OPageNumberDialog,NULL);

    m_pShowNumberOnFirstPage->Hide();

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
            switch ( m_pAlignmentLst->GetSelectEntryPos() )
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
            if ( m_pAlignmentLst->GetSelectEntryPos() > 2 )
                nPosX = nPos2X;

            sal_Int32 nLength = 0;
            uno::Sequence<beans::PropertyValue> aValues( 3 );
            aValues[nLength].Name = PROPERTY_POSITION;
            aValues[nLength++].Value <<= awt::Point(nPosX,0);

            aValues[nLength].Name = PROPERTY_PAGEHEADERON;
            aValues[nLength++].Value <<= m_pTopPage->IsChecked();

            aValues[nLength].Name = PROPERTY_STATE;
            aValues[nLength++].Value <<= m_pPageNofM->IsChecked();

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
