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
#include "DateTime.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include "strings.hrc"
#include "rptui_slotid.hrc"
#include "helpids.hrc"
#include <vcl/msgbox.hxx>
#include <connectivity/dbconversion.hxx>
#include <unotools/syslocale.hxx>
#include "UITools.hxx"
#include "RptDef.hxx"
#include "strings.hxx"
#include "ReportController.hxx"
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatPreviewer.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/i18n/NumberFormatIndex.hpp>
#include <comphelper/numbers.hxx>
#include <algorithm>

namespace rptui
{
using namespace ::com::sun::star;
using namespace ::comphelper;


// class ODateTimeDialog

ODateTimeDialog::ODateTimeDialog( vcl::Window* _pParent
                                           ,const uno::Reference< report::XSection >& _xHoldAlive
                                           ,OReportController* _pController)
    : ModalDialog( _pParent, "DateTimeDialog" , "modules/dbreport/ui/datetimedialog.ui" )

    , m_aDateControlling()
    , m_aTimeControlling()
    , m_pController(_pController)
    , m_xHoldAlive(_xHoldAlive)
{
    get(m_pDate,"date");
    get(m_pFTDateFormat,"datelistbox_label");
    get(m_pDateListBox,"datelistbox");
    get(m_pTime,"time");
    get(m_pFTTimeFormat,"timelistbox_label");
    get(m_pTimeListBox,"timelistbox");
    get(m_pPB_OK,"ok");


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

    m_pDateListBox->SelectEntryPos(0);

    m_pTimeListBox->SelectEntryPos(0);

    // use nice enhancement, to toggle enable/disable if a checkbox is checked or not
    m_aDateControlling.enableOnCheckMark( *m_pDate, *m_pFTDateFormat, *m_pDateListBox);
    m_aTimeControlling.enableOnCheckMark( *m_pTime, *m_pFTTimeFormat, *m_pTimeListBox);

    CheckBox* aCheckBoxes[] = { m_pDate,m_pTime};
    for (CheckBox* pCheckBox : aCheckBoxes)
        pCheckBox->SetClickHdl(LINK(this,ODateTimeDialog,CBClickHdl));

}

    void ODateTimeDialog::InsertEntry(sal_Int16 _nNumberFormatId)
    {
        const bool bTime = util::NumberFormat::TIME == _nNumberFormatId;
        ListBox* pListBox = m_pDateListBox;
        if ( bTime )
            pListBox = m_pTimeListBox;

        const uno::Reference< util::XNumberFormatter> xNumberFormatter = m_pController->getReportNumberFormatter();
        const uno::Reference< util::XNumberFormats> xFormats = xNumberFormatter->getNumberFormatsSupplier()->getNumberFormats();
        const uno::Sequence<sal_Int32> aFormatKeys = xFormats->queryKeys(_nNumberFormatId,m_nLocale,true);
        const sal_Int32* pIter = aFormatKeys.getConstArray();
        const sal_Int32* pEnd  = pIter + aFormatKeys.getLength();
        for(;pIter != pEnd;++pIter)
        {
            const sal_Int16 nPos = pListBox->InsertEntry(getFormatStringByKey(*pIter,xFormats,bTime));
            pListBox->SetEntryData(nPos, reinterpret_cast<void*>(*pIter));
        }
    }

ODateTimeDialog::~ODateTimeDialog()
{
    disposeOnce();
}

void ODateTimeDialog::dispose()
{
    m_pDate.clear();
    m_pFTDateFormat.clear();
    m_pDateListBox.clear();
    m_pTime.clear();
    m_pFTTimeFormat.clear();
    m_pTimeListBox.clear();
    m_pPB_OK.clear();
    ModalDialog::dispose();
}

short ODateTimeDialog::Execute()
{
    short nRet = ModalDialog::Execute();
    if ( nRet == RET_OK && (m_pDate->IsChecked() || m_pTime->IsChecked()) )
    {
        try
        {
            sal_Int32 nLength = 0;
            uno::Sequence<beans::PropertyValue> aValues( 6 );
            aValues[nLength].Name = PROPERTY_SECTION;
            aValues[nLength++].Value <<= m_xHoldAlive;

            aValues[nLength].Name = PROPERTY_TIME_STATE;
            aValues[nLength++].Value <<= m_pTime->IsChecked();

            aValues[nLength].Name = PROPERTY_DATE_STATE;
            aValues[nLength++].Value <<= m_pDate->IsChecked();

            aValues[nLength].Name = PROPERTY_FORMATKEYDATE;
            aValues[nLength++].Value <<= getFormatKey(true);

            aValues[nLength].Name = PROPERTY_FORMATKEYTIME;
            aValues[nLength++].Value <<= getFormatKey(false);

            sal_Int32 nWidth = 0;
            if ( m_pDate->IsChecked() )
            {
                OUString sDateFormat = m_pDateListBox->GetSelectEntry();
                nWidth = LogicToLogic(PixelToLogic(Size(GetCtrlTextWidth(sDateFormat),0)).Width(),GetMapMode().GetMapUnit(),MapUnit::Map100thMM);
            }
            if ( m_pTime->IsChecked() )
            {
                OUString sDateFormat = m_pTimeListBox->GetSelectEntry();
                nWidth = ::std::max<sal_Int32>(LogicToLogic(PixelToLogic(Size(GetCtrlTextWidth(sDateFormat),0)).Width(),GetMapMode().GetMapUnit(),MapUnit::Map100thMM),nWidth);
            }

            if ( nWidth > 4000 )
            {
                aValues[nLength].Name = PROPERTY_WIDTH;
                aValues[nLength++].Value <<= nWidth;
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
    xFormSet->getPropertyValue("FormatString") >>= sFormat;

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

IMPL_LINK( ODateTimeDialog, CBClickHdl, Button*, _pBox, void )
{
    if ( _pBox == m_pDate || _pBox == m_pTime)
    {
        bool bDate = m_pDate->IsChecked();
        bool bTime = m_pTime->IsChecked();
        if (!bDate && !bTime)
        {
            m_pPB_OK->Disable();
        }
        else
        {
            m_pPB_OK->Enable();
        }
    }
}

sal_Int32 ODateTimeDialog::getFormatKey(bool _bDate) const
{
    sal_Int32 nFormatKey;
    if ( _bDate )
    {
         nFormatKey = static_cast<sal_Int32>(reinterpret_cast<sal_IntPtr>(m_pDateListBox->GetSelectEntryData()));
    }
    else
    {
         nFormatKey = static_cast<sal_Int32>(reinterpret_cast<sal_IntPtr>(m_pTimeListBox->GetSelectEntryData()));
    }
    return nFormatKey;
}

} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
