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

#ifndef OOX_DRAWINGML_THEME_HXX
#define OOX_DRAWINGML_THEME_HXX

#include "oox/helper/containerhelper.hxx"
#include "oox/drawingml/clrscheme.hxx"
#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/textfont.hxx"

namespace oox {
namespace drawingml {

// ============================================================================

const sal_Int32 THEMED_STYLE_SUBTLE     = 1;
const sal_Int32 THEMED_STYLE_MODERATE   = 2;
const sal_Int32 THEMED_STYLE_INTENSE    = 3;

typedef RefVector< FillProperties >                     FillStyleList;
typedef RefVector< LineProperties >                     LineStyleList;
typedef RefVector< PropertyMap >                        EffectStyleList;
typedef RefMap< sal_Int32, TextCharacterProperties >    FontScheme;

// ============================================================================

class Theme
{
public:
    explicit            Theme();
                        ~Theme();

    inline void                     setStyleName( const ::rtl::OUString& rStyleName ) { maStyleName = rStyleName; }
    inline const ::rtl::OUString&   getStyleName() const { return maStyleName; }

    inline ClrScheme&               getClrScheme() { return maClrScheme; }
    inline const ClrScheme&         getClrScheme() const { return maClrScheme; }

    inline FillStyleList&           getFillStyleList() { return maFillStyleList; }
    inline const FillStyleList&     getFillStyleList() const { return maFillStyleList; }
    inline FillStyleList&           getBgFillStyleList() { return maBgFillStyleList; }
    inline const FillStyleList&     getBgFillStyleList() const { return maBgFillStyleList; }
    /** Returns the fill properties of the passed one-based themed style index. */
    const FillProperties*           getFillStyle( sal_Int32 nIndex ) const;

    inline LineStyleList&           getLineStyleList() { return maLineStyleList; }
    inline const LineStyleList&     getLineStyleList() const { return maLineStyleList; }
    /** Returns the line properties of the passed one-based themed style index. */
    const LineProperties*           getLineStyle( sal_Int32 nIndex ) const;

    inline EffectStyleList&         getEffectStyleList() { return maEffectStyleList; }
    inline const EffectStyleList&   getEffectStyleList() const { return maEffectStyleList; }
    /** Returns the effect properties of the passed one-based themed style index. */
    const PropertyMap*              getEffectStyle( sal_Int32 nIndex ) const;

    inline FontScheme&              getFontScheme() { return maFontScheme; }
    inline const FontScheme&        getFontScheme() const { return maFontScheme; }
    /** Returns theme font properties by scheme type (major/minor). */
    const TextCharacterProperties*  getFontStyle( sal_Int32 nSchemeType ) const;
    /** Returns theme font by placeholder name, e.g. the major latin theme font for the font name '+mj-lt'. */
    const TextFont*                 resolveFont( const ::rtl::OUString& rName ) const;

    inline Shape&                   getSpDef() { return maSpDef; }
    inline const Shape&             getSpDef() const { return maSpDef; }

    inline Shape&                   getLnDef() { return maLnDef; }
    inline const Shape&             getLnDef() const { return maLnDef; }

    inline Shape&                   getTxDef() { return maTxDef; }
    inline const Shape&             getTxDef() const { return maTxDef; }

private:
    ::rtl::OUString     maStyleName;
    ClrScheme           maClrScheme;
    FillStyleList       maFillStyleList;
    FillStyleList       maBgFillStyleList;
    LineStyleList       maLineStyleList;
    EffectStyleList     maEffectStyleList;
    FontScheme          maFontScheme;
    Shape               maSpDef;
    Shape               maLnDef;
    Shape               maTxDef;
};

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

