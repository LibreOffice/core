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
 * ruby style.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFRUBYSTYLE_HXX
#define INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFRUBYSTYLE_HXX

#include <xfilter/ixfattrlist.hxx>
#include <xfilter/xfstyle.hxx>
#include <xfilter/xfdefs.hxx>

class XFRubyStyle : public XFStyle
{
public:
    XFRubyStyle()
        : m_ePos(enumXFRubyLeft)
        , m_eAlign(enumXFRubyLeft)
    {
    }
    virtual void ToXml(IXFStream* strm) override;
    void SetPosition(enumXFRubyPosition ePosition);
    void SetAlignment(enumXFRubyPosition eAlignment);
    enumXFStyle GetStyleFamily() override;

private:
    enumXFRubyPosition m_ePos;
    enumXFRubyPosition m_eAlign;
};

void XFRubyStyle::SetPosition(enumXFRubyPosition ePosition) { m_ePos = ePosition; }
void XFRubyStyle::SetAlignment(enumXFRubyPosition eAlignment) { m_eAlign = eAlignment; }
enumXFStyle XFRubyStyle::GetStyleFamily() { return enumXFStyleRuby; }
void XFRubyStyle::ToXml(IXFStream* pStrm)
{
    IXFAttrList* pAttrList = pStrm->GetAttrList();
    OUString style = GetStyleName();

    pAttrList->Clear();
    if (!style.isEmpty())
        pAttrList->AddAttribute(u"style:name"_ustr, GetStyleName());
    pAttrList->AddAttribute(u"style:family"_ustr, u"ruby"_ustr);
    pStrm->StartElement(u"style:style"_ustr);

    pAttrList->Clear();

    OUString sPos;
    if (m_eAlign == enumXFRubyLeft)
    {
        sPos = "left";
    }
    else if (m_eAlign == enumXFRubyRight)
    {
        sPos = "right";
    }
    else if (m_eAlign == enumXFRubyCenter)
    {
        sPos = "center";
    }
    if (!sPos.isEmpty())
        pAttrList->AddAttribute(u"style:ruby-align"_ustr, sPos);

    OUString sAlign;
    if (m_ePos == enumXFRubyTop)
    {
        sAlign = "above";
    }
    else if (m_ePos == enumXFRubyBottom)
    {
        sAlign = "below";
    }
    if (!sAlign.isEmpty())
        pAttrList->AddAttribute(u"style:ruby-position"_ustr, sAlign);

    pStrm->StartElement(u"style:properties"_ustr);
    pStrm->EndElement(u"style:properties"_ustr);
    pStrm->EndElement(u"style:style"_ustr);
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
