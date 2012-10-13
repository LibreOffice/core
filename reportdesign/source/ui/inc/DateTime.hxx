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
#ifndef RPTUI_DATETIME_HXX
#define RPTUI_DATETIME_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include <svtools/dialogcontrolling.hxx>

namespace rptui
{
class OReportController;
/*************************************************************************
|*
|* Groups and Sorting dialog
|*
\************************************************************************/
class ODateTimeDialog : public ModalDialog
{
    CheckBox                                m_aDate;
    FixedText                               m_aFTDateFormat;
    ListBox                                 m_aDateListBox;
    FixedLine                               m_aFL0;
    CheckBox                                m_aTime;
    FixedText                               m_aFTTimeFormat;
    ListBox                                 m_aTimeListBox;
    FixedLine                               m_aFL1;
    OKButton                                m_aPB_OK;
    CancelButton                            m_aPB_CANCEL;
    HelpButton                              m_aPB_Help;


    svt::ControlDependencyManager           m_aDateControlling;
    svt::ControlDependencyManager           m_aTimeControlling;

    ::rptui::OReportController*             m_pController;
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>
                                            m_xHoldAlive;
    ::com::sun::star::lang::Locale          m_nLocale;

    /** returns the frmat string
    *
    * \param _nNumberFormatKey the number format key
    * \param _xFormats
    * \param _bTime
    * \return
    */
    ::rtl::OUString getFormatStringByKey(::sal_Int32 _nNumberFormatKey,const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats>& _xFormats,bool _bTime);

    /** returns the number format key
        @param  _nNumberFormatIndex the number format index @see com::sun::star::i18n::NumberFormatIndex
    */
    sal_Int32 getFormatKey(sal_Bool _bDate) const;

    DECL_LINK( CBClickHdl, CheckBox* );
    ODateTimeDialog(const ODateTimeDialog&);
    void operator =(const ODateTimeDialog&);

    // fill methods
    void InsertEntry(sal_Int16 _nNumberFormatId);
public:
    ODateTimeDialog( Window* pParent
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xHoldAlive
                        ,::rptui::OReportController* _pController);
    virtual ~ODateTimeDialog();
    virtual short   Execute();
};
// =============================================================================
} // namespace rptui
// =============================================================================
#endif // RPTUI_DATETIME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
