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
 * Header style,exist in page-master object.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFHEADERSTYLE_HXX
#define INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFHEADERSTYLE_HXX

#include <xfilter/xfstyle.hxx>
#include <xfilter/xfmargins.hxx>
#include <xfilter/xfborders.hxx>
#include <xfilter/xfpadding.hxx>
#include <xfilter/xfshadow.hxx>
#include <memory>

/**
 * @brief
 * Header style object.
 */
class XFBGImage;
class XFHeaderStyle : public XFStyle
{
public:
    explicit XFHeaderStyle(bool isFooter=false);

    virtual ~XFHeaderStyle() override;

public:
    /**
     * @descr   Set margins for header style.
     */
    void    SetMargins(double left, double right, double bottom);

    void    SetDynamicSpace(bool dynamic);

    /**
     * @descr   Set header fixed height.
     */
    void    SetHeight(double height);

    /**
     * @descr   Set header min height.
     */
    void    SetMinHeight(double minHeight);

    /**
     * @descr   Set header shadow.
     */
    void    SetShadow(XFShadow *pShadow);

    /**
     * @descr   Set header borders.
     */
    void    SetBorders(std::unique_ptr<XFBorders> pBorders);

    void    SetBackImage(std::unique_ptr<XFBGImage>& rImage);

    /**
     * @descr   Set header background color.
     */
    void    SetBackColor(XFColor color);

    /**
     * @descr   Output header style object.
     */
    virtual void    ToXml(IXFStream *pStrm) override;

protected:
    bool    m_bIsFooter;
    bool    m_bDynamicSpace;
    double      m_fHeight;
    double      m_fMinHeight;
    XFMargins   m_aMargin;
    std::unique_ptr<XFShadow>  m_pShadow;
    XFPadding   m_aPadding;
    std::unique_ptr<XFBorders> m_pBorders;
    std::unique_ptr<XFBGImage> m_pBGImage;
    XFColor     m_aBackColor;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
