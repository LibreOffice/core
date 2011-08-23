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
/*************************************************************************
 * Change History
 * 2004-2-21 create this file.
 ************************************************************************/
#include	"xfdrawstyle.hxx"
#include	"xfdrawlinestyle.hxx"
#include	"xfdrawareastyle.hxx"
#include	"xfstylemanager.hxx"
#include "xffontworkstyle.hxx"
#include "../lwpglobalmgr.hxx"
XFDrawStyle::XFDrawStyle()
{
    m_eWrap = enumXFWrapNone;
    m_nWrapLines = 0;	//not limited.

    m_pLineStyle = NULL;
    m_pAreaStyle = NULL;

    m_pFontWorkStyle = NULL;

    m_fArrowStartSize = 0.3;
    m_fArrowEndSize = 0.3;
    m_bArrowStartCenter = sal_False;
    m_bArrowStartCenter = sal_False;
}

XFDrawStyle::~XFDrawStyle()
{
    //don't delete m_pLineStyle, it was managed by XFStyleManager.
    if (m_pFontWorkStyle)
    {
        delete m_pFontWorkStyle;
        m_pFontWorkStyle = NULL;
    }
}

void	XFDrawStyle::SetLineStyle(double width, XFColor color, sal_Int32 transparency)
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

void	XFDrawStyle::SetLineDashStyle(enumXFLineStyle style, int num1, int num2, double len1, double len2, double space )
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

void	XFDrawStyle::SetAreaColor(XFColor& color)
{
    if( !m_pAreaStyle )
    {
        m_pAreaStyle = new XFDrawAreaStyle();
        m_pAreaStyle->SetStyleName( XFGlobal::GenAreaName());
    }
    m_pAreaStyle->SetBackColor(color);
}

void	XFDrawStyle::SetAreaLineStyle(enumXFAreaLineStyle style, sal_Int32 angle, double space, XFColor lineColor)
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

enumXFStyle	XFDrawStyle::GetStyleFamily()
{
    return enumXFStyleGraphics;
}

/**************************************************************************
 *************************************************************************/
