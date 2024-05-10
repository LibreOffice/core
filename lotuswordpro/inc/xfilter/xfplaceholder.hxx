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
 * place holder object.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFPLACEHOLDER_HXX
#define INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFPLACEHOLDER_HXX

#include <xfilter/ixfattrlist.hxx>
#include <xfilter/xfcontent.hxx>

/**
 * @brief
 * Index entry object.
 */
class XFHolderStart : public XFContent
{
public:
    XFHolderStart();
    virtual ~XFHolderStart() override;
    void SetType(const OUString& sType);
    void SetDesc(const OUString& sDesc);
    void SetPrompt(const OUString& sText);
    void ToXml(IXFStream* pStrm) override;

private:
    OUString m_strType;
    OUString m_strDesc;
    OUString m_strText;
};

XFHolderStart::XFHolderStart() {}

XFHolderStart::~XFHolderStart() {}

inline void XFHolderStart::SetType(const OUString& sType) { m_strType = sType; }

inline void XFHolderStart::SetDesc(const OUString& sDesc) { m_strDesc = sDesc; }

inline void XFHolderStart::SetPrompt(const OUString& sText) { m_strText = sText; }

inline void XFHolderStart::ToXml(IXFStream* pStrm)
{
    IXFAttrList* pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pAttrList->AddAttribute(u"text:placeholder-type"_ustr, m_strType);
    if (!m_strDesc.isEmpty())
        pAttrList->AddAttribute(u"text:description"_ustr, m_strDesc);
    pStrm->StartElement(u"text:placeholder"_ustr);
    if (!m_strText.isEmpty())
        pStrm->Characters(m_strText);
}

class XFHolderEnd : public XFContent
{
public:
    XFHolderEnd();
    virtual ~XFHolderEnd() override;
    void ToXml(IXFStream* pStrm) override;
};

XFHolderEnd::XFHolderEnd() {}

XFHolderEnd::~XFHolderEnd() {}

inline void XFHolderEnd::ToXml(IXFStream* pStrm) { pStrm->EndElement(u"text:placeholder"_ustr); }

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
