/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DateTime.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-02 14:36:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "precompiled_reportdesign.hxx"

#ifndef RPTUI_DATETIME_HXX
#include "DateTime.hxx"
#endif
#ifndef RPTUI_DATETIME_HRC
#include "DateTime.hrc"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _RPTUI_DLGRESID_HRC
#include "RptResId.hrc"
#endif
#ifndef _RPTUI_SLOTID_HRC_
#include "rptui_slotid.hrc"
#endif
#ifndef _RPTUI_MODULE_HELPER_DBU_HXX_
#include "ModuleHelper.hxx"
#endif
#ifndef RTPUI_REPORTDESIGN_HELPID_HRC
#include "helpids.hrc"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _GLOBLMN_HRC
#include <svx/globlmn.hrc>
#endif
#ifndef _SBASLTID_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef RPTUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _REPORT_RPTUIDEF_HXX
#include "RptDef.hxx"
#endif
#ifndef REPORTDESIGN_SHARED_UISTRINGS_HRC
#include "uistrings.hrc"
#endif
#ifndef RPTUI_REPORTCONTROLLER_HXX
#include "ReportController.hxx"
#endif
#ifndef _COM_SUN_STAR_REPORT_XFORMATTEDFIELD_HPP_
#include <com/sun/star/report/XFormattedField.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATPREVIEWER_HPP_
#include <com/sun/star/util/XNumberFormatPreviewer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_NUMBERFORMATINDEX_HPP_
#include <com/sun/star/i18n/NumberFormatIndex.hpp>
#endif
#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif
#include <algorithm>

namespace rptui
{
using namespace ::com::sun::star;
using namespace ::comphelper;

DBG_NAME( rpt_ODateTimeDialog )
//========================================================================
// class ODateTimeDialog
//========================================================================
ODateTimeDialog::ODateTimeDialog( Window* _pParent
                                           ,const uno::Reference< report::XSection >& _xHoldAlive
                                           ,OReportController* _pController)
    : ModalDialog( _pParent, ModuleRes(RID_DATETIME_DLG) )
    ,m_aFLDate(this, ModuleRes(FL_DATE      ) )
    ,m_aDate(this,   ModuleRes(CB_DATE      ) )
    ,m_aDateF1(this, ModuleRes(RB_DATE_F1   ) )
    ,m_aDateF2(this, ModuleRes(RB_DATE_F2   ) )
    ,m_aDateF3(this, ModuleRes(RB_DATE_F3   ) )
    ,m_aFLTime(this, ModuleRes(FL_TIME      ) )
    ,m_aTime(this,   ModuleRes(CB_TIME      ) )
    ,m_aTimeF1(this, ModuleRes(RB_TIME_F1   ) )
    ,m_aTimeF2(this, ModuleRes(RB_TIME_F2   ) )
    ,m_aTimeF3(this, ModuleRes(RB_TIME_F3   ) )
    ,m_aFL1(this,ModuleRes(FL_SEPARATOR1) )
    ,m_aPB_OK(this,     ModuleRes(PB_OK))
    ,m_aPB_CANCEL(this, ModuleRes(PB_CANCEL))
    ,m_aPB_Help(this,   ModuleRes(PB_HELP))
    ,m_pController(_pController)
    ,m_xHoldAlive(_xHoldAlive)
{
    DBG_CTOR( rpt_ODateTimeDialog,NULL);

    try
    {
        SvtSysLocale aSysLocale;
        m_nLocale = aSysLocale.GetLocaleData().getLocale();
    }
    catch(uno::Exception&)
    {
    }
    m_aDateF1.SetText(getFormatString(i18n::NumberFormatIndex::DATE_SYSTEM_LONG));
    m_aDateF2.SetText(getFormatString(i18n::NumberFormatIndex::DATE_SYS_DMMMYYYY));
    m_aDateF3.SetText(getFormatString(i18n::NumberFormatIndex::DATE_SYSTEM_SHORT));

    m_aTimeF1.SetText(getFormatString(i18n::NumberFormatIndex::TIME_HHMMSS));
    m_aTimeF2.SetText(getFormatString(i18n::NumberFormatIndex::TIME_HHMMSSAMPM));
    m_aTimeF3.SetText(getFormatString(i18n::NumberFormatIndex::TIME_HHMM));

    CheckBox* pCheckBoxes[] = { &m_aDate,&m_aTime};
    for ( size_t i = 0 ; i < sizeof(pCheckBoxes)/sizeof(pCheckBoxes[0]); ++i)
        pCheckBoxes[i]->SetClickHdl(LINK(this,ODateTimeDialog,CBClickHdl));

    FreeResource();
}

//------------------------------------------------------------------------
ODateTimeDialog::~ODateTimeDialog()
{
    DBG_DTOR( rpt_ODateTimeDialog,NULL);
}
// -----------------------------------------------------------------------------
short ODateTimeDialog::Execute()
{
    DBG_CHKTHIS( rpt_ODateTimeDialog,NULL);
    short nRet = ModalDialog::Execute();
    if ( nRet == RET_OK && (m_aDate.IsChecked() || m_aTime.IsChecked()) )
    {
        try
        {
            sal_Int32 nLength = 0;
            uno::Sequence<beans::PropertyValue> aValues( 5 );
            aValues[nLength].Name = PROPERTY_SECTION;
            aValues[nLength++].Value <<= m_xHoldAlive;

            aValues[nLength].Name = PROPERTY_TIME_STATE;
            aValues[nLength++].Value <<= m_aTime.IsChecked();

            aValues[nLength].Name = PROPERTY_DATE_STATE;
            aValues[nLength++].Value <<= m_aDate.IsChecked();

            aValues[nLength].Name = PROPERTY_FORMATKEYDATE;
            aValues[nLength++].Value <<= getFormatKey(getFormatIndex(sal_True));

            aValues[nLength].Name = PROPERTY_FORMATKEYTIME;
            aValues[nLength++].Value <<= getFormatKey(getFormatIndex(sal_False));

            m_pController->executeChecked(SID_DATETIME,aValues);
        }
        catch(uno::Exception&)
        {
            nRet = RET_NO;
        }
    }
    return nRet;
}
// -----------------------------------------------------------------------------
::rtl::OUString ODateTimeDialog::getFormatString(::sal_Int16 _nNumberFormatIndex)
{
    DBG_CHKTHIS( rpt_ODateTimeDialog,NULL);
    uno::Reference< util::XNumberFormatter> xNumberFormatter = m_pController->getReportNumberFormatter();
    uno::Reference< util::XNumberFormats> xFormats = xNumberFormatter->getNumberFormatsSupplier()->getNumberFormats();
    uno::Reference< util::XNumberFormatTypes> xNumType(xFormats,uno::UNO_QUERY);
    sal_Int32 nFormatKey = xNumType->getFormatIndex(_nNumberFormatIndex,m_nLocale);

    uno::Reference< beans::XPropertySet> xFormSet = xFormats->getByKey(nFormatKey);
    OSL_ENSURE(xFormSet.is(),"XPropertySet is null!");
    ::rtl::OUString sFormat;
    xFormSet->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FormatString"))) >>= sFormat;

    lang::Locale aLocale;
    ::comphelper::getNumberFormatProperty(xNumberFormatter,nFormatKey,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Locale"))) >>= aLocale;

    double nValue = 0;
    if ( _nNumberFormatIndex >= i18n::NumberFormatIndex::TIME_START )
    {
        Time aCurrentTime;
        nValue = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toTime(aCurrentTime.GetTime()));
    }
    else
    {
        Date aCurrentDate;
        static ::com::sun::star::util::Date STANDARD_DB_DATE(31,12,1899);
        nValue = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toDate(static_cast<sal_Int32>(aCurrentDate.GetDate())),STANDARD_DB_DATE);
    }

