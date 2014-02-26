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
 * Footnote config.
 ************************************************************************/
#ifndef     _XFFOOTNOTECONFIG_HXX
#define     _XFFOOTNOTECONFIG_HXX

#include "xfstyle.hxx"

class XFFootnoteConfig : public XFStyle
{
public:
    XFFootnoteConfig();

public:
    void    SetBodyStyle(OUString style);

    void    SetCitationStyle(OUString style);

    void    SetDefaultStyle(OUString style);

    void    SetMasterPage(OUString masterPage);

    void    SetNumberFormat(OUString numberFormat);

    void    SetStartValue(sal_Int32 value=0);

    void    SetRestartOnPage();

    void    SetRestartOnChapter();

    void    SetInsertInPage(sal_Bool page=sal_True);

    void    SetNumPrefix(OUString numprefix);

    void    SetNumSuffix(OUString numsuffix);

    void    SetMessageOn(OUString message);

    void    SetMessageFrom(OUString message);

    virtual void    ToXml(IXFStream *pStrm);

protected:
    OUString   m_strBodyStyle;
    OUString   m_strCitationStyle;
    OUString   m_strDefaultStyle;
    OUString   m_strMasterPage;
    OUString   m_strNumFmt;
    OUString   m_strNumPrefix;
    OUString   m_strNumSuffix;
    OUString   m_strMessageFrom;
    OUString   m_strMessageOn;
    sal_Int32   m_nStartValue;
    sal_Int32   m_nRestartType;
    sal_Bool    m_bInsertInPage;
    sal_Bool    m_bIsFootnote;
};

inline XFFootnoteConfig::XFFootnoteConfig()
{
    m_nStartValue = 0;
    m_nRestartType = -1;
    m_strNumFmt = A2OUSTR("1");
    m_bInsertInPage = sal_True;
    m_strDefaultStyle = A2OUSTR("Footnote");
    m_strMasterPage = A2OUSTR("Footnote");
    m_strCitationStyle = A2OUSTR("Footnote Symbol");
    m_strBodyStyle = A2OUSTR("Footnote anchor");
    m_bIsFootnote = sal_True;
}

inline void XFFootnoteConfig::SetBodyStyle(OUString style)
{
    m_strBodyStyle = style;
}

inline void XFFootnoteConfig::SetCitationStyle(OUString style)
{
    m_strCitationStyle = style;
}

inline void XFFootnoteConfig::SetDefaultStyle(OUString style)
{
    m_strDefaultStyle = style;
}

inline void XFFootnoteConfig::SetMasterPage(OUString masterPage)
{
    m_strMasterPage = masterPage;
}

inline void XFFootnoteConfig::SetNumberFormat(OUString numberFormat)
{
    m_strNumFmt = numberFormat;
}

inline void XFFootnoteConfig::SetStartValue(sal_Int32 value)
{
    if( value<0 )
        return;
    m_nStartValue = value;
}

inline void XFFootnoteConfig::SetRestartOnPage()
{
    m_nRestartType = 0;
}

inline void XFFootnoteConfig::SetRestartOnChapter()
{
    m_nRestartType = 1;
}

inline void XFFootnoteConfig::SetInsertInPage(sal_Bool page)
{
    m_bInsertInPage = page;
}

inline void XFFootnoteConfig::SetNumPrefix(OUString numprefix)
{
    m_strNumPrefix = numprefix;
}

inline void XFFootnoteConfig::SetNumSuffix(OUString numsuffix)
{
    m_strNumSuffix = numsuffix;
}

inline void XFFootnoteConfig::SetMessageOn(OUString message)
{
    m_strMessageOn = message;
}

inline void XFFootnoteConfig::SetMessageFrom(OUString message)
{
    m_strMessageFrom = message;
}

inline void XFFootnoteConfig::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    if( !m_strCitationStyle.isEmpty() )
        pAttrList->AddAttribute( A2OUSTR("text:citation-style-name"), m_strCitationStyle);

    if( !m_strBodyStyle.isEmpty() )
        pAttrList->AddAttribute( A2OUSTR("text:citation-body-style-name"), m_strBodyStyle);

    if( !m_strNumPrefix.isEmpty() )
        pAttrList->AddAttribute( A2OUSTR("style:num-prefix"), m_strNumPrefix);

    if( !m_strNumSuffix.isEmpty() )
        pAttrList->AddAttribute( A2OUSTR("style:num-suffix"), m_strNumSuffix);

    if( !m_strNumFmt.isEmpty() )
        pAttrList->AddAttribute( A2OUSTR("style:num-format"), m_strNumFmt);

    if( !m_strDefaultStyle.isEmpty() )
        pAttrList->AddAttribute( A2OUSTR("text:default-style-name"), m_strDefaultStyle);

    if( !m_strMasterPage.isEmpty() )
        pAttrList->AddAttribute( A2OUSTR("text:master-page-name"), m_strMasterPage);

    pAttrList->AddAttribute( A2OUSTR("text:start-value"), Int32ToOUString(m_nStartValue) );

    if( m_bIsFootnote )
    {
        if( m_nRestartType == -1 )
            pAttrList->AddAttribute( A2OUSTR("text:start-numbering-at"), A2OUSTR("document") );
        else if( m_nRestartType == 0 )
            pAttrList->AddAttribute( A2OUSTR("text:start-numbering-at"), A2OUSTR("page") );
        else if( m_nRestartType == 1 )
            pAttrList->AddAttribute( A2OUSTR("text:start-numbering-at"), A2OUSTR("chapter") );

        if( m_bInsertInPage )
            pAttrList->AddAttribute( A2OUSTR("text:footnotes-position"), A2OUSTR("page") );
        else
            pAttrList->AddAttribute( A2OUSTR("text:footnotes-position"), A2OUSTR("document") );
    }

    if(m_bIsFootnote)
    {
        pStrm->StartElement( A2OUSTR("text:footnotes-configuration") );
        if( !m_strMessageOn.isEmpty() )
        {
            pStrm->StartElement( A2OUSTR("text:footnote-continuation-notice-forward") );
            pStrm->Characters(m_strMessageOn);
            pStrm->EndElement( A2OUSTR("text:footnote-continuation-notice-forward") );
        }

        if( !m_strMessageFrom.isEmpty() )
        {
            pStrm->StartElement( A2OUSTR("text:footnote-continuation-notice-backward") );
            pStrm->Characters(m_strMessageFrom);
            pStrm->EndElement( A2OUSTR("text:footnote-continuation-notice-backward") );
        }

        pStrm->EndElement( A2OUSTR("text:footnotes-configuration") );
    }
    else
    {
        pStrm->StartElement( A2OUSTR("text:endnotes-configuration") );
        pStrm->EndElement( A2OUSTR("text:endnotes-configuration") );
    }

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
