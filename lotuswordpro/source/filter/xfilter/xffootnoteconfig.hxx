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
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFFOOTNOTECONFIG_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFFOOTNOTECONFIG_HXX

#include "xfstyle.hxx"

class XFFootnoteConfig : public XFStyle
{
public:
    XFFootnoteConfig();

public:
    void    SetMasterPage(const OUString& masterPage);

    void    SetStartValue(sal_Int32 value=0);

    void    SetRestartOnPage();

    void    SetNumPrefix(const OUString& numprefix);

    void    SetNumSuffix(const OUString& numsuffix);

    void    SetMessageOn(const OUString& message);

    void    SetMessageFrom(const OUString& message);

    virtual void    ToXml(IXFStream *pStrm) override;

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
    bool    m_bInsertInPage;
    bool    m_bIsFootnote;
};

inline XFFootnoteConfig::XFFootnoteConfig()
{
    m_nStartValue = 0;
    m_nRestartType = -1;
    m_strNumFmt = "1";
    m_bInsertInPage = true;
    m_strDefaultStyle = "Footnote";
    m_strMasterPage = "Footnote";
    m_strCitationStyle = "Footnote Symbol";
    m_strBodyStyle = "Footnote anchor";
    m_bIsFootnote = true;
}

inline void XFFootnoteConfig::SetMasterPage(const OUString& masterPage)
{
    m_strMasterPage = masterPage;
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

inline void XFFootnoteConfig::SetNumPrefix(const OUString& numprefix)
{
    m_strNumPrefix = numprefix;
}

inline void XFFootnoteConfig::SetNumSuffix(const OUString& numsuffix)
{
    m_strNumSuffix = numsuffix;
}

inline void XFFootnoteConfig::SetMessageOn(const OUString& message)
{
    m_strMessageOn = message;
}

inline void XFFootnoteConfig::SetMessageFrom(const OUString& message)
{
    m_strMessageFrom = message;
}

inline void XFFootnoteConfig::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    if( !m_strCitationStyle.isEmpty() )
        pAttrList->AddAttribute( "text:citation-style-name", m_strCitationStyle);

    if( !m_strBodyStyle.isEmpty() )
        pAttrList->AddAttribute( "text:citation-body-style-name", m_strBodyStyle);

    if( !m_strNumPrefix.isEmpty() )
        pAttrList->AddAttribute( "style:num-prefix", m_strNumPrefix);

    if( !m_strNumSuffix.isEmpty() )
        pAttrList->AddAttribute( "style:num-suffix", m_strNumSuffix);

    if( !m_strNumFmt.isEmpty() )
        pAttrList->AddAttribute( "style:num-format", m_strNumFmt);

    if( !m_strDefaultStyle.isEmpty() )
        pAttrList->AddAttribute( "text:default-style-name", m_strDefaultStyle);

    if( !m_strMasterPage.isEmpty() )
        pAttrList->AddAttribute( "text:master-page-name", m_strMasterPage);

    pAttrList->AddAttribute( "text:start-value", OUString::number(m_nStartValue) );

    if( m_bIsFootnote )
    {
        if( m_nRestartType == -1 )
            pAttrList->AddAttribute( "text:start-numbering-at", "document" );
        else if( m_nRestartType == 0 )
            pAttrList->AddAttribute( "text:start-numbering-at", "page" );
        else if( m_nRestartType == 1 )
            pAttrList->AddAttribute( "text:start-numbering-at", "chapter" );

        if( m_bInsertInPage )
            pAttrList->AddAttribute( "text:footnotes-position", "page" );
        else
            pAttrList->AddAttribute( "text:footnotes-position", "document" );
    }

    if(m_bIsFootnote)
    {
        pStrm->StartElement( "text:footnotes-configuration" );
        if( !m_strMessageOn.isEmpty() )
        {
            pStrm->StartElement( "text:footnote-continuation-notice-forward" );
            pStrm->Characters(m_strMessageOn);
            pStrm->EndElement( "text:footnote-continuation-notice-forward" );
        }

        if( !m_strMessageFrom.isEmpty() )
        {
            pStrm->StartElement( "text:footnote-continuation-notice-backward" );
            pStrm->Characters(m_strMessageFrom);
            pStrm->EndElement( "text:footnote-continuation-notice-backward" );
        }

        pStrm->EndElement( "text:footnotes-configuration" );
    }
    else
    {
        pStrm->StartElement( "text:endnotes-configuration" );
        pStrm->EndElement( "text:endnotes-configuration" );
    }

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
