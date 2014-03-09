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
 * Image style object.
 ************************************************************************/
#include "xfimagestyle.hxx"
#include "xfborders.hxx"
#include "xfshadow.hxx"

XFImageStyle::XFImageStyle()
{
    m_nBrightness = 0;
    m_nContrast = 0;
    m_nGamma = 0;
    m_nTransparent = 0;
    m_nAdjustRed = 0;
    m_nAdjustGreen = 0;
    m_nAdjustBlue = 0;
    m_fClipLeft = 0;
    m_fClipRight = 0;
    m_fClipTop = 0;
    m_fClipBottom = 0;
    m_bHoriFlip = sal_False;
    m_bVertFlip = sal_False;
    m_eColorMode = enumXFColorStandard;
}

void XFImageStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    pAttrList->AddAttribute( "style:name", GetStyleName() );
    if( !GetParentStyleName().isEmpty() )
        pAttrList->AddAttribute("style:parent-style-name",GetParentStyleName());
    pAttrList->AddAttribute( "style:family", "graphics" );
    //parent style name ignore now.
    pStrm->StartElement( "style:style" );

    m_aMargins.ToXml(pStrm);

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
    //background
    if( m_aBackColor.IsValid() )
        pAttrList->AddAttribute( "fo:background-color", m_aBackColor.ToString() );
    //pad
    m_aPad.ToXml(pStrm);
    //margin:
    m_aMargins.ToXml(pStrm);
    //flip
    if( m_bHoriFlip || m_bVertFlip )
    {
        if( m_bHoriFlip && m_bVertFlip )
            pAttrList->AddAttribute( "style:mirror", "horizontal-on-right-pages vertical" );
        else if( m_bHoriFlip && !m_bVertFlip )
            pAttrList->AddAttribute( "style:mirror", "horizontal-on-right-pages" );
        else if( !m_bHoriFlip && m_bVertFlip )
            pAttrList->AddAttribute( "style:mirror", "vertical" );
    }
    //color adjust
    if( m_nAdjustRed )
        pAttrList->AddAttribute( "draw:red", OUString::number(m_nAdjustRed) + "%" );
    if( m_nAdjustGreen )
        pAttrList->AddAttribute( "draw:green", OUString::number(m_nAdjustGreen) + "%" );
    if( m_nAdjustBlue )
        pAttrList->AddAttribute( "draw:blue", OUString::number(m_nAdjustBlue) + "%" );

    if( m_nGamma )
        pAttrList->AddAttribute( "draw:gamma", OUString::number(m_nGamma) );
    if( m_nBrightness )
        pAttrList->AddAttribute( "draw:luminance", OUString::number(m_nBrightness) + "%" );
    if( m_nContrast )
        pAttrList->AddAttribute( "draw:contrast", OUString::number(m_nContrast) + "%" );

    if( m_nTransparent )
        pAttrList->AddAttribute( "draw:transparency", OUString::number(m_nTransparent) + "%" );

    pAttrList->AddAttribute("draw:color-mode", GetColorMode(m_eColorMode));
    //border
    if( m_pBorders )
        m_pBorders->ToXml(pStrm);
    //shadow
    if( m_pShadow )
        m_pShadow->ToXml(pStrm);

    if( m_bPrintable )
        pAttrList->AddAttribute( "style:print-content", "true" );
    //protect:
    if( m_bProtectContent || m_bProtectSize || m_bProtectPos )
    {
        OUString protect;
        if( m_bProtectContent )
            protect += "content";
        if( m_bProtectSize )
        {
            if( !protect.isEmpty() )
                protect += " ";
            protect += "size";
        }
        if( m_bProtectPos )
        {
            if( !protect.isEmpty() )
                protect += " ";
            protect += "position";
        }
        pAttrList->AddAttribute( "style:protect", protect );
    }
    //vertical pos and horizon pos:
    pAttrList->AddAttribute( "style:vertical-pos", GetFrameYPos(m_eYPos) );
    pAttrList->AddAttribute( "style:vertical-rel", GetFrameYRel(m_eYRel) );
    pAttrList->AddAttribute( "style:horizontal-pos", GetFrameXPos(m_eXPos) );
    pAttrList->AddAttribute( "style:horizontal-rel", GetFrameXRel(m_eXRel) );

    //clip:
    if( FABS(m_fClipLeft)>FLOAT_MIN || FABS(m_fClipRight)>FLOAT_MIN || FABS(m_fClipTop)>FLOAT_MIN || FABS(m_fClipBottom)>FLOAT_MIN )
    {
        OUString clip = "rect(" + OUString::number(m_fClipTop) + "cm " +
            OUString::number(m_fClipRight) + "cm " +
            OUString::number(m_fClipBottom) + "cm " +
            OUString::number(m_fClipLeft) + "cm" + ")";
        pAttrList->AddAttribute("fo:clip",clip);
    }

    pStrm->StartElement( "style:properties" );
    pStrm->EndElement( "style:properties" );

    pStrm->EndElement( "style:style" );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
