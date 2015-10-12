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
    VclPtr<RadioButton>                            m_pPageN;
    VclPtr<RadioButton>                            m_pPageNofM;

    VclPtr<RadioButton>                            m_pTopPage;
    VclPtr<RadioButton>                            m_pBottomPage;
    VclPtr<ListBox>                                m_pAlignmentLst;

    VclPtr<CheckBox>                               m_pShowNumberOnFirstPage;

    ::rptui::OReportController*             m_pController;
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition>
                                            m_xHoldAlive;

    OPageNumberDialog(const OPageNumberDialog&) = delete;
    void operator =(const OPageNumberDialog&) = delete;
public:
    OPageNumberDialog( vcl::Window* pParent
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition>& _xHoldAlive
                        ,::rptui::OReportController* _pController);
    virtual ~OPageNumberDialog();
    virtual void    dispose() SAL_OVERRIDE;
    virtual short   Execute() SAL_OVERRIDE;
};

} // namespace rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_PAGENUMBER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
