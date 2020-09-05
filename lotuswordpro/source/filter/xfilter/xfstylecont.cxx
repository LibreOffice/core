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
 * Font object to serial to xml filter.
 ************************************************************************/
#include <xfilter/xfstylecont.hxx>
#include <xfilter/ixfstyle.hxx>
#include <xfilter/xffont.hxx>
#include <xfilter/xftextstyle.hxx>
#include <xfilter/xfparastyle.hxx>
#include <xfilter/xffontfactory.hxx>
#include <lwpglobalmgr.hxx>

XFStyleContainer::XFStyleContainer(const OUString& strStyleNamePrefix)
    :m_strStyleNamePrefix(strStyleNamePrefix)
{
}

XFStyleContainer::~XFStyleContainer()
{
}

void XFStyleContainer::Reset()
{
    m_aStyles.clear();
}

IXFStyleRet XFStyleContainer::AddStyle(std::unique_ptr<IXFStyle> pStyle)
{
    IXFStyleRet aRet;

    IXFStyle    *pConStyle = nullptr;

    if( !pStyle )
        return aRet;
    //no matter we want to delete the style or not,XFFont object should be saved first.
    ManageStyleFont(pStyle.get());

    if( pStyle->GetStyleName().isEmpty() )
        pConStyle = FindSameStyle(pStyle.get());

    if( pConStyle )//such a style has exist:
    {
        aRet.m_pStyle = pConStyle;
        aRet.m_bOrigDeleted = true;
        return aRet;
    }
    else
    {
        OUString   name;
        if( pStyle->GetStyleName().isEmpty() )
        {
            name = m_strStyleNamePrefix + OUString::number(m_aStyles.size()+1);
            pStyle->SetStyleName(name);
        }
        else
        {
            name = pStyle->GetStyleName();
            //for name conflict
            if(FindStyle( name))
            {
                name += OUString::number(m_aStyles.size()+1);
                pStyle->SetStyleName(name);
            }
        }

        //transform the font object to XFFontFactory
        aRet.m_pStyle = pStyle.get();
        m_aStyles.push_back(std::move(pStyle));
        return aRet;
    }
}

IXFStyle*   XFStyleContainer::FindSameStyle(IXFStyle *pStyle)
{
    for (auto const& style : m_aStyles)
    {
        assert(style);
        if( style->Equal(pStyle) )
            return style.get();
    }

    return nullptr;
}

IXFStyle*   XFStyleContainer::FindStyle(const OUString& name)
{
    for (auto const& style : m_aStyles)
    {
        assert(style);
        if( style->GetStyleName() == name )
            return style.get();
    }

    return nullptr;
}

const IXFStyle* XFStyleContainer::Item(size_t index) const
{
    assert(index<m_aStyles.size());
    if (index < m_aStyles.size())
    {
        return m_aStyles[index].get();
    }
    return nullptr;
}

void    XFStyleContainer::ToXml(IXFStream *pStrm)
{
    for (auto const& style : m_aStyles)
    {
        assert(style);
        style->ToXml(pStrm);
    }
}

void    XFStyleContainer::ManageStyleFont(IXFStyle *pStyle)
{
    rtl::Reference<XFFont> pStyleFont;
    rtl::Reference<XFFont> pFactoryFont;

    if( !pStyle )
        return;

    if( pStyle->GetStyleFamily() == enumXFStyleText )
    {
        XFTextStyle *pTS = static_cast<XFTextStyle*>(pStyle);
        pStyleFont = pTS->GetFont();
        if( !pStyleFont.is() )
            return;
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        XFFontFactory* pFontFactory = pGlobal->GetXFFontFactory();
        pFactoryFont = pFontFactory->FindSameFont(pStyleFont);
        //this font has been exists in the factory:
        if( pFactoryFont.is() )
        {
            pTS->SetFont(pFactoryFont);
        }
        else
        {
            pFontFactory->AddFont(pStyleFont);
        }
    }
    else if( pStyle->GetStyleFamily() == enumXFStylePara )
    {
        XFParaStyle *pPS = static_cast<XFParaStyle*>(pStyle);
        pStyleFont = pPS->GetFont();
        if( !pStyleFont.is() )
            return;
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        XFFontFactory* pFontFactory = pGlobal->GetXFFontFactory();
        pFactoryFont = pFontFactory->FindSameFont(pStyleFont);
        //this font has been exists in the factory:
        if( pFactoryFont.is() )
        {
            pPS->SetFont(pFactoryFont);
        }
        else
        {
            pFontFactory->AddFont(pStyleFont);
        }
    }
}

bool operator==(XFStyleContainer& b1, XFStyleContainer& b2)
{
    if( b1.m_strStyleNamePrefix != b2.m_strStyleNamePrefix )
        return false;
    if( b1.m_aStyles.size() != b2.m_aStyles.size() )
        return false;
    for( size_t i=0; i<b1.m_aStyles.size(); ++i )
    {
        IXFStyle *pS1 = b1.m_aStyles[i].get();
        IXFStyle *pS2 = b2.m_aStyles[i].get();

        if( pS1 )
        {
            if( !pS2 )
                return false;
            if( !pS1->Equal(pS2) )
                return false;
        }
        else
        {
            if( pS2 )
                return false;
        }
    }
    return true;
}

bool operator!=(XFStyleContainer& b1, XFStyleContainer& b2)
{
    return !(b1==b2);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
