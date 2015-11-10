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
#include "xfdrawstyle.hxx"
#include "xfdrawlinestyle.hxx"
#include "xfdrawareastyle.hxx"
#include "xfstylemanager.hxx"
#include "xffontworkstyle.hxx"
#include "../lwpglobalmgr.hxx"
XFDrawStyle::XFDrawStyle()
{
    m_eWrap = enumXFWrapNone;
    m_nWrapLines = 0;   //not limited.

    m_pLineStyle = nullptr;
    m_pAreaStyle = nullptr;

    m_pFontWorkStyle = nullptr;

    m_fArrowStartSize = 0.3;
    m_fArrowEndSize = 0.3;
    m_bArrowStartCenter = false;
    m_bArrowEndCenter = false;
}

XFDrawStyle::~XFDrawStyle()
{
    //don't delete m_pLineStyle, it was managed by XFStyleManager.
    if (m_pFontWorkStyle)
    {
        delete m_pFontWorkStyle;
        m_pFontWorkStyle = nullptr;
    }
}

void    XFDrawStyle::SetLineStyle(double width, XFColor color, sal_Int32 transparency)
{
    if( !m_pLineStyle )
    {
        m_pLineStyle = new XFDrawLineStyle();
        m_pLineStyle->SetStyleName( XFGlobal::GenStrokeDashName());
    }
    m_pLineStyle->SetWidth(width);
    m_pLineStyle->SetColor(color);
    m_pLineStyle->SetTransparency(transparency);
}

void    XFDrawStyle::SetLineDashStyle(enumXFLineStyle style, int num1, int num2, double len1, double len2, double space )
{
    if( !m_pLineStyle )
    {
        m_pLineStyle = new XFDrawLineStyle();
        m_pLineStyle->SetStyleName( XFGlobal::GenStrokeDashName());
    }
    m_pLineStyle->SetLineStyle(style);
    m_pLineStyle->SetDot1Number(num1);
    m_pLineStyle->SetDot1Length(len1);
    m_pLineStyle->SetDot2Number(num2);
    m_pLineStyle->SetDot2Length(len2);
    m_pLineStyle->SetSpace(space);
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    pXFStyleManager->AddStyle(m_pLineStyle);
}

void XFDrawStyle::SetFontWorkStyle(sal_Int8 nForm, enumXFFWStyle eStyle, enumXFFWAdjust eAdjust)
{
    if (!m_pFontWorkStyle)
    {
        m_pFontWorkStyle = new XFFontWorkStyle();
    }

    m_pFontWorkStyle->SetButtonForm(nForm);
    m_pFontWorkStyle->SetFWStyleType(eStyle);
    m_pFontWorkStyle->SetFWAdjustType(eAdjust);
}

