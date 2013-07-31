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
#ifndef RPTUI_PAGENUMBER_HXX
#define RPTUI_PAGENUMBER_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <com/sun/star/report/XReportDefinition.hpp>


namespace rptui
{
class OReportController;
/*************************************************************************
|*
|* Groups and Sorting dialog
|*
\************************************************************************/
class OPageNumberDialog :   public ModalDialog
{
    RadioButton*                            m_pPageN;
    RadioButton*                            m_pPageNofM;

    RadioButton*                            m_pTopPage;
    RadioButton*                            m_pBottomPage;
    ListBox*                                m_pAlignmentLst;

    CheckBox*                               m_pShowNumberOnFirstPage;
    OKButton*                               m_pPB_OK;
    CancelButton*                           m_pPB_CANCEL;
    HelpButton*                             m_pPB_Help;

    ::rptui::OReportController*             m_pController;
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition>
                                            m_xHoldAlive;

    OPageNumberDialog(const OPageNumberDialog&);
    void operator =(const OPageNumberDialog&);
public:
    OPageNumberDialog( Window* pParent
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition>& _xHoldAlive
                        ,::rptui::OReportController* _pController);
    virtual ~OPageNumberDialog();
    virtual short   Execute();
};
// =============================================================================
} // namespace rptui
// =============================================================================
#endif // RPTUI_PAGENUMBER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
