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
 * Style for all draw object.
 ************************************************************************/
#include <xfilter/xfdrawstyle.hxx>
#include <xfilter/xfdrawlinestyle.hxx>
#include <xfilter/xfglobal.hxx>
#include "xfdrawareastyle.hxx"
#include <xfilter/xfstylemanager.hxx>
#include "xffontworkstyle.hxx"
#include <lwpglobalmgr.hxx>
XFDrawStyle::XFDrawStyle()
    : m_pLineStyle(nullptr)
    , m_pAreaStyle(nullptr)
    , m_fArrowStartSize(0.3)
    , m_fArrowEndSize(0.3)
    , m_bArrowStartCenter(false)
    , m_bArrowEndCenter(false)
    , m_bLineStyleRegistered(false)
    , m_bAreaStyleRegistered(false)
{}

XFDrawStyle::~XFDrawStyle()
{
    // normally don't delete m_pLineStyle, it was managed by XFStyleManager.
    if (!m_bLineStyleRegistered)
        delete m_pLineStyle;
    // normally don't delete m_pAreaStyle, it was managed by XFStyleManager.
    if (!m_bAreaStyleRegistered)
        delete m_pAreaStyle;
}

void    XFDrawStyle::SetLineStyle(double width, XFColor color)
{
    if( !m_pLineStyle )
    {
        m_pLineStyle = new XFDrawLineStyle();
        m_pLineStyle->SetStyleName( XFGlobal::GenStrokeDashName());
    }
    m_pLineStyle->SetWidth(width);
    m_pLineStyle->SetColor(color);
    m_pLineStyle->SetTransparency(0);
}

void    XFDrawStyle::SetLineDashStyle(enumXFLineStyle style, double len1, double len2, double space )
{
    if( !m_pLineStyle )
    {
        m_pLineStyle = new XFDrawLineStyle();
        m_pLineStyle->SetStyleName( XFGlobal::GenStrokeDashName());
    }
    m_pLineStyle->SetLineStyle(style);
    m_pLineStyle->SetDot1Number(1);
    m_pLineStyle->SetDot1Length(len1);
    m_pLineStyle->SetDot2Number(1);
    m_pLineStyle->SetDot2Length(len2);
    m_pLineStyle->SetSpace(space);
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_bLineStyleRegistered = true;
    pXFStyleManager->AddStyle(std::unique_ptr<IXFStyle>(m_pLineStyle));
}

void XFDrawStyle::SetFontWorkStyle(enumXFFWStyle eStyle, enumXFFWAdjust eAdjust)
{
    if (!m_pFontWorkStyle)
    {
        m_pFontWorkStyle.reset( new XFFontWorkStyle() );
    }

    m_pFontWorkStyle->SetButtonForm(0);
    m_pFontWorkStyle->SetFWStyleType(eStyle);
    m_pFontWorkStyle->SetFWAdjustType(eAdjust);
}

void    XFDrawStyle::SetAreaColor(XFColor const & color)
{
    if( !m_pAreaStyle )
    {
        m_pAreaStyle = new XFDrawAreaStyle();
        m_pAreaStyle->SetStyleName( XFGlobal::GenAreaName());
    }
    m_pAreaStyle->SetBackColor(color);
}

void    XFDrawStyle::SetAreaLineStyle(enumXFAreaLineStyle style, sal_Int32 angle, double space, XFColor lineColor)
{
    if( !m_pAreaStyle )
    {
        m_pAreaStyle = new XFDrawAreaStyle();
        m_pAreaStyle->SetStyleName( XFGlobal::GenAreaName());
    }
    m_pAreaStyle->SetAreaStyle(enumXFAreaHatch);

    m_pAreaStyle->SetLineStyle(style);
    m_pAreaStyle->SetLineAngle(angle);
    m_pAreaStyle->SetLineSpace(space);
    m_pAreaStyle->SetLineColor(lineColor);
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_bAreaStyleRegistered = true;
    pXFStyleManager->AddStyle(std::unique_ptr<IXFStyle>(m_pAreaStyle));
}

enumXFStyle XFDrawStyle::GetStyleFamily()
{
    return enumXFStyleGraphics;
}

/**************************************************************************
 *************************************************************************/
