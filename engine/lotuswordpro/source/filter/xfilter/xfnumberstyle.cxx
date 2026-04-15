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
#include <xfilter/ixfattrlist.hxx>
#include <xfilter/xfnumberstyle.hxx>

XFNumberStyle::XFNumberStyle()
    : m_eType(enumXFNumberNumber)
    , m_nDecimalDigits(0)
    , m_bGroup(false)
    , m_aColor(0,0,0)
    , m_bRedIfNegative(false)
    , m_aNegativeColor(255,0,0)
{}

enumXFStyle XFNumberStyle::GetStyleFamily()
{
    return enumXFStyleNumber;
}

bool XFNumberStyle::Equal(IXFStyle *pStyle)
{
    if( !pStyle || pStyle->GetStyleFamily() != enumXFStyleNumber )
        return false;
    XFNumberStyle *pOther = dynamic_cast<XFNumberStyle*>(pStyle);
    if( !pOther )
        return false;

    if( m_eType != pOther->m_eType )
        return false;
    if( m_nDecimalDigits != pOther->m_nDecimalDigits )
        return false;
    if( m_bRedIfNegative != pOther->m_bRedIfNegative )
        return false;
    if( m_bGroup != pOther->m_bGroup )
        return false;
    if( m_aColor != pOther->m_aColor )
        return false;
    if( m_strPrefix != pOther->m_strPrefix )
        return false;
    if( m_strSuffix != pOther->m_strSuffix )
        return false;

    if( m_bRedIfNegative )
    {
        if( m_aNegativeColor != pOther->m_aNegativeColor )
            return false;
        if( m_strNegativePrefix != pOther->m_strNegativePrefix )
            return false;
        if( m_strNegativeSuffix != pOther->m_strNegativeSuffix )
            return false;
    }

    if( m_eType == enuMXFNumberCurrency )
    {
        if( m_strCurrencySymbol != pOther->m_strCurrencySymbol )
            return false;
    }

    return true;
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

    pAttrList->AddAttribute( u"style:name"_ustr, GetStyleName() );
    if( !GetParentStyleName().isEmpty() )
        pAttrList->AddAttribute(u"style:parent-style-name"_ustr,GetParentStyleName());

    pAttrList->AddAttribute( u"style:family"_ustr, u"data-style"_ustr );

    if( m_eType == enumXFNumberNumber )
    {
        pStrm->StartElement( u"number:number-style"_ustr );
    }
    else if( m_eType == enumXFNumberPercent )
    {
        pStrm->StartElement( u"number:percentage-style"_ustr );
    }
    else if( m_eType == enuMXFNumberCurrency )
    {
        pStrm->StartElement( u"number:currency-style"_ustr );
    }
    else if( m_eType == enumXFNumberScientific )
    {
        pStrm->StartElement( u"number:number-style"_ustr );
    }
    // for Text content number format
    else if (m_eType == enumXFText)
    {
        pStrm->StartElement( u"number:text-content"_ustr);
    }

}

void XFNumberStyle::ToXml_EndElement(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pAttrList->AddAttribute( u"style:name"_ustr, GetStyleName() );
    pAttrList->AddAttribute( u"style:family"_ustr, u"data-style"_ustr );

    if( m_eType == enumXFNumberNumber )
    {
        pStrm->EndElement( u"number:number-style"_ustr );
    }
    else if( m_eType == enumXFNumberPercent )
    {
        pStrm->EndElement( u"number:percentage-style"_ustr );
    }
    else if( m_eType == enuMXFNumberCurrency )
    {
        pStrm->EndElement( u"number:currency-style"_ustr );
    }
    else if( m_eType == enumXFNumberScientific )
    {
        pStrm->EndElement( u"number:number-style"_ustr );
    }
    // for Text content number format
    else if (m_eType == enumXFText)
    {
        pStrm->EndElement( u"number:text-content"_ustr);
    }
    // END for Text content number format

}
void XFNumberStyle::ToXml_Normal(IXFStream *pStrm)
{
    ToXml_StartElement(pStrm);

    ToXml_Content(pStrm,false);

    ToXml_EndElement(pStrm);
}

