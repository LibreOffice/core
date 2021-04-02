/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sal/config.h>
#include <basegfx/basegfxdllapi.h>
#include <basegfx/polygon/b2dpolypolygon.hxx>

namespace com::sun::star::lang
{
struct Locale;
}

namespace gfx
{
class BASEGFX_DLLPUBLIC IFontAttribute
{
public:
    virtual ~IFontAttribute() {}

    virtual const OUString& getFamilyName() const = 0;
    virtual const OUString& getStyleName() const = 0;
    virtual sal_uInt16 getWeight() const = 0;
    virtual bool getSymbol() const = 0;
    virtual bool getVertical() const = 0;
    virtual bool getItalic() const = 0;
    virtual bool getOutline() const = 0;
    virtual bool getRTL() const = 0;
    virtual bool getBiDiStrong() const = 0;
    virtual bool getMonospaced() const = 0;
};

class BASEGFX_DLLPUBLIC ITextLayouter
{
public:
    virtual ~ITextLayouter() {}

    virtual void setFontAttribute(IFontAttribute const& rFontAttribute, double fFontScaleX,
                                  double fFontScaleY, const css::lang::Locale& rLocale)
        = 0;

    virtual void getTextOutlines(basegfx::B2DPolyPolygonVector& rB2DPolyPolyVector,
                                 const OUString& rText, sal_uInt32 nIndex, sal_uInt32 nLength,
                                 const std::vector<double>& rDXArray) const = 0;
};

} // end namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
