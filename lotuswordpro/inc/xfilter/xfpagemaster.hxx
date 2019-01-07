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
 * Page master used bye XFMasterPage.
 * It is the real object to define header and footer of pages.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFPAGEMASTER_HXX
#define INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFPAGEMASTER_HXX

#include <xfilter/xfglobal.hxx>
#include <xfilter/xfstyle.hxx>
#include <xfilter/xfcolor.hxx>
#include <xfilter/xfmargins.hxx>
#include <memory>

class XFBorders;
class XFShadow;
class XFColumns;
class XFHeaderStyle;
class XFFooterStyle;
class XFBGImage;

class XFPageMaster : public XFStyle
{
public:
    XFPageMaster();

    virtual ~XFPageMaster() override;

public:
    void    SetPageWidth(double width);

    void    SetPageHeight(double height);

    void    SetMargins(double left, double right, double top, double bottom);

    void    SetPageUsage(enumXFPageUsage usage);

    void    SetBorders(std::unique_ptr<XFBorders> pBorders);

    void    SetShadow(XFShadow *pShadow);

    void    SetBackColor(XFColor color);

    void    SetBackImage(std::unique_ptr<XFBGImage>& rImage);

    void    SetColumns(XFColumns *pColumns);

    void    SetHeaderStyle(XFHeaderStyle *pHeaderStyle);

    void    SetFooterStyle(XFFooterStyle *pFooterStyle);

    void    SetTextDir(enumXFTextDir dir);

    void    SetFootNoteSeparator(enumXFAlignType align,
                                double width,
                                sal_Int32 lengthPercent,
                                double spaceAbove,
                                double spaceBelow,
                                XFColor color
                                );

    virtual enumXFStyle GetStyleFamily() override;

    virtual void    ToXml(IXFStream *pStream) override;

private:
    double  m_fPageWidth;
    double  m_fPageHeight;

    XFMargins   m_aMargin;

    enumXFPageUsage m_eUsage;
    enumXFTextDir   m_eTextDir;

    std::unique_ptr<XFBorders>   m_pBorders;
    std::unique_ptr<XFShadow>    m_pShadow;
    XFColor     m_aBackColor;
    std::unique_ptr<XFColumns>   m_pColumns;
    std::unique_ptr<XFBGImage>   m_pBGImage;

    std::unique_ptr<XFHeaderStyle> m_pHeaderStyle;
    std::unique_ptr<XFFooterStyle> m_pFooterStyle;
    //separator:
    enumXFAlignType m_eSepAlign;
    double  m_fSepWidth;
    XFColor m_aSepColor;
    double  m_fSepSpaceAbove;
    double  m_fSepSpaceBelow;
    sal_Int32   m_nSepLengthPercent;

};

inline void XFPageMaster::SetPageUsage(enumXFPageUsage usage)
{
    m_eUsage = usage;
}

inline void XFPageMaster::SetTextDir(enumXFTextDir dir)
{
    m_eTextDir = dir;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
