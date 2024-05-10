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
#include <PageNumber.hxx>
#include <rptui_slotid.hrc>
#include <RptDef.hxx>

#include <strings.hxx>
#include <ReportController.hxx>
#include <comphelper/propertysequence.hxx>
#include <utility>

namespace rptui
{
using namespace ::com::sun::star;
using namespace ::comphelper;



OPageNumberDialog::OPageNumberDialog(weld::Window* pParent,
                                     uno::Reference< report::XReportDefinition > _xHoldAlive,
                                     OReportController* _pController)
    : GenericDialogController(pParent, u"modules/dbreport/ui/pagenumberdialog.ui"_ustr, u"PageNumberDialog"_ustr)
    , m_pController(_pController)
    , m_xHoldAlive(std::move(_xHoldAlive))
    , m_xPageNofM(m_xBuilder->weld_radio_button(u"pagenofm"_ustr))
    , m_xTopPage(m_xBuilder->weld_radio_button(u"toppage"_ustr))
    , m_xAlignmentLst(m_xBuilder->weld_combo_box(u"alignment"_ustr))
    , m_xShowNumberOnFirstPage(m_xBuilder->weld_check_button(u"shownumberonfirstpage"_ustr))
{
    m_xShowNumberOnFirstPage->hide();
}

OPageNumberDialog::~OPageNumberDialog()
{
}

short OPageNumberDialog::run()
{
    short nRet = GenericDialogController::run();
    if (nRet == RET_OK)
    {
        try
        {
            sal_Int32 nControlMaxSize = 3000;
            sal_Int32 nPosX = 0;
            sal_Int32 nPos2X = 0;
            awt::Size aRptSize = getStyleProperty<awt::Size>(m_xHoldAlive,PROPERTY_PAPERSIZE);
            switch (m_xAlignmentLst->get_active())
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
            if (m_xAlignmentLst->get_active() > 2)
                nPosX = nPos2X;

            uno::Sequence<beans::PropertyValue> aValues( comphelper::InitPropertySequence({
                    { PROPERTY_POSITION, uno::Any(awt::Point(nPosX,0)) },
                    { PROPERTY_PAGEHEADERON, uno::Any(m_xTopPage->get_active()) },
                    { PROPERTY_STATE, uno::Any(m_xPageNofM->get_active()) }
                }));

            m_pController->executeChecked(SID_INSERT_FLD_PGNUMBER,aValues);
        }
        catch(uno::Exception&)
        {
        }
    }
    return nRet;
}

} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