void    XFDrawStyle::SetAreaColor(XFColor& color)
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
    pXFStyleManager->AddStyle(m_pAreaStyle);
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

    pAttrList->AddAttribute( "style:name", GetStyleName() );
    pAttrList->AddAttribute( "style:family", "graphics" );
    pStrm->StartElement( "style:style" );

    pAttrList->Clear();

    if( m_eWrap == enumXFWrapBackground )
    {
        pAttrList->AddAttribute( "style:run-through", "background" );
    }
    else
    {
        pAttrList->AddAttribute( "style:run-through", "foreground" );

        if( m_eWrap == enumXFWrapNone )
            pAttrList->AddAttribute( "style:wrap", "none" );
        else if( m_eWrap == enumXFWrapLeft )
            pAttrList->AddAttribute( "style:wrap", "left" );
        else if( m_eWrap == enumXFWrapRight )
            pAttrList->AddAttribute( "style:wrap", "right" );
        else if( m_eWrap == enumXFWrapParallel )
            pAttrList->AddAttribute( "style:wrap", "parallel" );
        else if( m_eWrap == enumXFWrapRunThrough )
            pAttrList->AddAttribute( "style:wrap", "run-through" );
        else if( m_eWrap == enumXFWrapBest )
            pAttrList->AddAttribute( "style:wrap", "dynamic" );
    }

    //line style:
    if( m_pLineStyle )
    {
        //1. if is a dash style, register the stroke-dash style first.
        if( !m_pLineStyle->IsSolid() )
        {
//          pAttrList->Clear();
            pAttrList->AddAttribute( "draw:stroke", "dash" );
            pAttrList->AddAttribute( "draw:stroke-dash", m_pLineStyle->GetStyleName() );
        }
        else
        {
            pAttrList->AddAttribute( "draw:stroke", "solid" );
        }

        if( m_pLineStyle->GetWidth() > 0 )
            pAttrList->AddAttribute( "svg:stroke-width", OUString::number(m_pLineStyle->GetWidth()) + "cm" );

        XFColor color = m_pLineStyle->GetColor();
        if( color.IsValid() )
            pAttrList->AddAttribute( "svg:stroke-color", color.ToString() );

        if( m_pLineStyle->GetTransparency()>0 )
            pAttrList->AddAttribute( "svg:stroke-opacity", OUString::number(m_pLineStyle->GetTransparency()) + "%" );
    }
    else
        pAttrList->AddAttribute( "draw:stroke", "none" );
    //area style:
    if( m_pAreaStyle )
    {
        if( enumXFAreaSolid == m_pAreaStyle->GetAreaStyle() )
        {
            pAttrList->AddAttribute( "draw:fill", "solid" );
            XFColor color = m_pAreaStyle->GetBackColor();
            if( color.IsValid() )
                pAttrList->AddAttribute( "draw:fill-color", color.ToString() );
        }
        else if( enumXFAreaHatch == m_pAreaStyle->GetAreaStyle() )
        {
            pAttrList->AddAttribute( "draw:fill", "hatch" );
            pAttrList->AddAttribute( "draw:fill-hatch-name", m_pAreaStyle->GetStyleName() );
            XFColor color = m_pAreaStyle->GetBackColor();
            if( color.IsValid() )
            {
                pAttrList->AddAttribute( "draw:fill-hatch-solid", "true" );
                pAttrList->AddAttribute( "draw:fill-color", color.ToString() );
            }
        }
    }
    else
        pAttrList->AddAttribute( "draw:fill", "none" );

    if( !m_strArrowStart.isEmpty() )
    {
        pAttrList->AddAttribute( "draw:marker-start", m_strArrowStart );
        pAttrList->AddAttribute( "draw:marker-start-width", OUString::number(m_fArrowStartSize)+"cm" );
        if( m_bArrowStartCenter )
            pAttrList->AddAttribute( "draw:draw:marker-start-center", "true"  );
    }
    if( !m_strArrowEnd.isEmpty() )
    {
        pAttrList->AddAttribute( "draw:marker-end", m_strArrowEnd );
        pAttrList->AddAttribute( "draw:marker-end-width", OUString::number(m_fArrowEndSize)+"cm" );
        if( m_bArrowEndCenter )
            pAttrList->AddAttribute( "draw:draw:marker-end-center", "true"  );
    }
    if (m_pFontWorkStyle)
    {
        // style
        OUString aStr = "";
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
            pAttrList->AddAttribute("draw:fontwork-style", aStr);
        }

        //adjust
        aStr.clear();
        switch (m_pFontWorkStyle->GetAdjustType())
        {
        default: // fall througth
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
            pAttrList->AddAttribute("draw:fontwork-adjust", aStr);
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
            pAttrList->AddAttribute("draw:fontwork-form", aStr);
        }

        // distance
        //pAttrList->AddAttribute("draw:fontwork-distance",
        //  OUString::number(m_pFontWorkStyle->GetFWDistance())+"cm");
    }

    pStrm->StartElement( "style:properties" );

    pStrm->EndElement( "style:properties" );

    pStrm->EndElement( "style:style" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
