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
 * Background image.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFBGIMAGE_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFBGIMAGE_HXX

#include "xfstyle.hxx"

/**
 * @brief
 * Background image object.
 */
class XFBGImage : public XFStyle
{
public:
    XFBGImage();

public:
    /**
     * @descr   Use file link as image source.
     */
    void    SetFileLink(const OUString& fileName);

    /**
     * @descr   Use base64 stream as image source.
     */
    void    SetImageData(sal_uInt8 *buf, int len);

    /**
     * @descr   Set the iamge position type.
     */
    void    SetPosition(enumXFAlignType horiAlign = enumXFAlignCenter, enumXFAlignType vertAlign = enumXFAlignCenter);

    /**
     * @descr   Set background image repeat style.
     */
    void    SetRepeate();

    /**
     * @descr   Set whether the image should be streatched to fill the owner object.
     */
    void    SetStretch();

    /**
     * @descr   decide whether there has been a style that is the same as this object.
     *          If it use base64 stream, i'll simply return sal_False because it's too slow to compare two base64 stream.
     */
    virtual bool Equal(IXFStyle *pStyle) override;

    virtual void ToXml(IXFStream *pStrm) override;

    friend bool operator==(XFBGImage& img1, XFBGImage& img2);
    friend bool operator!=(XFBGImage& img1, XFBGImage& img2);
private:
    OUString   m_strFileName;
    OUString   m_strData;
    bool       m_bUserFileLink;
    bool       m_bRepeate;
    bool       m_bStretch;
    bool       m_bPosition;
    enumXFAlignType m_eHoriAlign;
    enumXFAlignType m_eVertAlign;
};

inline void XFBGImage::SetFileLink(const OUString& fileName)
{
    m_strFileName = fileName;
    m_bUserFileLink = true;
}

inline void XFBGImage::SetPosition(enumXFAlignType horiAlign, enumXFAlignType vertAlign)
{
    m_bPosition = true;
    m_eHoriAlign = horiAlign;
    m_eVertAlign = vertAlign;

    m_bStretch = false;
    m_bRepeate = false;
}

inline void XFBGImage::SetRepeate()
{
    m_bRepeate = true;
    m_bStretch = false;
    m_bPosition = false;
}

inline void XFBGImage::SetStretch()
{
    m_bStretch = true;
    m_bRepeate = false;
    m_bPosition = false;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
