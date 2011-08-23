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
/*************************************************************************
 * Change History
   2005-02-22	create and implemente.
 ************************************************************************/
#include	"xfimagestyle.hxx"
#include	"xfborders.hxx"
#include	"xfshadow.hxx"

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
    pAttrList->AddAttribute( A2OUSTR("style:name"), GetStyleName() );
    if( GetParentStyleName().getLength() > 0 )
        pAttrList->AddAttribute(A2OUSTR("style:parent-style-name"),GetParentStyleName());
    pAttrList->AddAttribute( A2OUSTR("style:family"), A2OUSTR("graphics") );
    //parent style name ignore now.
    pStrm->StartElement( A2OUSTR("style:style") );

    m_aMargins.ToXml(pStrm);

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
    //background
    if( m_aBackColor.IsValid() )
        pAttrList->AddAttribute( A2OUSTR("fo:background-color"), m_aBackColor.ToString() );
    //pad
    m_aPad.ToXml(pStrm);
    //margin:
    m_aMargins.ToXml(pStrm);
    //flip
    if( m_bHoriFlip || m_bVertFlip )
    {
        if( m_bHoriFlip && m_bVertFlip )
            pAttrList->AddAttribute( A2OUSTR("style:mirror"), A2OUSTR("horizontal-on-right-pages vertical") );
        else if( m_bHoriFlip && !m_bVertFlip )
            pAttrList->AddAttribute( A2OUSTR("style:mirror"), A2OUSTR("horizontal-on-right-pages") );
        else if( !m_bHoriFlip && m_bVertFlip )
            pAttrList->AddAttribute( A2OUSTR("style:mirror"), A2OUSTR("vertical") );
    }
    //color adjust
    if( m_nAdjustRed )
        pAttrList->AddAttribute( A2OUSTR("draw:red"), Int32ToOUString(m_nAdjustRed) + A2OUSTR("%") );
    if( m_nAdjustGreen )
        pAttrList->AddAttribute( A2OUSTR("draw:green"), Int32ToOUString(m_nAdjustGreen) + A2OUSTR("%") );
    if( m_nAdjustBlue )
        pAttrList->AddAttribute( A2OUSTR("draw:blue"), Int32ToOUString(m_nAdjustBlue) + A2OUSTR("%") );

    if( m_nGamma )
        pAttrList->AddAttribute( A2OUSTR("draw:gamma"), Int32ToOUString(m_nGamma) );
    if( m_nBrightness )
        pAttrList->AddAttribute( A2OUSTR("draw:luminance"), Int32ToOUString(m_nBrightness) + A2OUSTR("%") );
    if( m_nContrast )
        pAttrList->AddAttribute( A2OUSTR("draw:contrast"), Int32ToOUString(m_nContrast) + A2OUSTR("%") );

    if( m_nTransparent )
        pAttrList->AddAttribute( A2OUSTR("draw:transparency"), Int32ToOUString(m_nTransparent) + A2OUSTR("%") );

    pAttrList->AddAttribute(A2OUSTR("draw:color-mode"), GetColorMode(m_eColorMode));
    //border
    if( m_pBorders )
        m_pBorders->ToXml(pStrm);
    //shadow
    if( m_pShadow )
        m_pShadow->ToXml(pStrm);

    if( m_bPrintable )
        pAttrList->AddAttribute( A2OUSTR("style:print-content"), A2OUSTR("true") );
    //protect:
    if( m_bProtectContent || m_bProtectSize || m_bProtectPos )
    {
        rtl::OUString protect;
        if( m_bProtectContent )
            protect += A2OUSTR("content");
        if( m_bProtectSize )
        {
            if( protect.getLength()>0 )
                protect += A2OUSTR(" ");
            protect += A2OUSTR("size");
        }
        if( m_bProtectPos )
        {
            if( protect.getLength()>0 )
                protect += A2OUSTR(" ");
            protect += A2OUSTR("position");
        }
        pAttrList->AddAttribute( A2OUSTR("style:protect"), protect );
    }
    //vertical pos and horizon pos:
    pAttrList->AddAttribute( A2OUSTR("style:vertical-pos"), GetFrameYPos(m_eYPos) );
    pAttrList->AddAttribute( A2OUSTR("style:vertical-rel"), GetFrameYRel(m_eYRel) );
    pAttrList->AddAttribute( A2OUSTR("style:horizontal-pos"), GetFrameXPos(m_eXPos) );
    pAttrList->AddAttribute( A2OUSTR("style:horizontal-rel"), GetFrameXRel(m_eXRel) );

    //clip:
    if( FABS(m_fClipLeft)>FLOAT_MIN || FABS(m_fClipRight)>FLOAT_MIN || FABS(m_fClipTop)>FLOAT_MIN || FABS(m_fClipBottom)>FLOAT_MIN )
    {
        rtl::OUString clip = A2OUSTR("rect(");
        clip += DoubleToOUString(m_fClipTop) + A2OUSTR("cm ");
        clip += DoubleToOUString(m_fClipRight) + A2OUSTR("cm ");
        clip += DoubleToOUString(m_fClipBottom) + A2OUSTR("cm ");
        clip += DoubleToOUString(m_fClipLeft) + A2OUSTR("cm");
        clip += A2OUSTR(")");
        pAttrList->AddAttribute(A2OUSTR("fo:clip"),clip);
    }

    //
    pStrm->StartElement( A2OUSTR("style:properties") );
    pStrm->EndElement( A2OUSTR("style:properties") );

    pStrm->EndElement( A2OUSTR("style:style") );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
