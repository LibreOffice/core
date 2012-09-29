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
 * Line height for paragraph.
 ************************************************************************/
#ifndef     _XFLINEHEIGHT_HXX
#define     _XFLINEHEIGHT_HXX

#include <string.h>

class XFLineHeight
{
public:
    XFLineHeight()
    {
        Reset();
    }
public:
    void    SetHeight(double value);

    void    SetLeastHeight(double value);

    void    SetPercent(sal_Int32 value);

    void    SetSpace(double value);

    void    Reset();

    void    ToXml(IXFStream *pStrm);

    enumLHType GetType();

    friend bool operator==(XFLineHeight& indent1, XFLineHeight& indent2);
    friend bool operator!=(XFLineHeight& indent1, XFLineHeight& indent2);
private:
    union{
        sal_Int32   m_nValue;
        double      m_fValue;
    }m_Value;
    enumLHType      m_eType;
};

inline void XFLineHeight::SetHeight(double value)
{
    m_Value.m_fValue = value;
    m_eType = enumLHHeight;
}

inline void XFLineHeight::SetLeastHeight(double value)
{
    m_Value.m_fValue = value;
    m_eType = enumLHLeast;
}

inline void XFLineHeight::SetPercent(sal_Int32 value)
{
    m_Value.m_nValue = value;
    m_eType = enumLHPercent;
}
inline void XFLineHeight::SetSpace(double value)
{
    m_Value.m_fValue = value;
    m_eType = enumLHSpace;
}
inline void XFLineHeight::Reset()
{
    m_eType = enumLHNone;
    memset(&m_Value, 0, sizeof(m_Value));
}

inline enumLHType XFLineHeight::GetType()
{
    return m_eType;
}

inline void XFLineHeight::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    switch(m_eType)
    {
    case enumLHNone:
        return;
    case enumLHHeight:
        pAttrList->AddAttribute( A2OUSTR("fo:line-height"),DoubleToOUString(m_Value.m_fValue) + A2OUSTR("cm") );
        break;
    case enumLHLeast:
        pAttrList->AddAttribute( A2OUSTR("style:line-height-at-least"),DoubleToOUString(m_Value.m_fValue) + A2OUSTR("cm") );
        break;
    case enumLHPercent:
        pAttrList->AddAttribute( A2OUSTR("fo:line-height"), DoubleToOUString(m_Value.m_nValue) + A2OUSTR("%") );
        break;
    case enumLHSpace:
        pAttrList->AddAttribute( A2OUSTR("style:line-spacing"), DoubleToOUString(m_Value.m_fValue) + A2OUSTR("cm") );
        break;
    }
}

inline bool operator==(XFLineHeight& lh1, XFLineHeight& lh2)
{
    if( lh1.m_eType != lh2.m_eType )
        return false;
    if( lh1.m_eType == enumLHPercent )
    {
        if( lh1.m_Value.m_nValue != lh2.m_Value.m_nValue )
            return false;
    }
    else
    {
        if( lh1.m_Value.m_fValue != lh2.m_Value.m_fValue )
            return false;
    }
    return true;
}

inline bool operator!=(XFLineHeight& lh1, XFLineHeight& lh2)
{
    return !(lh1==lh2);
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
