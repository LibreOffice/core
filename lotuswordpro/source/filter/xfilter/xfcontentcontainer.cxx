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
 * Container for content.It will destroy all children when destroy.
 ************************************************************************/
#include "xfcontentcontainer.hxx"
#include "xftextcontent.hxx"

XFContentContainer::XFContentContainer()
{
}

XFContentContainer::~XFContentContainer()
{
}

void    XFContentContainer::Add(XFContent *pContent)
{
    m_aContents.push_back(pContent);
}

void    XFContentContainer::Add(const OUString& text)
{
    XFTextContent *pTC = new XFTextContent();
    pTC->SetText(text);
    Add(pTC);
}

int     XFContentContainer::GetCount() const
{
    return m_aContents.size();
}

void    XFContentContainer::Reset()
{
    m_aContents.clear();
}

rtl::Reference<XFContent> XFContentContainer::FindFirstContent(enumXFContent type)
{
    rtl::Reference<XFContent> pRet;
    rtl::Reference<XFContent> pContent;

    for( int i=0; i<GetCount(); i++ )
    {
        pContent = GetContent(i);
        if( !pContent.is() )
            continue;

        enumXFContent eType = pContent->GetContentType();
        if( eType == type )
            return pContent;
        else
        {
            XFContentContainer *pChildCont = static_cast<XFContentContainer*>(pContent.get());
            if( pChildCont )
            {
                pRet = pChildCont->FindFirstContent(type);
                if( pRet.is() )
                    return pRet;
            }
        }
    }
    return pRet;
}

enumXFContent   XFContentContainer::GetContentType()
{
    return enumXFContentContainer;
}

void    XFContentContainer::ToXml(IXFStream *pStrm)
{
    std::vector< rtl::Reference<XFContent> >::iterator it;

    for( it = m_aContents.begin(); it != m_aContents.end(); ++it )
    {
        XFContent *pContent = it->get();
        if( pContent )
            pContent->ToXml(pStrm);
    }
}

rtl::Reference<XFContent> XFContentContainer::GetLastContent()
{
    sal_uInt32 index = m_aContents.size()-1;
    if(index >0)
    {
        return m_aContents[index];
    }

    return nullptr;
}

void XFContentContainer::RemoveLastContent()
{
    sal_uInt32 index = m_aContents.size()-1;
    if(index >0)
    {
        m_aContents.erase(m_aContents.begin() + index);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