    uno::Reference< util::XNumberFormatPreviewer> xPreViewer(xNumberFormatter,uno::UNO_QUERY);
    OSL_ENSURE(xPreViewer.is(),"XNumberFormatPreviewer is null!");
    return xPreViewer->convertNumberToPreviewString(sFormat,nValue,aLocale,sal_True);
}
// -----------------------------------------------------------------------------
IMPL_LINK( ODateTimeDialog, CBClickHdl, CheckBox*, _pBox )
{
    DBG_CHKTHIS( rpt_ODateTimeDialog,NULL);

    RadioButton* pRadioButtons[] = { &m_aDateF1,&m_aDateF2,&m_aDateF3, &m_aTimeF1, &m_aTimeF2, &m_aTimeF3};
    int i = 0;
    int nCount = 3;
    sal_Bool bShow = sal_False;
    if ( _pBox == &m_aDate )
    {
        bShow = m_aDate.IsChecked();
    }
    else if ( _pBox == &m_aTime )
    {
        bShow = m_aTime.IsChecked();
        i = 3;
        nCount = sizeof(pRadioButtons)/sizeof(pRadioButtons[0]);
    }

    for (  ; i < nCount ; ++i)
        pRadioButtons[i]->Enable(bShow);
    return 1L;
}
// -----------------------------------------------------------------------------
sal_Int16 ODateTimeDialog::getFormatIndex(sal_Bool _bDate)
{
    DBG_CHKTHIS( rpt_ODateTimeDialog,NULL);
    sal_Int16 nFormat;
    if ( _bDate )
        nFormat = m_aDateF1.IsChecked() ? i18n::NumberFormatIndex::DATE_SYSTEM_LONG : (m_aDateF2.IsChecked() ? i18n::NumberFormatIndex::DATE_SYS_DMMMYYYY : i18n::NumberFormatIndex::DATE_SYSTEM_SHORT);
    else
        nFormat = m_aTimeF1.IsChecked() ? i18n::NumberFormatIndex::TIME_HHMMSS : (m_aTimeF2.IsChecked() ? i18n::NumberFormatIndex::TIME_HHMMSSAMPM : i18n::NumberFormatIndex::TIME_HHMM);
    return nFormat;
}
// -----------------------------------------------------------------------------
sal_Int32 ODateTimeDialog::getFormatKey(::sal_Int16 _nNumberFormatIndex)
{
    uno::Reference< util::XNumberFormatter> xNumberFormatter = m_pController->getReportNumberFormatter();
    uno::Reference< util::XNumberFormats> xFormats = xNumberFormatter->getNumberFormatsSupplier()->getNumberFormats();
    uno::Reference< util::XNumberFormatTypes> xNumType(xFormats,uno::UNO_QUERY);
    return xNumType->getFormatIndex(_nNumberFormatIndex,m_nLocale);
}
// =============================================================================
} // rptui
// =============================================================================
