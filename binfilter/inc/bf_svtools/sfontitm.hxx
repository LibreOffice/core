/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SFONTITM_HXX
#define _SFONTITM_HXX

#include <tools/color.hxx>
#include <tools/gen.hxx>
#include <i18npool/lang.h>

#include <bf_svtools/poolitem.hxx>

namespace binfilter
{

//============================================================================
class SfxFontItem: public SfxPoolItem
{
    XubString m_aName;
    XubString m_aStyleName;
    Size m_aSize;
    Color m_aColor;
    Color m_aFillColor;
    rtl_TextEncoding m_nCharSet;
    LanguageType m_nLanguage;
    sal_Int16 m_nFamily;
    sal_Int16 m_nPitch;
    sal_Int16 m_nWeight;
    sal_Int16 m_nWidthType;
    sal_Int16 m_nItalic;
    sal_Int16 m_nUnderline;
    sal_Int16 m_nStrikeout;
    sal_Int16 m_nOrientation;
    unsigned m_bWordLine: 1;
    unsigned m_bOutline: 1;
    unsigned m_bShadow: 1;
    unsigned m_bKerning: 1;
    unsigned m_bHasFont: 1;
    unsigned m_bHasColor: 1;
    unsigned m_bHasFillColor: 1;

public:
    TYPEINFO();

    inline SfxFontItem(USHORT nWhich);

    virtual	int operator ==(const SfxPoolItem & rItem) const;

    virtual SfxPoolItem * Create(SvStream & rStream, USHORT) const;

    virtual SvStream & Store(SvStream & rStream, USHORT) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const
    { return new SfxFontItem(*this); }

    sal_Bool hasFont() const { return m_bHasFont; }

    sal_Bool hasColor() const { return m_bHasColor; }

    sal_Bool hasFillColor() const { return m_bHasFillColor; }

    const XubString & getName() const { return m_aName; }

    const XubString & getStyleName() const { return m_aStyleName; }

    const Size & getSize() const { return m_aSize; }

    const Color & getColor() const { return m_aColor; }

    const Color & getFillColor() const { return m_aFillColor; }

    rtl_TextEncoding getCharSet() const { return m_nCharSet; }

    LanguageType getLanguage() const { return m_nLanguage; }

    sal_Int16 getFamily() const { return m_nFamily; }

    sal_Int16 getPitch() const { return m_nPitch; }

    sal_Int16 getWeight() const { return m_nWeight; }

    sal_Int16 getWidthType() const { return m_nWidthType; }

    sal_Int16 getItalic() const { return m_nItalic; }

    sal_Int16 getUnderline() const { return m_nUnderline; }

    sal_Int16 getStrikeout() const { return m_nStrikeout; }

    sal_Int16 getOrientation() const { return m_nOrientation; }

    sal_Bool getWordLine() const { return m_bWordLine; }

    sal_Bool getOutline() const { return m_bOutline; }

    sal_Bool getShadow() const { return m_bShadow; }

    sal_Bool getKerning() const { return m_bKerning; }

    inline void setFont(sal_Int16 nTheFamily, const XubString & rTheName,
                        const XubString & rTheStyleName, sal_Int16 nThePitch,
                        rtl_TextEncoding nTheCharSet);

    inline void setWeight(sal_Int16 nTheWeight);

    inline void setItalic(sal_Int16 nTheItalic);

    inline void setHeight(sal_Int32 nHeight);

    inline void setColor(const Color & rTheColor);

    inline void setFillColor(const Color & rTheFillColor);

    inline void setUnderline(sal_Int16 nTheUnderline);

    inline void setStrikeout(sal_Int16 nTheStrikeout);

    inline void setOutline(sal_Bool bTheOutline);

    inline void setShadow(sal_Bool bTheShadow);

    inline void setLanguage(LanguageType nTheLanguage);
};

inline SfxFontItem::SfxFontItem(USHORT which):
    SfxPoolItem(which),
    m_nCharSet(RTL_TEXTENCODING_DONTKNOW),
    m_nLanguage(LANGUAGE_DONTKNOW),
    m_nFamily(0), // FAMILY_DONTKNOW
    m_nPitch(0), // PITCH_DONTKNOW
    m_nWeight(0), // WEIGHT_DONTKNOW
    m_nWidthType(0), // WIDTH_DONTKNOW
    m_nItalic(3), // ITALIC_DONTKNOW
    m_nUnderline(4), // UNDERLINE_DONTKNOW
    m_nStrikeout(3), // STRIKEOUT_DONTKNOW
    m_nOrientation(0),
    m_bWordLine(sal_False),
    m_bOutline(sal_False),
    m_bShadow(sal_False),
    m_bKerning(sal_False),
    m_bHasFont(sal_False),
    m_bHasColor(sal_False),
    m_bHasFillColor(sal_False)
{}

inline void SfxFontItem::setFont(sal_Int16 nTheFamily,
                                 const XubString & rTheName,
                                 const XubString & rTheStyleName,
                                 sal_Int16 nThePitch,
                                 rtl_TextEncoding nTheCharSet)
{
    m_nFamily = nTheFamily;
    m_aName = rTheName;
    m_aStyleName = rTheStyleName;
    m_nPitch = nThePitch;
    m_nCharSet = nTheCharSet;
    m_bHasFont = sal_True;
}

inline void SfxFontItem::setWeight(sal_Int16 nTheWeight)
{
    m_nWeight = nTheWeight;
    m_bHasFont = sal_True;
}

inline void SfxFontItem::setItalic(sal_Int16 nTheItalic)
{
    m_nItalic = nTheItalic;
    m_bHasFont = sal_True;
}

inline void SfxFontItem::setHeight(sal_Int32 nHeight)
{
    m_aSize.setHeight(nHeight);
    m_bHasFont = sal_True;
}

inline void SfxFontItem::setColor(const Color & rTheColor)
{
    m_aColor = rTheColor;
    m_bHasColor = sal_True;
}

inline void SfxFontItem::setFillColor(const Color & rTheFillColor)
{
    m_aFillColor = rTheFillColor;
    m_bHasFillColor = sal_True;
}

inline void SfxFontItem::setUnderline(sal_Int16 nTheUnderline)
{
    m_nUnderline = nTheUnderline;
    m_bHasFont = sal_True;
}

inline void SfxFontItem::setStrikeout(sal_Int16 nTheStrikeout)
{
    m_nStrikeout = nTheStrikeout;
    m_bHasFont = sal_True;
}

inline void SfxFontItem::setOutline(sal_Bool bTheOutline)
{
    m_bOutline = bTheOutline;
    m_bHasFont = sal_True;
}

inline void SfxFontItem::setShadow(sal_Bool bTheShadow)
{
    m_bShadow = bTheShadow;
    m_bHasFont = sal_True;
}

inline void SfxFontItem::setLanguage(LanguageType nTheLanguage)
{
    m_nLanguage = nTheLanguage;
    m_bHasFont = sal_True;
}

}

#endif // _SFONTITM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
