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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_PAGENUMBER_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_PAGENUMBER_HXX

#include <vcl/weld.hxx>
#include <com/sun/star/report/XReportDefinition.hpp>

namespace rptui
{
class OReportController;
/*************************************************************************
|*
|* Groups and Sorting dialog
|*
\************************************************************************/
class OPageNumberDialog : public weld::GenericDialogController
{
    ::rptui::OReportController*                    m_pController;
    css::uno::Reference< css::report::XReportDefinition>
                                                   m_xHoldAlive;
    std::unique_ptr<weld::RadioButton> m_xPageN;
    std::unique_ptr<weld::RadioButton> m_xPageNofM;
    std::unique_ptr<weld::RadioButton> m_xTopPage;
    std::unique_ptr<weld::RadioButton> m_xBottomPage;
    std::unique_ptr<weld::ComboBox> m_xAlignmentLst;
    std::unique_ptr<weld::CheckButton> m_xShowNumberOnFirstPage;

    OPageNumberDialog(const OPageNumberDialog&) = delete;
    void operator =(const OPageNumberDialog&) = delete;
public:
    OPageNumberDialog(weld::Window* pParent,
                      css::uno::Reference< css::report::XReportDefinition> _xHoldAlive,
                      ::rptui::OReportController* _pController);
    virtual ~OPageNumberDialog() override;
    virtual short run() override;
};

} // namespace rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_PAGENUMBER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
