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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <rptui_slotid.hrc>
#include <RptDef.hxx>
#include <vcl/settings.hxx>

#include <UITools.hxx>
#include <strings.hxx>
#include <ReportController.hxx>
#include <comphelper/propertysequence.hxx>
#include <algorithm>

namespace rptui
{
using namespace ::com::sun::star;
using namespace ::comphelper;


// class OPageNumberDialog

OPageNumberDialog::OPageNumberDialog(weld::Window* pParent,
                                     const uno::Reference< report::XReportDefinition >& _xHoldAlive,
                                     OReportController* )
    : GenericDialogController(pParent, "modules/dbreport/ui/pagenumberdialog.ui", "PageNumberDialog")
    , m_xHoldAlive(_xHoldAlive)
    , m_xPageN(m_xBuilder->weld_radio_button("pagen"))
    , m_xPageNofM(m_xBuilder->weld_radio_button("pagenofm"))
    , m_xTopPage(m_xBuilder->weld_radio_button("toppage"))
    , m_xBottomPage(m_xBuilder->weld_radio_button("bottompage"))
    , m_xAlignmentLst(m_xBuilder->weld_combo_box("alignment"))
    , m_xShowNumberOnFirstPage(m_xBuilder->weld_check_button("shownumberonfirstpage"))
{
    m_xShowNumberOnFirstPage->hide();
}

OPageNumberDialog::~OPageNumberDialog()
{
}

} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