void    XFDrawStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();

    pAttrList->AddAttribute( u"style:name"_ustr, GetStyleName() );
    pAttrList->AddAttribute( u"style:family"_ustr, u"graphics"_ustr );
    pStrm->StartElement( u"style:style"_ustr );

    pAttrList->Clear();

    pAttrList->AddAttribute( u"style:run-through"_ustr, u"foreground"_ustr );
    pAttrList->AddAttribute( u"style:wrap"_ustr, u"none"_ustr );

    //line style:
    if( m_pLineStyle )
    {
        //1. if is a dash style, register the stroke-dash style first.
        if( !m_pLineStyle->IsSolid() )
        {
//          pAttrList->Clear();
            pAttrList->AddAttribute( u"draw:stroke"_ustr, u"dash"_ustr );
            pAttrList->AddAttribute( u"draw:stroke-dash"_ustr, m_pLineStyle->GetStyleName() );
        }
        else
        {
            pAttrList->AddAttribute( u"draw:stroke"_ustr, u"solid"_ustr );
        }

        if( m_pLineStyle->GetWidth() > 0 )
            pAttrList->AddAttribute( u"svg:stroke-width"_ustr, OUString::number(m_pLineStyle->GetWidth()) + "cm" );

        XFColor color = m_pLineStyle->GetColor();
        if( color.IsValid() )
            pAttrList->AddAttribute( u"svg:stroke-color"_ustr, color.ToString() );

        if( m_pLineStyle->GetTransparency()>0 )
            pAttrList->AddAttribute( u"svg:stroke-opacity"_ustr, OUString::number(m_pLineStyle->GetTransparency()) + "%" );
    }
    else
        pAttrList->AddAttribute( u"draw:stroke"_ustr, u"none"_ustr );
    //area style:
    if( m_pAreaStyle )
    {
        if( enumXFAreaSolid == m_pAreaStyle->GetAreaStyle() )
        {
            pAttrList->AddAttribute( u"draw:fill"_ustr, u"solid"_ustr );
            XFColor color = m_pAreaStyle->GetBackColor();
            if( color.IsValid() )
                pAttrList->AddAttribute( u"draw:fill-color"_ustr, color.ToString() );
        }
        else if( enumXFAreaHatch == m_pAreaStyle->GetAreaStyle() )
        {
            pAttrList->AddAttribute( u"draw:fill"_ustr, u"hatch"_ustr );
            pAttrList->AddAttribute( u"draw:fill-hatch-name"_ustr, m_pAreaStyle->GetStyleName() );
            XFColor color = m_pAreaStyle->GetBackColor();
            if( color.IsValid() )
            {
                pAttrList->AddAttribute( u"draw:fill-hatch-solid"_ustr, u"true"_ustr );
                pAttrList->AddAttribute( u"draw:fill-color"_ustr, color.ToString() );
            }
        }
    }
    else
        pAttrList->AddAttribute( u"draw:fill"_ustr, u"none"_ustr );

    if( !m_strArrowStart.isEmpty() )
    {
        pAttrList->AddAttribute( u"draw:marker-start"_ustr, m_strArrowStart );
        pAttrList->AddAttribute( u"draw:marker-start-width"_ustr, OUString::number(m_fArrowStartSize)+"cm" );
        if( m_bArrowStartCenter )
            pAttrList->AddAttribute( u"draw:draw:marker-start-center"_ustr, u"true"_ustr  );
    }
    if( !m_strArrowEnd.isEmpty() )
    {
        pAttrList->AddAttribute( u"draw:marker-end"_ustr, m_strArrowEnd );
        pAttrList->AddAttribute( u"draw:marker-end-width"_ustr, OUString::number(m_fArrowEndSize)+"cm" );
        if( m_bArrowEndCenter )
            pAttrList->AddAttribute( u"draw:draw:marker-end-center"_ustr, u"true"_ustr  );
    }
    if (m_pFontWorkStyle)
    {
        // style
        OUString aStr = u""_ustr;
        switch (m_pFontWorkStyle->GetStyleType())
        {
        default: // fall through!
        case enumXFFWOff:
            break;
        case enumXFFWRotate:
            aStr = "rotate";
            break;
        case enumXFFWUpright:
            aStr = "upright";
            break;
        case enumXFFWSlantX:
            aStr = "slant-x";
            break;
        case enumXFFWSlantY:
            aStr = "slant-y";
            break;
        }
        if (!aStr.isEmpty())
        {
            pAttrList->AddAttribute(u"draw:fontwork-style"_ustr, aStr);
        }

        //adjust
        aStr.clear();
        switch (m_pFontWorkStyle->GetAdjustType())
        {
        default: // fall through
        case enumXFFWAdjustAutosize:
            aStr = "autosize";
            break;
        case enumXFFWAdjustLeft:
            aStr = "left";
            break;
        case enumXFFWAdjustCenter:
            aStr = "center";
            break;
        case enumXFFWAdustRight:
            aStr = "right";
            break;
        }
        if (!aStr.isEmpty())
        {
            pAttrList->AddAttribute(u"draw:fontwork-adjust"_ustr, aStr);
        }

        //form
        aStr.clear();
        switch (m_pFontWorkStyle->GetButtonForm())
        {
        default: // fall through!
        case -1:
            break;
        case 4:
            aStr = "top-arc";
            break;
        case 5:
            aStr = "bottom-arc";
            break;
        }
        if (!aStr.isEmpty())
        {
            pAttrList->AddAttribute(u"draw:fontwork-form"_ustr, aStr);
        }

        // distance
        //pAttrList->AddAttribute("draw:fontwork-distance",
        //  OUString::number(m_pFontWorkStyle->GetFWDistance())+"cm");
    }

    pStrm->StartElement( u"style:properties"_ustr );

    pStrm->EndElement( u"style:properties"_ustr );

    pStrm->EndElement( u"style:style"_ustr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
