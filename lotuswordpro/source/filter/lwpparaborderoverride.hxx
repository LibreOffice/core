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
* Border override of Wordpro.
************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPPARABORDEROVERRIDE_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPPARABORDEROVERRIDE_HXX

#include "lwpoverride.hxx"

class LwpBorderStuff;
class LwpShadow;
class LwpMargins;

class LwpParaBorderOverride : public LwpOverride
{
public:
    LwpParaBorderOverride();
    virtual ~LwpParaBorderOverride();

    virtual LwpParaBorderOverride* clone() const override;

    enum BorderWidthType
    {
        PB_NONE         = 0,        /* No border */
        PB_TEXTWIDTH    = 1,        /* Border is width of text */
        PB_MARGINWIDTH  = 2,        /* Border extends to margins */
        PB_CUSTOMWIDTH  = 3         /* Border width is specified explicitly */
    };
public:
    virtual void Read(LwpObjectStream *pStrm) override;

    LwpShadow*  GetShadow(){ return m_pShadow; }
    LwpBorderStuff* GetBorderStuff(){ return m_pBorderStuff; }
    LwpMargins* GetMargins() { return m_pMargins; };

    void Override(LwpParaBorderOverride* pOther);

    inline bool IsBorderStuffOverridden();
    inline bool IsBetweenStuffOverridden();
    inline bool IsShadowOverridden();
    inline bool IsMarginsOverridden();
    inline bool IsAboveTypeOverridden();
    inline bool IsBelowTypeOverridden();
    inline bool IsRightTypeOverridden();
    inline bool IsBetweenTypeOverridden();
    inline bool IsAboveWidthOverridden();
    inline bool IsBelowWidthOverridden();
    inline bool IsBetweenWidthOverridden();
    inline bool IsRightWidthOverridden();
    inline bool IsBetweenMarginOverridden();

    void OverrideBorderStuff(LwpBorderStuff* pBorderStuff);
    void OverrideBetweenStuff(LwpBorderStuff* pBorderStuff);
    void OverrideShadow(LwpShadow* pShadow);
    void OverrideMargins(LwpMargins* pMargins);
    void OverrideAboveType(BorderWidthType eNewType);
    void OverrideBelowType(BorderWidthType eNewType);
    void OverrideRightType(BorderWidthType eNewType);
    void OverrideBetweenType(BorderWidthType eNewType);
    void OverrideAboveWidth(sal_uInt32 nNewWidth);
    void OverrideBelowWidth(sal_uInt32 nNewWidth);
    void OverrideBetweenWidth(sal_uInt32 nNewWidth);
    void OverrideRightWidth(sal_uInt32 nNewWidth);
    void OverrideBetweenMargin(sal_uInt32 nNewMargin);

    inline void RevertBorderStuff();
    inline void RevertBetweenStuff();
    inline void RevertShadow();
    inline void RevertMargins();
    inline void RevertAboveType();
    inline void RevertBelowType();
    inline void RevertRightType();
    inline void RevertBetweenType();
    inline void RevertAboveWidth();
    inline void RevertBelowWidth();
    inline void RevertBetweenWidth();
    inline void RevertRightWidth();
    inline void RevertBetweenMargin();

    friend class LwpParaBorderPiece;

protected:
    LwpParaBorderOverride(LwpParaBorderOverride const& rOther);

private:
    LwpParaBorderOverride& operator=(LwpParaBorderOverride const& rOther) = delete;

protected:
    enum
    {
        PBO_STUFF           = 0x0001,
        PBO_SHADOW          = 0x0002,
        PBO_MARGINS         = 0x0004,
        PBO_ABOVETYPE       = 0x0008,
        PBO_BELOWTYPE       = 0x0010,
        PBO_RIGHTTYPE       = 0x0020,
        PBO_ABOVE           = 0x0040,
        PBO_BELOW           = 0x0080,
        PBO_BETWEENSTUFF    = 0x0100,
        PBO_BETWEENTYPE     = 0x0200,
        PBO_BETWEEN         = 0x0400,
        PBO_BETWEENMARGIN   = 0x0800,
        PBO_RIGHT           = 0x1000
    };

private:
    LwpBorderStuff      *m_pBorderStuff;
    LwpBorderStuff      *m_pBetweenStuff;
    LwpShadow           *m_pShadow;
    LwpMargins          *m_pMargins;

