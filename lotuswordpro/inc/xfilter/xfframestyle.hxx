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
 * Frame object style for OOo.
 * You can reference to the XFFrame object.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFFRAMESTYLE_HXX
#define INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFFRAMESTYLE_HXX

#include <xfilter/xfglobal.hxx>
#include <xfilter/xfstyle.hxx>
#include <xfilter/xfmargins.hxx>
#include <xfilter/xfcolor.hxx>
#include <xfilter/xfpadding.hxx>
#include <memory>

class XFBorders;
class XFColumns;
class XFShadow;
class XFBGImage;

/**
 * @brief
 * Style for all frame objects.
 */
class XFFrameStyle : public XFStyle
{
public:
    XFFrameStyle();

    virtual ~XFFrameStyle() override;

public:
    void    SetWrapType(enumXFWrap wrap);

    /**
     * @descr:  space between frame and paragraph text.
     */
    void    SetMargins(double left, double right, double top, double bottom);

    /**
     * @descr:  space between frame and text inside frame.
     */
    void    SetPadding(double left, double right, double top, double bottom);

    /**
     * @descr:  set the border property of the frame.
     */
    void    SetBorders(std::unique_ptr<XFBorders> pBorders);

    /**
     * @descr:  set the column property of the frame.
     */
    void    SetColumns(XFColumns *pColumns);

    /**
     * @descr:  set the shadow object the frame.
     */
    void    SetShadow(XFShadow *pShadow);

    /**
     * @descr:  set the background image of the frame.
     */
    void    SetBackImage(std::unique_ptr<XFBGImage>& rImage);

    /**
     * @descr:  set the background color of the frame.
     */
    void    SetBackColor(XFColor const & color);

    /**
     * @descr   Set whether frame is protected. There are three properties that can be protected, content,size and position.
     */
    void    SetProtect(bool content, bool size, bool pos);

    /**
     * @descr   Set text dir.
     */
    void    SetTextDir(enumXFTextDir dir);

    /**
     * @descr   Set horizontal position type.
     */
    void    SetXPosType(enumXFFrameXPos pos, enumXFFrameXRel rel);

    /**
     * @descr   Set vertical position type.
     */
    void    SetYPosType(enumXFFrameYPos pos, enumXFFrameYRel rel);

    void SetTransparency(sal_Int16 nTransparency);

    virtual enumXFStyle GetStyleFamily() override;

    virtual void    ToXml(IXFStream *pStrm) override;

protected:
    enumXFWrap  m_eWrap;
    XFPadding   m_aPad;
    XFMargins   m_aMargins;
    std::unique_ptr<XFBorders> m_pBorders;
    std::unique_ptr<XFColumns> m_pColumns;
    std::unique_ptr<XFShadow>  m_pShadow;
    std::unique_ptr<XFBGImage> m_pBGImage;
    XFColor     m_aBackColor;
    bool        m_bProtectContent;
    bool        m_bProtectSize;
    bool        m_bProtectPos;
    sal_Int16   m_nTransparency;

    enumXFTextDir   m_eTextDir;
    enumXFFrameXPos m_eXPos;
    enumXFFrameXRel m_eXRel;
    enumXFFrameYPos m_eYPos;
    enumXFFrameYRel m_eYRel;
};

inline void XFFrameStyle::SetWrapType(enumXFWrap wrap)
{
    m_eWrap = wrap;
}

inline void XFFrameStyle::SetMargins(double left, double right,double top, double bottom)
{
    if( left != -1 )
        m_aMargins.SetLeft(left);
    if( right != -1 )
        m_aMargins.SetRight(right);
    if( top != -1 )
        m_aMargins.SetTop(top);
    if( bottom != -1 )
        m_aMargins.SetBottom( bottom );
}

inline void XFFrameStyle::SetPadding(double left, double right,double top, double bottom)
{
    if( left != -1 )
        m_aPad.SetLeft(left);
    if( right != -1 )
        m_aPad.SetRight(right);
    if( top != -1 )
        m_aPad.SetTop(top);
    if( bottom != -1 )
        m_aPad.SetBottom( bottom );
}

inline void XFFrameStyle::SetBackColor(XFColor const & color)
{
    m_aBackColor = color;
}

inline void XFFrameStyle::SetTextDir(enumXFTextDir dir)
{
    m_eTextDir = dir;
}

inline void XFFrameStyle::SetProtect(bool content, bool size, bool pos)
{
    m_bProtectContent = content;
    m_bProtectSize = size;
    m_bProtectPos = pos;
}

inline void XFFrameStyle::SetXPosType(enumXFFrameXPos pos, enumXFFrameXRel rel)
{
    m_eXPos = pos;
    m_eXRel = rel;
}

inline void XFFrameStyle::SetYPosType(enumXFFrameYPos pos, enumXFFrameYRel rel)
{
    m_eYPos = pos;
    m_eYRel = rel;
}

inline void XFFrameStyle::SetTransparency(sal_Int16 nTransparency)
{
    m_nTransparency = nTransparency;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
