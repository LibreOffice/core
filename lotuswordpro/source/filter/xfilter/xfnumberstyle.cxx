/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 * Number style for table cell.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-03-23 create this file.
 ************************************************************************/
#include	"xfnumberstyle.hxx"

XFNumberStyle::XFNumberStyle():m_aColor(0,0,0),m_aNegativeColor(255,0,0)
{
    m_eType = enumXFNumberNumber;
    m_nDecimalDigits = 0;
    m_nMinInteger = 1;
    m_nMinExponent = 2;
    m_bGroup = sal_False;
    m_bRedIfNegative = sal_False;
    m_bCurrencySymbolPost = sal_False;
}

XFNumberStyle::XFNumberStyle(enumXFNumberType type)
{
    m_eType = type;
    m_nDecimalDigits = 0;
    m_nMinInteger = 1;
    m_nMinExponent = 1;
    m_bRedIfNegative = sal_False;
    m_bCurrencySymbolPost = sal_False;
}

enumXFStyle XFNumberStyle::GetStyleFamily()
{
    return enumXFStyleNumber;
}

sal_Bool XFNumberStyle::Equal(IXFStyle *pStyle)
{
    if( !pStyle || pStyle->GetStyleFamily() != enumXFStyleNumber )
        return sal_False;
    XFNumberStyle *pOther = (XFNumberStyle*)pStyle;
    if( !pOther )
        return sal_False;

    if( m_eType != pOther->m_eType )
        return sal_False;
    if( m_nDecimalDigits != pOther->m_nDecimalDigits )
        return sal_False;
    if( m_nMinInteger != pOther->m_nMinInteger )
        return sal_False;
    if( m_bRedIfNegative != pOther->m_bRedIfNegative )
        return sal_False;
    if( m_bGroup != pOther->m_bGroup )
        return sal_False;
    if( m_aColor != pOther->m_aColor )
        return sal_False;
    if( m_strPrefix != pOther->m_strPrefix )
        return sal_False;
    if( m_strSuffix != pOther->m_strSuffix )
        return sal_False;

    //When category of number format is scientific, the number can not be displayed normally in table.
    if ( m_nMinExponent != pOther->m_nMinExponent )
        return sal_False;

    if( m_bRedIfNegative )
    {
        if( m_aNegativeColor != pOther->m_aNegativeColor )
            return sal_False;
        if( m_strNegativePrefix != pOther->m_strNegativePrefix )
            return sal_False;
        if( m_strNegativeSuffix != pOther->m_strNegativeSuffix )
            return sal_False;
    }

    if( m_eType == enuMXFNumberCurrency )
    {
        if( m_bCurrencySymbolPost != pOther->m_bCurrencySymbolPost )
            return sal_False;
        if( m_strCurrencySymbol != pOther->m_strCurrencySymbol )
            return sal_False;
    }

    return sal_True;
}

void XFNumberStyle::ToXml(IXFStream *pStrm)
{
    // for Text content number format
    if (m_eType == enumXFText)
    {
        ToXml_StartElement(pStrm);
        ToXml_EndElement(pStrm);
        return;
    }
    // END for Text content number format

    if( !m_bRedIfNegative )
    {
        ToXml_Normal(pStrm);
    }
    else
    {
        ToXml_Negative(pStrm);
    }
}

void XFNumberStyle::ToXml_StartElement(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pAttrList->AddAttribute( A2OUSTR("style:name"), GetStyleName() );
    if( GetParentStyleName().getLength() > 0 )
        pAttrList->AddAttribute(A2OUSTR("style:parent-style-name"),GetParentStyleName());

    pAttrList->AddAttribute( A2OUSTR("style:family"), A2OUSTR("data-style") );

    if( m_eType == enumXFNumberNumber )
    {
        pStrm->StartElement( A2OUSTR("number:number-style") );
    }
    else if( m_eType == enumXFNumberPercent )
    {
        pStrm->StartElement( A2OUSTR("number:percentage-style") );
    }
    else if( m_eType == enuMXFNumberCurrency )
    {
        pStrm->StartElement( A2OUSTR("number:currency-style") );
    }
    else if( m_eType == enumXFNumberScientific )
    {
        pStrm->StartElement( A2OUSTR("number:number-style") );
    }
    // for Text content number format
    else if (m_eType == enumXFText)
    {
        pStrm->StartElement( A2OUSTR("number:text-content"));
    }
    // END for Text content number format

}

void XFNumberStyle::ToXml_EndElement(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pAttrList->AddAttribute( A2OUSTR("style:name"), GetStyleName() );
    pAttrList->AddAttribute( A2OUSTR("style:family"), A2OUSTR("data-style") );

    if( m_eType == enumXFNumberNumber )
    {
        pStrm->EndElement( A2OUSTR("number:number-style") );
    }
    else if( m_eType == enumXFNumberPercent )
    {
        pStrm->EndElement( A2OUSTR("number:percentage-style") );
    }
    else if( m_eType == enuMXFNumberCurrency )
    {
        pStrm->EndElement( A2OUSTR("number:currency-style") );
    }
    else if( m_eType == enumXFNumberScientific )
    {
        pStrm->EndElement( A2OUSTR("number:number-style") );
    }
    // for Text content number format
    else if (m_eType == enumXFText)
    {
        pStrm->EndElement( A2OUSTR("number:text-content"));
    }
    // END for Text content number format

}
void XFNumberStyle::ToXml_Normal(IXFStream *pStrm)
{
    ToXml_StartElement(pStrm);

    ToXml_Content(pStrm,sal_False);

    ToXml_EndElement(pStrm);
}