    BorderWidthType     m_eAboveType;
    BorderWidthType     m_eBelowType;
    BorderWidthType     m_eRightType;
    BorderWidthType     m_eBetweenType;

    sal_uInt32          m_nAboveWidth;
    sal_uInt32          m_nBelowWidth;
    sal_uInt32          m_nBetweenWidth;
    sal_uInt32          m_nRightWidth;

    sal_uInt32          m_nBetweenMargin;
};

inline bool LwpParaBorderOverride::IsBorderStuffOverridden()
{
    return ((m_nOverride & PBO_STUFF) != 0);
}
inline bool LwpParaBorderOverride::IsBetweenStuffOverridden()
{
    return ((m_nOverride & PBO_BETWEENSTUFF) != 0);
}
inline bool LwpParaBorderOverride::IsShadowOverridden()
{
    return ((m_nOverride & PBO_SHADOW) != 0);
}
inline bool LwpParaBorderOverride::IsMarginsOverridden()
{
    return ((m_nOverride & PBO_MARGINS) != 0);
}
inline bool LwpParaBorderOverride::IsAboveTypeOverridden()
{
    return ((m_nOverride & PBO_ABOVETYPE) != 0);
}
inline bool LwpParaBorderOverride::IsBelowTypeOverridden()
{
    return ((m_nOverride & PBO_BELOWTYPE) != 0);
}
inline bool LwpParaBorderOverride::IsRightTypeOverridden()
{
    return ((m_nOverride & PBO_RIGHTTYPE) != 0);
}
inline bool LwpParaBorderOverride::IsBetweenTypeOverridden()
{
    return ((m_nOverride & PBO_BETWEENTYPE) != 0);
}
inline bool LwpParaBorderOverride::IsAboveWidthOverridden()
{
    return ((m_nOverride & PBO_ABOVE) != 0);
}
inline bool LwpParaBorderOverride::IsBelowWidthOverridden()
{
    return ((m_nOverride & PBO_BELOW) != 0);
}
inline bool LwpParaBorderOverride::IsBetweenWidthOverridden()
{
    return ((m_nOverride & PBO_BETWEEN) != 0);
}
inline bool LwpParaBorderOverride::IsRightWidthOverridden()
{
    return ((m_nOverride & PBO_RIGHT) != 0);
}
inline bool LwpParaBorderOverride::IsBetweenMarginOverridden()
{
    return ((m_nOverride & PBO_BETWEENMARGIN) != 0);
}

inline void LwpParaBorderOverride::RevertBorderStuff()
{
    LwpOverride::Override(PBO_STUFF, STATE_STYLE);
}
inline void LwpParaBorderOverride::RevertBetweenStuff()
{
    LwpOverride::Override(PBO_BETWEENSTUFF, STATE_STYLE);
}
inline void LwpParaBorderOverride::RevertShadow()
{
    LwpOverride::Override(PBO_SHADOW, STATE_STYLE);
}
inline void LwpParaBorderOverride::RevertMargins()
{
    LwpOverride::Override(PBO_MARGINS, STATE_STYLE);
}
inline void LwpParaBorderOverride::RevertAboveType()
{
    LwpOverride::Override(PBO_ABOVETYPE, STATE_STYLE);
}
inline void LwpParaBorderOverride::RevertBelowType()
{
    LwpOverride::Override(PBO_BELOWTYPE, STATE_STYLE);
}
inline void LwpParaBorderOverride::RevertRightType()
{
    LwpOverride::Override(PBO_RIGHTTYPE, STATE_STYLE);
}
inline void LwpParaBorderOverride::RevertBetweenType()
{
    LwpOverride::Override(PBO_BETWEENTYPE, STATE_STYLE);
}
inline void LwpParaBorderOverride::RevertAboveWidth()
{
    LwpOverride::Override(PBO_ABOVE, STATE_STYLE);
}
inline void LwpParaBorderOverride::RevertBelowWidth()
{
    LwpOverride::Override(PBO_BELOW, STATE_STYLE);
}
inline void LwpParaBorderOverride::RevertBetweenWidth()
{
    LwpOverride::Override(PBO_BETWEEN, STATE_STYLE);
}
inline void LwpParaBorderOverride::RevertRightWidth()
{
    LwpOverride::Override(PBO_RIGHT, STATE_STYLE);
}
inline void LwpParaBorderOverride::RevertBetweenMargin()
{
    LwpOverride::Override(PBO_BETWEENMARGIN, STATE_STYLE);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
