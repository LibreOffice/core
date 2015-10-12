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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DATETIME_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DATETIME_HXX

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
    VclPtr<CheckBox>                                m_pDate;
    VclPtr<FixedText>                               m_pFTDateFormat;
    VclPtr<ListBox>                                 m_pDateListBox;
    VclPtr<CheckBox>                                m_pTime;
    VclPtr<FixedText>                               m_pFTTimeFormat;
    VclPtr<ListBox>                                 m_pTimeListBox;
    VclPtr<OKButton>                                m_pPB_OK;

    svt::ControlDependencyManager           m_aDateControlling;
    svt::ControlDependencyManager           m_aTimeControlling;

    ::rptui::OReportController*             m_pController;
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>
                                            m_xHoldAlive;
    ::com::sun::star::lang::Locale          m_nLocale;

    /** returns the format string
    *
    * \param _nNumberFormatKey the number format key
    * \param _xFormats
    * \param _bTime
    * \return
    */
    OUString getFormatStringByKey(::sal_Int32 _nNumberFormatKey,const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats>& _xFormats,bool _bTime);

    /** returns the number format key
        @param  _nNumberFormatIndex the number format index @see com::sun::star::i18n::NumberFormatIndex
    */
    sal_Int32 getFormatKey(bool _bDate) const;

    DECL_LINK_TYPED( CBClickHdl, Button*, void );
    ODateTimeDialog(const ODateTimeDialog&) = delete;
    void operator =(const ODateTimeDialog&) = delete;

    // fill methods
    void InsertEntry(sal_Int16 _nNumberFormatId);
public:
    ODateTimeDialog( vcl::Window* pParent
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xHoldAlive
                        ,::rptui::OReportController* _pController);
    virtual ~ODateTimeDialog();
    virtual void dispose() SAL_OVERRIDE;
    virtual short   Execute() SAL_OVERRIDE;
};

} // namespace rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DATETIME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
