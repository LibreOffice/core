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
 *  For LWP filter architecture prototype
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/

#ifndef _LWPCHARBORDEROVERRIDE_HXX
#define _LWPCHARBORDEROVERRIDE_HXX

#include "lwpoverride.hxx"

class LwpBorderStuff;
class LwpMargins;
class LwpCharacterBorderOverride : public LwpOverride
{
public:
    LwpCharacterBorderOverride();

    virtual ~LwpCharacterBorderOverride();

    void Read(LwpObjectStream* pStrm);

    void Override(LwpCharacterBorderOverride* pOther);

    inline LwpBorderStuff* GetBorderStuff();
    inline LwpMargins* GetMargins();
    inline sal_Int32 GetAboveWidth() const;
    inline sal_Int32 GetBelowWidth() const;

    inline sal_Bool IsBorderStuffOverridden();
    inline sal_Bool IsMarginsOverridden();
    inline sal_Bool IsAboveWidthOverridden();
    inline sal_Bool IsBelowWidthOverridden();

    inline void OverrideBorderStuff(LwpBorderStuff* pOther);
    inline void OverrideMargins(LwpMargins* pOther);
    inline void OverrideAboveWidth(sal_Int32 nNewWidth);
    inline void OverrideBelowWidth(sal_uInt32 nNewWidth);

    inline void RevertBorderStuff();
    inline void RevertMargins();
    inline void RevertAboveWidth();
    inline void RevertBelowWidth();

private:
    LwpBorderStuff*	m_pBorderStuff;
    LwpMargins*		m_pMargins;
    sal_Int32		m_nAboveWidth;
    sal_Int32		m_nBelowWidth;

    enum
    {
        PBO_STUFF		= 0x01,
        PBO_MARGINS 	= 0x04,
        PBO_ABOVE		= 0x40,
        PBO_BELOW		= 0x80
    };
};

inline LwpMargins* LwpCharacterBorderOverride::GetMargins()
{
    return m_pMargins;
}

inline LwpBorderStuff* LwpCharacterBorderOverride::GetBorderStuff()
{
    return m_pBorderStuff;
}

inline sal_Int32 LwpCharacterBorderOverride::GetAboveWidth() const
{
    return m_nAboveWidth;
}

inline sal_Int32 LwpCharacterBorderOverride::GetBelowWidth() const
{
    return m_nBelowWidth;
}

inline sal_Bool LwpCharacterBorderOverride::IsBorderStuffOverridden()
{
    return (sal_Bool)((m_nOverride & PBO_STUFF) != 0);
}

inline sal_Bool LwpCharacterBorderOverride::IsMarginsOverridden()
{
    return (sal_Bool)((m_nOverride & PBO_MARGINS) != 0);
}

inline sal_Bool LwpCharacterBorderOverride::IsAboveWidthOverridden()
{
    return (sal_Bool)((m_nOverride & PBO_ABOVE) != 0);
}

inline sal_Bool LwpCharacterBorderOverride::IsBelowWidthOverridden()
{
    return (sal_Bool)((m_nOverride & PBO_BELOW) != 0);
}

inline void LwpCharacterBorderOverride::OverrideBorderStuff(LwpBorderStuff* pOther)
{
    m_pBorderStuff = pOther;
    LwpOverride::Override(PBO_STUFF, STATE_ON);
}
inline void LwpCharacterBorderOverride::OverrideMargins(LwpMargins* pOther)
{
    m_pMargins = pOther;
    LwpOverride::Override(PBO_MARGINS, STATE_ON);
}
inline void LwpCharacterBorderOverride::OverrideAboveWidth(sal_Int32 nNewWidth)
{
    m_nAboveWidth = nNewWidth;
    LwpOverride::Override(PBO_ABOVE, STATE_ON);
}
inline void LwpCharacterBorderOverride::OverrideBelowWidth(sal_uInt32 nNewWidth)
{
    m_nBelowWidth = nNewWidth;
    LwpOverride::Override(PBO_BELOW, STATE_ON);
}

inline void LwpCharacterBorderOverride::RevertBorderStuff()
{
    LwpOverride::Override(PBO_STUFF, STATE_STYLE);
}
inline void LwpCharacterBorderOverride::RevertMargins()
{
    LwpOverride::Override(PBO_MARGINS, STATE_STYLE);
}
inline void LwpCharacterBorderOverride::RevertAboveWidth()
{
    LwpOverride::Override(PBO_ABOVE, STATE_STYLE);
}
inline void LwpCharacterBorderOverride::RevertBelowWidth()
{
    LwpOverride::Override(PBO_BELOW, STATE_STYLE);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
