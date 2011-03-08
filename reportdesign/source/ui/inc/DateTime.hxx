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
    // FixedLine                            m_aFLDate;
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
