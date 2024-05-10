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
#include <DateTime.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <rptui_slotid.hrc>
#include <connectivity/dbconversion.hxx>
#include <unotools/syslocale.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <strings.hxx>
#include <ReportController.hxx>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatPreviewer.hpp>
#include <algorithm>

namespace rptui
{
using namespace ::com::sun::star;



ODateTimeDialog::ODateTimeDialog(weld::Window* _pParent, uno::Reference< report::XSection > _xHoldAlive,
                                 OReportController* _pController)
    : GenericDialogController(_pParent, u"modules/dbreport/ui/datetimedialog.ui"_ustr, u"DateTimeDialog"_ustr)

    , m_pController(_pController)
    , m_xHoldAlive(std::move(_xHoldAlive))
    , m_xDate(m_xBuilder->weld_check_button(u"date"_ustr))
    , m_xFTDateFormat(m_xBuilder->weld_label(u"datelistbox_label"_ustr))
    , m_xDateListBox(m_xBuilder->weld_combo_box(u"datelistbox"_ustr))
    , m_xTime(m_xBuilder->weld_check_button(u"time"_ustr))
    , m_xFTTimeFormat(m_xBuilder->weld_label(u"timelistbox_label"_ustr))
    , m_xTimeListBox(m_xBuilder->weld_combo_box(u"timelistbox"_ustr))
    , m_xPB_OK(m_xBuilder->weld_button(u"ok"_ustr))
{
    try
    {
        SvtSysLocale aSysLocale;
        m_nLocale = aSysLocale.GetLanguageTag().getLocale();
        // Fill listbox with all well known date types
        InsertEntry(util::NumberFormat::DATE);
        InsertEntry(util::NumberFormat::TIME);
    }
    catch (const uno::Exception&)
    {
    }

    m_xDateListBox->set_active(0);
    m_xTimeListBox->set_active(0);

    weld::CheckButton* aCheckBoxes[] = { m_xDate.get(), m_xTime.get() };
    for (weld::CheckButton* pCheckBox : aCheckBoxes)
        pCheckBox->connect_toggled(LINK(this,ODateTimeDialog,CBClickHdl));
    CBClickHdl(*m_xTime);
}

void ODateTimeDialog::InsertEntry(sal_Int16 _nNumberFormatId)
{
    const bool bTime = util::NumberFormat::TIME == _nNumberFormatId;
    weld::ComboBox* pListBox = m_xDateListBox.get();
    if (bTime)
        pListBox = m_xTimeListBox.get();

    const uno::Reference< util::XNumberFormatter> xNumberFormatter = m_pController->getReportNumberFormatter();
    const uno::Reference< util::XNumberFormats> xFormats = xNumberFormatter->getNumberFormatsSupplier()->getNumberFormats();
    const uno::Sequence<sal_Int32> aFormatKeys = xFormats->queryKeys(_nNumberFormatId,m_nLocale,true);
    for (const sal_Int32 nFormatKey : aFormatKeys)
    {
        pListBox->append(OUString::number(nFormatKey), getFormatStringByKey(nFormatKey,xFormats,bTime));
    }
}

short ODateTimeDialog::run()
{
    short nRet = GenericDialogController::run();
    if (nRet == RET_OK && (m_xDate->get_active() || m_xTime->get_active()))
    {
        try
        {
            sal_Int32 nLength = 0;
            uno::Sequence<beans::PropertyValue> aValues( 6 );
            auto pValues = aValues.getArray();
            pValues[nLength].Name = PROPERTY_SECTION;
            pValues[nLength++].Value <<= m_xHoldAlive;

            pValues[nLength].Name = PROPERTY_TIME_STATE;
            pValues[nLength++].Value <<= m_xTime->get_active();

            pValues[nLength].Name = PROPERTY_DATE_STATE;
            pValues[nLength++].Value <<= m_xDate->get_active();

            pValues[nLength].Name = PROPERTY_FORMATKEYDATE;
            pValues[nLength++].Value <<= getFormatKey(true);

            pValues[nLength].Name = PROPERTY_FORMATKEYTIME;
            pValues[nLength++].Value <<= getFormatKey(false);

            OutputDevice* pDefDev = Application::GetDefaultDevice();
            sal_Int32 nWidth = 0;
            if ( m_xDate->get_active() )
            {
                OUString sDateFormat = m_xDateListBox->get_active_text();
                nWidth = OutputDevice::LogicToLogic(pDefDev->PixelToLogic(Size(pDefDev->GetCtrlTextWidth(sDateFormat),0)).Width(),
                        pDefDev->GetMapMode().GetMapUnit(),MapUnit::Map100thMM);
            }
            if ( m_xTime->get_active() )
            {
                OUString sDateFormat = m_xTimeListBox->get_active_text();
                nWidth = ::std::max<sal_Int32>(OutputDevice::LogicToLogic(pDefDev->PixelToLogic(Size(pDefDev->GetCtrlTextWidth(sDateFormat),0)).Width(),
                        pDefDev->GetMapMode().GetMapUnit(),MapUnit::Map100thMM),nWidth);
            }

            if ( nWidth > 4000 )
            {
                pValues[nLength].Name = PROPERTY_WIDTH;
                pValues[nLength++].Value <<= nWidth;
            }

            m_pController->executeChecked(SID_DATETIME,aValues);
        }
        catch (const uno::Exception&)
        {
            nRet = RET_NO;
        }
    }
    return nRet;
}

OUString ODateTimeDialog::getFormatStringByKey(::sal_Int32 _nNumberFormatKey,const uno::Reference< util::XNumberFormats>& _xFormats,bool _bTime)
{
    uno::Reference< beans::XPropertySet> xFormSet = _xFormats->getByKey(_nNumberFormatKey);
    OSL_ENSURE(xFormSet.is(),"XPropertySet is null!");
    OUString sFormat;
    xFormSet->getPropertyValue(u"FormatString"_ustr) >>= sFormat;

    double nValue = 0;
    if ( _bTime )
    {
        tools::Time aCurrentTime( tools::Time::SYSTEM );
        nValue = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toTime(aCurrentTime.GetTime()));
    }
    else
    {
        Date aCurrentDate( Date::SYSTEM );
        static css::util::Date STANDARD_DB_DATE(30,12,1899);
        nValue = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toDate(aCurrentDate.GetDate()),STANDARD_DB_DATE);
    }

    uno::Reference< util::XNumberFormatPreviewer> xPreviewer(m_pController->getReportNumberFormatter(),uno::UNO_QUERY);
    OSL_ENSURE(xPreviewer.is(),"XNumberFormatPreviewer is null!");
    return xPreviewer->convertNumberToPreviewString(sFormat,nValue,m_nLocale,true);
}

IMPL_LINK_NOARG(ODateTimeDialog, CBClickHdl, weld::Toggleable&, void)
{
    const bool bDate = m_xDate->get_active();
    m_xFTDateFormat->set_sensitive(bDate);
    m_xDateListBox->set_sensitive(bDate);

    const bool bTime = m_xTime->get_active();
    m_xFTTimeFormat->set_sensitive(bTime);
    m_xTimeListBox->set_sensitive(bTime);

    if (!bDate && !bTime)
    {
        m_xPB_OK->set_sensitive(false);
    }
    else
    {
        m_xPB_OK->set_sensitive(true);
    }
}

sal_Int32 ODateTimeDialog::getFormatKey(bool _bDate) const
{
    sal_Int32 nFormatKey;
    if ( _bDate )
    {
         nFormatKey = m_xDateListBox->get_active_id().toInt32();
    }
    else
    {
         nFormatKey = m_xTimeListBox->get_active_id().toInt32();
    }
    return nFormatKey;
}

} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