void XFNumberStyle::ToXml_Negative(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    rtl::OUString strStyleName = GetStyleName();
    rtl::OUString strGEStyle = strStyleName + A2OUSTR("PO");

    SetStyleName(strGEStyle);
    ToXml_Normal(pStrm);
    SetStyleName(strStyleName);

    ToXml_StartElement(pStrm);

    ToXml_Content(pStrm,sal_True);

    pAttrList->Clear();
    pAttrList->AddAttribute( A2OUSTR("style:condition"), A2OUSTR("value()>=0") );
    pAttrList->AddAttribute( A2OUSTR("style:apply-style-name"), strGEStyle );
    pStrm->StartElement( A2OUSTR("style:map") );
    pStrm->EndElement( A2OUSTR("style:map") );

    ToXml_EndElement(pStrm);
}

void XFNumberStyle::ToXml_Content(IXFStream *pStrm, sal_Bool nagetive)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    //color:
    if( !nagetive )
        pAttrList->AddAttribute( A2OUSTR("fo:color"), m_aColor.ToString() );
    else
        pAttrList->AddAttribute( A2OUSTR("fo:color"), m_aNegativeColor.ToString() );

    pStrm->StartElement( A2OUSTR("style:properties") );
    pStrm->EndElement( A2OUSTR("style:properties") );

    if( !nagetive )
    {
        if( m_strPrefix.getLength()>0 )
        {
            pStrm->StartElement( A2OUSTR("number:text") );
            pStrm->Characters(m_strPrefix);
            pStrm->EndElement( A2OUSTR("number:text") );
        }
    }
    else
    {
        if( m_strNegativePrefix.getLength() == 0 )
            m_strNegativePrefix = m_strPrefix;
        if( m_strNegativePrefix.getLength()>0 )
        {
            pStrm->StartElement( A2OUSTR("number:text") );
            // pStrm->Characters(m_strNegativePrefix);
            pStrm->Characters(m_strNegativePrefix + A2OUSTR("-"));
            pStrm->EndElement( A2OUSTR("number:text") );
        }
        else
        {
            pStrm->StartElement( A2OUSTR("number:text") );
            pStrm->Characters(A2OUSTR("-"));
            pStrm->EndElement( A2OUSTR("number:text") );
        }
    }

    if( m_eType == enuMXFNumberCurrency && !m_bCurrencySymbolPost )
    {
        if( m_strCurrencySymbol.getLength() > 0 )
        {
            pStrm->StartElement( A2OUSTR("number:currency-symbol") );
            pStrm->Characters(m_strCurrencySymbol);
            pStrm->EndElement( A2OUSTR("number:currency-symbol") );
        }
    }

    //When category of number format is scientific, the number can not be displayed normally in table.
    if ( m_eType == enumXFNumberScientific )
    {
        pAttrList->Clear();
        pAttrList->AddAttribute(A2OUSTR("number:decimal-places"), Int32ToOUString(m_nDecimalDigits));
        pAttrList->AddAttribute(A2OUSTR("number:min-integer-digits"), Int32ToOUString(m_nMinInteger));
        pAttrList->AddAttribute(A2OUSTR("number:min-exponent-digits"), Int32ToOUString(m_nMinExponent));
        pStrm->StartElement( A2OUSTR("number:scientific-number") );
        pStrm->EndElement( A2OUSTR("number:scientific-number") );
    }
    else
    {
        pAttrList->Clear();
        pAttrList->AddAttribute(A2OUSTR("number:decimal-places"),Int32ToOUString(m_nDecimalDigits));
        pAttrList->AddAttribute(A2OUSTR("number:min-integer-digits"),Int32ToOUString(m_nMinInteger));

        if( m_bGroup )
            pAttrList->AddAttribute(A2OUSTR("number:grouping"),A2OUSTR("true"));
        else
            pAttrList->AddAttribute(A2OUSTR("number:grouping"),A2OUSTR("false"));

        pStrm->StartElement( A2OUSTR("number:number") );
        pStrm->EndElement( A2OUSTR("number:number") );
    }

    if( m_eType == enuMXFNumberCurrency && m_bCurrencySymbolPost )
    {
        if( m_strCurrencySymbol.getLength()>0 )
        {
            pStrm->StartElement( A2OUSTR("number:currency-symbol") );
            pStrm->Characters(m_strCurrencySymbol);
            pStrm->EndElement( A2OUSTR("number:currency-symbol") );
        }
    }

    if( !nagetive )
    {
        if( m_strSuffix.getLength()>0 )
        {
            pStrm->StartElement( A2OUSTR("number:text") );
            pStrm->Characters(m_strSuffix);
            pStrm->EndElement( A2OUSTR("number:text") );
        }
        else
        {
            if( m_eType == enumXFNumberPercent )
            {
                pStrm->StartElement( A2OUSTR("number:text") );
                pStrm->Characters(A2OUSTR("%"));
                pStrm->EndElement( A2OUSTR("number:text") );
            }
        }
    }
    else
    {
        if( m_strNegativeSuffix.getLength() == 0 )
            m_strNegativeSuffix = m_strSuffix;
        if( m_strNegativeSuffix.getLength()>0 )
        {
            pStrm->StartElement( A2OUSTR("number:text") );
            pStrm->Characters(m_strNegativeSuffix);
            pStrm->EndElement( A2OUSTR("number:text") );
        }
        else
        {
            if( m_eType == enumXFNumberPercent )
            {
                pStrm->StartElement( A2OUSTR("number:text") );
                pStrm->Characters(A2OUSTR("%"));
                pStrm->EndElement( A2OUSTR("number:text") );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
