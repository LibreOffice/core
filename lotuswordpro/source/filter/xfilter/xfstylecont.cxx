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
/*************************************************************************
 * Change History
 * 2004-12-23 create this file.
 ************************************************************************/
#include	"xfstylecont.hxx"
#include	"ixfstyle.hxx"
#include	"xffont.hxx"
#include	"xftextstyle.hxx"
#include	"xfparastyle.hxx"
#include	"xffontfactory.hxx"
#include "../lwpglobalmgr.hxx"

XFStyleContainer::XFStyleContainer(const rtl::OUString& strStyleNamePrefix)
    :m_strStyleNamePrefix(strStyleNamePrefix)
{
}

XFStyleContainer::XFStyleContainer(const XFStyleContainer& other)
    :m_strStyleNamePrefix(other.m_strStyleNamePrefix)
{
    this->m_aStyles = other.m_aStyles;
}

XFStyleContainer& XFStyleContainer::operator=(const XFStyleContainer& other)
{
    this->m_strStyleNamePrefix = other.m_strStyleNamePrefix;
    this->m_aStyles = other.m_aStyles;
    return *this;
}

XFStyleContainer::~XFStyleContainer()
{
    std::vector<IXFStyle*>::iterator it;
    for( it = m_aStyles.begin(); it != m_aStyles.end(); it++ )
    {
        IXFStyle *pStyle = *it;
        if( pStyle )
            delete pStyle;
    }
}


void	XFStyleContainer::Reset()
{
    std::vector<IXFStyle*>::iterator it;

    for( it = m_aStyles.begin(); it != m_aStyles.end(); it++ )
    {
        IXFStyle *pStyle = *it;
        if( pStyle )
            delete pStyle;
    }
    m_aStyles.clear();
}

IXFStyle*	XFStyleContainer::AddStyle(IXFStyle *pStyle)
{
    IXFStyle	*pConStyle = NULL;
    rtl::OUString	name;

    if( !pStyle )
        return NULL;
    //no matter we want to delete the style or not,XFFont obejct should be saved first.
    ManageStyleFont(pStyle);

    if( pStyle->GetStyleName().getLength()==0 )
        pConStyle = FindSameStyle(pStyle);

    if( pConStyle )//such a style has exist:
    {
        delete pStyle;
        return pConStyle;
    }
    else
    {
        if( pStyle->GetStyleName().getLength() == 0 )
        {
            name = m_strStyleNamePrefix + Int32ToOUString(m_aStyles.size()+1);
            pStyle->SetStyleName(name);
        }
        else
        {
            name = pStyle->GetStyleName();
            //for name conflict
            if(FindStyle( name))
            {
                name = name + Int32ToOUString(m_aStyles.size()+1);
                pStyle->SetStyleName(name);
            }
        }


        m_aStyles.push_back(pStyle);
        //transform the font object to XFFontFactory

        return pStyle;
    }
}

IXFStyle*	XFStyleContainer::FindSameStyle(IXFStyle *pStyle)
{
    std::vector<IXFStyle*>::iterator it;
    for( it = m_aStyles.begin(); it != m_aStyles.end(); it++ )
    {
        IXFStyle *pConStyle = *it;
        if( !pConStyle )
            continue;

        if( pConStyle->Equal(pStyle) )
            return pConStyle;
    }

    return NULL;
}

IXFStyle*	XFStyleContainer::FindStyle(rtl::OUString name)
{
    std::vector<IXFStyle*>::iterator it;
    for( it = m_aStyles.begin(); it != m_aStyles.end(); it++ )
    {
        IXFStyle *pConStyle = *it;
        if( !pConStyle )
            continue;

        if( pConStyle->GetStyleName() == name )
            return pConStyle;
    }

    return NULL;
}

IXFStyle*	XFStyleContainer::Item(size_t index)
{
    assert(index<m_aStyles.size());
    if (index < m_aStyles.size())
    {
        return m_aStyles[index];
    }
    return NULL;
}

void	XFStyleContainer::ToXml(IXFStream *pStrm)
{
    std::vector<IXFStyle*>::iterator it;

    for( it = m_aStyles.begin(); it != m_aStyles.end(); it++ )
    {
        IXFStyle *pStyle = *it;

        assert(pStyle);
        if( !pStyle )
            continue;

        pStyle->ToXml(pStrm);
    }
}


void	XFStyleContainer::ManageStyleFont(IXFStyle *pStyle)
{
    XFFont *pStyleFont = NULL;
    XFFont *pFactoryFont = NULL;

    if( !pStyle )
        return;

    if( pStyle->GetStyleFamily() == enumXFStyleText )
    {
        XFTextStyle	*pTS = (XFTextStyle*)pStyle;
        pStyleFont = pTS->GetFont();
        if( !pStyleFont )
            return;
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        XFFontFactory* pFontFactory = pGlobal->GetXFFontFactory();
        pFactoryFont = pFontFactory->FindSameFont(pStyleFont);
        //this font has been exists in the factory:
        if( pFactoryFont )
        {
            pTS->SetFont(pFactoryFont);
            if( pStyleFont != pFactoryFont )
                delete pStyleFont;
        }
        else
        {
            pFontFactory->AddFont(pStyleFont);
        }
    }
    else if( pStyle->GetStyleFamily() == enumXFStylePara )
    {
        XFParaStyle	*pPS = (XFParaStyle*)pStyle;
        pStyleFont = pPS->GetFont();
        if( !pStyleFont )
            return;
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        XFFontFactory* pFontFactory = pGlobal->GetXFFontFactory();
        pFactoryFont = pFontFactory->FindSameFont(pStyleFont);
        //this font has been exists in the factory:
        if( pFactoryFont )
        {
            pPS->SetFont(pFactoryFont);
            if( pFactoryFont != pStyleFont )
                delete pStyleFont;
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
        IXFStyle *pS1 = b1.m_aStyles[i];
        IXFStyle *pS2 = b2.m_aStyles[i];

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