void	XFDrawStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();

    pAttrList->AddAttribute( A2OUSTR("style:name"), GetStyleName() );
    pAttrList->AddAttribute( A2OUSTR("style:family"), A2OUSTR("graphics") );
    pStrm->StartElement( A2OUSTR("style:style") );

    pAttrList->Clear();

    if( m_eWrap == enumXFWrapBackground )
    {
        pAttrList->AddAttribute( A2OUSTR("style:run-through"), A2OUSTR("background") );
    }
    else
    {
        pAttrList->AddAttribute( A2OUSTR("style:run-through"), A2OUSTR("foreground") );

        if( m_eWrap == enumXFWrapNone )
            pAttrList->AddAttribute( A2OUSTR("style:wrap"), A2OUSTR("none") );
        else if( m_eWrap == enumXFWrapLeft )
            pAttrList->AddAttribute( A2OUSTR("style:wrap"), A2OUSTR("left") );
        else if( m_eWrap == enumXFWrapRight )
            pAttrList->AddAttribute( A2OUSTR("style:wrap"), A2OUSTR("right") );
        else if( m_eWrap == enumXFWrapParallel )
            pAttrList->AddAttribute( A2OUSTR("style:wrap"), A2OUSTR("parallel") );
        else if( m_eWrap == enumXFWrapRunThrough )
            pAttrList->AddAttribute( A2OUSTR("style:wrap"), A2OUSTR("run-through") );
        else if( m_eWrap == enumXFWrapBest )
            pAttrList->AddAttribute( A2OUSTR("style:wrap"), A2OUSTR("dynamic") );
    }

    //line style:
    if( m_pLineStyle )
    {
        //1. if is a dash style, register the stroke-dash style first.
        if( !m_pLineStyle->IsSolid() )
        {
//			pAttrList->Clear();
            pAttrList->AddAttribute( A2OUSTR("draw:stroke"), A2OUSTR("dash") );
            pAttrList->AddAttribute( A2OUSTR("draw:stroke-dash"), m_pLineStyle->GetStyleName() );
        }
        else
        {
            pAttrList->AddAttribute( A2OUSTR("draw:stroke"), A2OUSTR("solid") );
        }

        if( m_pLineStyle->GetWidth() > 0 )
            pAttrList->AddAttribute( A2OUSTR("svg:stroke-width"), DoubleToOUString(m_pLineStyle->GetWidth()) + A2OUSTR("cm") );

        XFColor color = m_pLineStyle->GetColor();
        if( color.IsValid() )
            pAttrList->AddAttribute( A2OUSTR("svg:stroke-color"), color.ToString() );

        if( m_pLineStyle->GetTransparency()>0 )
            pAttrList->AddAttribute( A2OUSTR("svg:stroke-opacity"), Int32ToOUString(m_pLineStyle->GetTransparency()) + A2OUSTR("%") );
    }
    else
        pAttrList->AddAttribute( A2OUSTR("draw:stroke"), A2OUSTR("none") );
    //area style:
    if( m_pAreaStyle )
    {
        if( enumXFAreaSolid == m_pAreaStyle->GetAreaStyle() )
        {
            pAttrList->AddAttribute( A2OUSTR("draw:fill"), A2OUSTR("solid") );
            XFColor	color = m_pAreaStyle->GetBackColor();
            if( color.IsValid() )
                pAttrList->AddAttribute( A2OUSTR("draw:fill-color"), color.ToString() );
        }
        else if( enumXFAreaHatch == m_pAreaStyle->GetAreaStyle() )
        {
            pAttrList->AddAttribute( A2OUSTR("draw:fill"), A2OUSTR("hatch") );
            pAttrList->AddAttribute( A2OUSTR("draw:fill-hatch-name"), m_pAreaStyle->GetStyleName() );
            XFColor	color = m_pAreaStyle->GetBackColor();
            if( color.IsValid() )
            {
                pAttrList->AddAttribute( A2OUSTR("draw:fill-hatch-solid"), A2OUSTR("true") );
                pAttrList->AddAttribute( A2OUSTR("draw:fill-color"), color.ToString() );
            }
        }
    }
    else
        pAttrList->AddAttribute( A2OUSTR("draw:fill"), A2OUSTR("none") );

    if( m_strArrowStart.getLength()>0 )
    {
        pAttrList->AddAttribute( A2OUSTR("draw:marker-start"), m_strArrowStart );
        pAttrList->AddAttribute( A2OUSTR("draw:marker-start-width"), DoubleToOUString(m_fArrowStartSize)+A2OUSTR("cm") );
        if( m_bArrowStartCenter )
            pAttrList->AddAttribute( A2OUSTR("draw:draw:marker-start-center"), A2OUSTR("true")  );
    }
    if( m_strArrowEnd.getLength()>0 )
    {
        pAttrList->AddAttribute( A2OUSTR("draw:marker-end"), m_strArrowEnd );
        pAttrList->AddAttribute( A2OUSTR("draw:marker-end-width"), DoubleToOUString(m_fArrowEndSize)+A2OUSTR("cm") );
        if( m_bArrowEndCenter )
            pAttrList->AddAttribute( A2OUSTR("draw:draw:marker-end-center"), A2OUSTR("true")  );
    }
    if (m_pFontWorkStyle)
    {
        // style
        rtl::OUString aStr = A2OUSTR("");
        switch (m_pFontWorkStyle->GetStyleType())
        {
        default: // fall through!
        case enumXFFWOff:
            break;
        case enumXFFWRotate:
            aStr = A2OUSTR("rotate");
            break;
        case enumXFFWUpright:
            aStr = A2OUSTR("upright");
            break;
        case enumXFFWSlantX:
            aStr = A2OUSTR("slant-x");
            break;
        case enumXFFWSlantY:
            aStr = A2OUSTR("slant-y");
            break;
        }
        if (aStr.getLength() > 0)
        {
            pAttrList->AddAttribute(A2OUSTR("draw:fontwork-style"), aStr);
        }

        //adjust
        aStr = A2OUSTR("");
        switch (m_pFontWorkStyle->GetAdjustType())
        {
        default: // fall througth
        case enumXFFWAdjustAutosize:
            aStr = A2OUSTR("autosize");
            break;
        case enumXFFWAdjustLeft:
            aStr = A2OUSTR("left");
            break;
        case enumXFFWAdjustCenter:
            aStr = A2OUSTR("center");
            break;
        case enumXFFWAdustRight:
            aStr = A2OUSTR("right");
            break;
        }
        if (aStr.getLength() > 0)
        {
            pAttrList->AddAttribute(A2OUSTR("draw:fontwork-adjust"), aStr);
        }

        //form
        aStr = A2OUSTR("");
        switch (m_pFontWorkStyle->GetButtonForm())
        {
        default: // fall through!
        case -1:
            break;
        case 4:
            aStr = A2OUSTR("top-arc");
            break;
        case 5:
            aStr = A2OUSTR("bottom-arc");
            break;
        }
        if (aStr.getLength() > 0)
        {
            pAttrList->AddAttribute(A2OUSTR("draw:fontwork-form"), aStr);
        }

        // distance
        //pAttrList->AddAttribute(A2OUSTR("draw:fontwork-distance"),
        //	DoubleToOUString(m_pFontWorkStyle->GetFWDistance())+A2OUSTR("cm"));
    }

    pStrm->StartElement( A2OUSTR("style:properties") );

    pStrm->EndElement( A2OUSTR("style:properties") );

    pStrm->EndElement( A2OUSTR("style:style") );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
