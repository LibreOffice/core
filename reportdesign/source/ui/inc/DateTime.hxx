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

#include <vcl/weld.hxx>
#include <com/sun/star/report/XSection.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <com/sun/star/lang/Locale.hpp>

namespace rptui
{
class OReportController;
/*************************************************************************
|*
|* Groups and Sorting dialog
|*
\************************************************************************/
class ODateTimeDialog : public weld::GenericDialogController
{
    ::rptui::OReportController*             m_pController;
    css::uno::Reference< css::report::XSection>
                                            m_xHoldAlive;
    css::lang::Locale                       m_nLocale;

    std::unique_ptr<weld::CheckButton> m_xDate;
    std::unique_ptr<weld::Label> m_xFTDateFormat;
    std::unique_ptr<weld::ComboBox> m_xDateListBox;
    std::unique_ptr<weld::CheckButton> m_xTime;
    std::unique_ptr<weld::Label> m_xFTTimeFormat;
    std::unique_ptr<weld::ComboBox> m_xTimeListBox;
    std::unique_ptr<weld::Button> m_xPB_OK;

    /** returns the format string
    *
    * \param _nNumberFormatKey the number format key
    * \param _xFormats
    * \param _bTime
    * \return
    */
    OUString getFormatStringByKey(::sal_Int32 _nNumberFormatKey,const css::uno::Reference< css::util::XNumberFormats>& _xFormats,bool _bTime);

    /** returns the number format key
        @param  _nNumberFormatIndex the number format index @see css::i18n::NumberFormatIndex
    */
    sal_Int32 getFormatKey(bool _bDate) const;

    DECL_LINK(CBClickHdl, weld::ToggleButton&, void);
    ODateTimeDialog(const ODateTimeDialog&) = delete;
    void operator =(const ODateTimeDialog&) = delete;

    // fill methods
    void InsertEntry(sal_Int16 _nNumberFormatId);
public:
    ODateTimeDialog(weld::Window* pParent,
                     const css::uno::Reference< css::report::XSection>& _xHoldAlive,
                     ::rptui::OReportController* _pController);
    virtual short run() override;
};

} // namespace rptui

#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DATETIME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