void XFNumberStyle::ToXml_Negative(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    OUString strStyleName = GetStyleName();
    OUString strGEStyle = strStyleName + "PO";

    SetStyleName(strGEStyle);
    ToXml_Normal(pStrm);
    SetStyleName(strStyleName);

    ToXml_StartElement(pStrm);

    ToXml_Content(pStrm,true);

    pAttrList->Clear();
    pAttrList->AddAttribute( u"style:condition"_ustr, u"value()>=0"_ustr );
    pAttrList->AddAttribute( u"style:apply-style-name"_ustr, strGEStyle );
    pStrm->StartElement( u"style:map"_ustr );
    pStrm->EndElement( u"style:map"_ustr );

    ToXml_EndElement(pStrm);
}

void XFNumberStyle::ToXml_Content(IXFStream *pStrm, bool nagetive)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    //color:
    if( !nagetive )
        pAttrList->AddAttribute( u"fo:color"_ustr, m_aColor.ToString() );
    else
        pAttrList->AddAttribute( u"fo:color"_ustr, m_aNegativeColor.ToString() );

    pStrm->StartElement( u"style:properties"_ustr );
    pStrm->EndElement( u"style:properties"_ustr );

    if( !nagetive )
    {
        if( !m_strPrefix.isEmpty() )
        {
            pStrm->StartElement( u"number:text"_ustr );
            pStrm->Characters(m_strPrefix);
            pStrm->EndElement( u"number:text"_ustr );
        }
    }
    else
    {
        if( m_strNegativePrefix.isEmpty() )
            m_strNegativePrefix = m_strPrefix;
        if( !m_strNegativePrefix.isEmpty() )
        {
            pStrm->StartElement( u"number:text"_ustr );
            // pStrm->Characters(m_strNegativePrefix);
            pStrm->Characters(m_strNegativePrefix + "-");
            pStrm->EndElement( u"number:text"_ustr );
        }
        else
        {
            pStrm->StartElement( u"number:text"_ustr );
            pStrm->Characters(u"-"_ustr);
            pStrm->EndElement( u"number:text"_ustr );
        }
    }

    if( m_eType == enuMXFNumberCurrency )
    {
        if( !m_strCurrencySymbol.isEmpty() )
        {
            pStrm->StartElement( u"number:currency-symbol"_ustr );
            pStrm->Characters(m_strCurrencySymbol);
            pStrm->EndElement( u"number:currency-symbol"_ustr );
        }
    }

    //When category of number format is scientific, the number can not be displayed normally in table.
    if ( m_eType == enumXFNumberScientific )
    {
        pAttrList->Clear();
        pAttrList->AddAttribute(u"number:decimal-places"_ustr, OUString::number(m_nDecimalDigits));
        pAttrList->AddAttribute(u"number:min-integer-digits"_ustr, OUString::number(1));
        pAttrList->AddAttribute(u"number:min-exponent-digits"_ustr, OUString::number(2));
        pStrm->StartElement( u"number:scientific-number"_ustr );
        pStrm->EndElement( u"number:scientific-number"_ustr );
    }
    else
    {
        pAttrList->Clear();
        pAttrList->AddAttribute(u"number:decimal-places"_ustr, OUString::number(m_nDecimalDigits));
        pAttrList->AddAttribute(u"number:min-integer-digits"_ustr, OUString::number(1));

        if( m_bGroup )
            pAttrList->AddAttribute(u"number:grouping"_ustr,u"true"_ustr);
        else
            pAttrList->AddAttribute(u"number:grouping"_ustr,u"false"_ustr);

        pStrm->StartElement( u"number:number"_ustr );
        pStrm->EndElement( u"number:number"_ustr );
    }

    if( !nagetive )
    {
        if( !m_strSuffix.isEmpty() )
        {
            pStrm->StartElement( u"number:text"_ustr );
            pStrm->Characters(m_strSuffix);
            pStrm->EndElement( u"number:text"_ustr );
        }
        else
        {
            if( m_eType == enumXFNumberPercent )
            {
                pStrm->StartElement( u"number:text"_ustr );
                pStrm->Characters(u"%"_ustr);
                pStrm->EndElement( u"number:text"_ustr );
            }
        }
    }
    else
    {
        if( m_strNegativeSuffix.isEmpty() )
            m_strNegativeSuffix = m_strSuffix;
        if( !m_strNegativeSuffix.isEmpty() )
        {
            pStrm->StartElement( u"number:text"_ustr );
            pStrm->Characters(m_strNegativeSuffix);
            pStrm->EndElement( u"number:text"_ustr );
        }
        else
        {
            if( m_eType == enumXFNumberPercent )
            {
                pStrm->StartElement( u"number:text"_ustr );
                pStrm->Characters(u"%"_ustr);
                pStrm->EndElement( u"number:text"_ustr );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
