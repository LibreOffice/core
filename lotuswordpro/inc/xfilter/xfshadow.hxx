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
 * Shadow object,now only used by paragraph object.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFSHADOW_HXX
#define INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFSHADOW_HXX

#include <xfilter/xfglobal.hxx>
#include <xfilter/xfcolor.hxx>

/**
 * @brief
 * Shadow object.
 */
class XFShadow
{
public:
    XFShadow();

public:
    /**
     * @descr   set shadow position. You can refer to enumXFShadowPos to get all possible positions.
     */
    void    SetPosition(enumXFShadowPos pos);

    /**
     * @descr   return shadow position.
     */
    enumXFShadowPos GetPosition() const;

    /**
     * @descr   Set shadow offset from owner object.
     */
    void    SetOffset(double offset);

    /**
     * @descr   return shadow offset.
     */
    double  GetOffset() const;

    /**
     * @descr   Set shadow color.
     */
    void    SetColor(const XFColor& color);

    /**
     * @descr   return shadow color.
     */
    const XFColor& GetColor() const;

    OUString   ToString();

    void    ToXml(IXFStream *pStrm);

    friend bool operator==(XFShadow const & s1, XFShadow const & s2);
    friend bool operator!=(XFShadow const & s1, XFShadow const & s2);
private:
    enumXFShadowPos m_ePosition;
    double          m_fOffset;
    XFColor         m_aColor;
};

inline void XFShadow::SetPosition(enumXFShadowPos pos)
{
    m_ePosition = pos;
}

inline enumXFShadowPos XFShadow::GetPosition() const
{
    return m_ePosition;
}

inline void XFShadow::SetOffset(double offset)
{
    m_fOffset = offset;
}

inline double   XFShadow::GetOffset() const
{
    return m_fOffset;
}

inline void XFShadow::SetColor(const XFColor& color)
{
    m_aColor = color;
}

inline const XFColor& XFShadow::GetColor() const
{
    return m_aColor;
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
