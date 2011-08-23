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
/*************************************************************************
 * Change History
 * 2005-01-17 create this file.
 ************************************************************************/
#ifndef		_XFPAGEMASTER_HXX
#define		_XFPAGEMASTER_HXX

#include	"xfglobal.hxx"
#include	"xfstyle.hxx"
#include	"xfcolor.hxx"
#include	"xfmargins.hxx"

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

    virtual ~XFPageMaster();

public:
    void	SetPageWidth(double width);

    void	SetPageHeight(double height);

    void	SetMargins(double left=-1, double right=-1,double top=-1, double bottom=-1);

    void	SetPageUsage(enumXFPageUsage usage);

    void	SetPrintOrient(sal_Bool bUseWidth=sal_True);

    void	SetBorders(XFBorders *pBorders);

    void	SetShadow(XFShadow *pShadow);

    void	SetBackColor(XFColor color);

    void	SetBackImage(XFBGImage *image);

    void	SetColumns(XFColumns *pColumns);

    void	SetHeaderStyle(XFHeaderStyle *pHeaderStyle);

    void	SetFooterStyle(XFFooterStyle *pFooterStyle);

    void	SetTextDir(enumXFTextDir dir);

    void	SetFootNoteSeparator(enumXFAlignType align = enumXFAlignStart,
                                double width = 0.05,
                                sal_Int32 lengthPercent = 25,
                                double spaceAbove = 0.1,
                                double spaceBelow = 0.1,
                                XFColor color=XFColor(0,0,0)
                                );

    virtual enumXFStyle	GetStyleFamily();

    virtual void	ToXml(IXFStream *pStream);

private:
    double	m_fPageWidth;
    double	m_fPageHeight;

    XFMargins	m_aMargin;

    enumXFPageUsage m_eUsage;
    enumXFTextDir	m_eTextDir;
    sal_Bool	m_bPrintOrient;

    XFBorders	*m_pBorders;
    XFShadow	*m_pShadow;
    XFColor		m_aBackColor;
    XFColumns	*m_pColumns;
    XFBGImage	*m_pBGImage;

    XFHeaderStyle	*m_pHeaderStyle;
    XFFooterStyle	*m_pFooterStyle;
    //seperator:
    enumXFAlignType	m_eSepAlign;
    double	m_fSepWidth;
    XFColor	m_aSepColor;
    double	m_fSepSpaceAbove;
    double	m_fSepSpaceBelow;
    sal_Int32	m_nSepLengthPercent;

};

inline void	XFPageMaster::SetPageUsage(enumXFPageUsage usage)
{
    this->m_eUsage = usage;
}

inline void	XFPageMaster::SetPrintOrient(sal_Bool bUseWidth)
{
    m_bPrintOrient = bUseWidth;
}

inline void	XFPageMaster::SetTextDir(enumXFTextDir dir)
{
    m_eTextDir = dir;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
