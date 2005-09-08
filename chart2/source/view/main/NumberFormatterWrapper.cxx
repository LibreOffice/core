/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NumberFormatterWrapper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:48:49 $
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
#include "chartview/NumberFormatterWrapper.hxx"
#include "macros.hxx"

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
// header for class SvNumberformat
#ifndef _ZFORMAT_HXX
#include <svtools/zformat.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

FixedNumberFormatter::FixedNumberFormatter(
                NumberFormatterWrapper* pNumberFormatterWrapper
                , const ::com::sun::star::chart2::NumberFormat& rNumberFormat )
            : m_pNumberFormatterWrapper(pNumberFormatterWrapper)
            , m_nFormatIndex(0)
{
    //add the given format string to the NumberFormatter and
    //get the index of that format to use further
    if( m_pNumberFormatterWrapper )
        m_nFormatIndex = m_pNumberFormatterWrapper->getKeyForNumberFormat( rNumberFormat );
}

FixedNumberFormatter::~FixedNumberFormatter()
{
}

/*
sal_Int32 FixedNumberFormatter::getTextAndColor( double fUnscaledValueForText, rtl::OUString& rLabel ) const
{
    sal_Int32 nLabelColor = Color(COL_BLUE).GetColor(); //@todo get this from somewheres
    rLabel = getFormattedString( fUnscaledValueForText, nLabelColor );
    return nLabelColor;
}
*/

rtl::OUString FixedNumberFormatter::getFormattedString( double fValue, sal_Int32& rLabelColor, bool& rbColorChanged ) const
{
    String aText;
    Color* pTextColor = NULL;
    m_pNumberFormatterWrapper->m_pNumberFormatter->GetOutputString(
        fValue, m_nFormatIndex, aText, &pTextColor);
    rtl::OUString aRet( aText );

    if(pTextColor)
    {
        rbColorChanged = true;
        rLabelColor = pTextColor->GetColor();
    }
    else
        rbColorChanged = false;

    return aRet;
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

NumberFormatterWrapper::NumberFormatterWrapper()
                    : m_pNumberFormatter(NULL)

{
    //@todo: use a formatter via api (does not exsist so far)
    m_pNumberFormatter = new SvNumberFormatter(
        //m_xCC->getServiceManager() ,
        ::comphelper::getProcessServiceFactory(),
        LANGUAGE_SYSTEM );
}
NumberFormatterWrapper::~NumberFormatterWrapper()
{
    delete m_pNumberFormatter;
}

SvNumberFormatter* NumberFormatterWrapper::getSvNumberFormatter() const
{
    return m_pNumberFormatter;
}

LanguageType lcl_GetLanguage( const lang::Locale& rLocale )
{
    //  empty language -> LANGUAGE_SYSTEM
    if ( rLocale.Language.getLength() == 0 )
        return LANGUAGE_SYSTEM;

    String aLangStr = rLocale.Language;
    String aCtryStr = rLocale.Country;
    //  Variant is ignored

    LanguageType eRet = ConvertIsoNamesToLanguage( aLangStr, aCtryStr );
    if ( eRet == LANGUAGE_NONE )
        eRet = LANGUAGE_SYSTEM;         //! or throw an exception?

    return eRet;
}

sal_Int32 NumberFormatterWrapper::getKeyForNumberFormat( const NumberFormat& rNumberFormat ) const
{
    String aStr( rNumberFormat.aFormat );
    LanguageType eLnge = lcl_GetLanguage( rNumberFormat.aLocale );
    return m_pNumberFormatter->GetEntryKey( aStr, eLnge );
}

NumberFormat NumberFormatterWrapper::getNumberFormatForKey( sal_Int32 nIndex ) const
{
    const SvNumberformat* pSvNumberformat = m_pNumberFormatter->GetEntry( nIndex );

    NumberFormat aNumberFormat;
    aNumberFormat.aFormat = pSvNumberformat->GetFormatstring();
    LanguageType nLanguageType = pSvNumberformat->GetLanguage();
    aNumberFormat.aLocale = SvNumberFormatter::ConvertLanguageToLocale( nLanguageType );
    return aNumberFormat;
}

/*
    uno::Reference< i18n::XNumberFormatCode > xNumberFormatCode(
        m_xCC->getServiceManager()->createInstanceWithContext( C2U(
        "com.sun.star.i18n.NumberFormatMapper" ), m_xCC ), uno::UNO_QUERY );

    i18n::NumberFormatCode aNumberFormatCode = xNumberFormatCode->getDefault (
        i18n::KNumberFormatType::MEDIUM,
        i18n::KNumberFormatUsage::SCIENTIFIC_NUMBER,
        aLocale );

    uno::Sequence< i18n::NumberFormatCode > aListOfNumberFormatCode = xNumberFormatCode->getAllFormatCode(
        i18n::KNumberFormatUsage::SCIENTIFIC_NUMBER,
        aLocale );

    i18n::NumberFormatCode aNumberFormatCode0 = aListOfNumberFormatCode[0];
    i18n::NumberFormatCode aNumberFormatCode1 = aListOfNumberFormatCode[1];
*/

//.............................................................................
} //namespace chart
//.............................................................................
