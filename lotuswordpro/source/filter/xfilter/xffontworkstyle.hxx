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
 * Style of Fontwork object.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-06-20  create this file.
 ************************************************************************/
 #ifndef _XFFONTWORKSTYLE_HXX
 #define _XFFONTWORKSTYLE_HXX

#include	"xfstyle.hxx"

class XFFontWorkStyle
{
public:
    XFFontWorkStyle() :
        m_nButtonForm(-1),
        m_bHideForm(sal_True),
        m_fDistance(-0.10),
        m_eStyleType(enumXFFWOff),
        m_eAdjustType(enumXFFWAdjustAutosize),
        m_eShadowType(enumXFFWShadowNone)
        {};

    ~XFFontWorkStyle(){}

    inline void SetButtonForm(sal_Int8 nIndex);
    inline void SetHideForm(sal_Bool bHide);
    inline void SetFWDistance(double fDist);
    inline void SetFWStyleType(enumXFFWStyle eStyle);
    inline void SetFWAdjustType(enumXFFWAdjust eAdjust);
    inline void SetFWShadowType(enumXFFWShadow eShadow);

    inline sal_Int8 GetButtonForm() const;
    inline double GetFWDistance() const;
    inline enumXFFWStyle GetStyleType() const;
    inline enumXFFWAdjust GetAdjustType() const;
    inline enumXFFWShadow GetShadowType() const;

private:
    sal_Int8 m_nButtonForm;
    sal_Bool m_bHideForm;
    double m_fDistance;
    enumXFFWStyle m_eStyleType;
    enumXFFWAdjust m_eAdjustType;
    enumXFFWShadow m_eShadowType;
};

inline void XFFontWorkStyle::SetButtonForm(sal_Int8 nIndex)
{
    m_nButtonForm = nIndex;
}

inline void XFFontWorkStyle::SetHideForm(sal_Bool bHide)
{
    m_bHideForm = bHide;
}

inline void XFFontWorkStyle::SetFWDistance(double fDist)
{
    m_fDistance = fDist;
}

inline void XFFontWorkStyle::SetFWStyleType(enumXFFWStyle eStyle)
{
    m_eStyleType = eStyle;
}

inline void XFFontWorkStyle::SetFWAdjustType(enumXFFWAdjust eAdjust)
{
    m_eAdjustType = eAdjust;
}

inline void XFFontWorkStyle::SetFWShadowType(enumXFFWShadow eShadow)
{
    m_eShadowType = eShadow;
}

inline sal_Int8 XFFontWorkStyle::GetButtonForm() const
{
    return m_nButtonForm;
}
inline double XFFontWorkStyle::GetFWDistance() const
{
    return m_fDistance;
}

inline enumXFFWStyle XFFontWorkStyle::GetStyleType() const
{
    return m_eStyleType;
}
inline enumXFFWAdjust XFFontWorkStyle::GetAdjustType() const
{
    return m_eAdjustType;
}
inline enumXFFWShadow XFFontWorkStyle::GetShadowType() const
{
    return m_eShadowType;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
