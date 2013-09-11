/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef OOX_DRAWINGML_THEME_HXX
#define OOX_DRAWINGML_THEME_HXX

#include "oox/drawingml/clrscheme.hxx"
#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/textfont.hxx"
#include <com/sun/star/xml/dom/XDocument.hpp>
#include "oox/dllapi.h"

namespace oox {
namespace drawingml {

// ============================================================================

const sal_Int32 THEMED_STYLE_SUBTLE     = 1;
const sal_Int32 THEMED_STYLE_MODERATE   = 2;
const sal_Int32 THEMED_STYLE_INTENSE    = 3;

typedef RefVector< FillProperties >                     FillStyleList;
typedef RefVector< LineProperties >                     LineStyleList;
typedef RefVector< EffectProperties >                   EffectStyleList;
typedef RefMap< sal_Int32, TextCharacterProperties >    FontScheme;

// ============================================================================

class OOX_DLLPUBLIC Theme
{
public:
                        Theme();
                        ~Theme();

    void                     setStyleName( const OUString& rStyleName ) { maStyleName = rStyleName; }
    const OUString&          getStyleName() const { return maStyleName; }

    ClrScheme&               getClrScheme() { return maClrScheme; }
    const ClrScheme&         getClrScheme() const { return maClrScheme; }

    FillStyleList&           getFillStyleList() { return maFillStyleList; }
    const FillStyleList&     getFillStyleList() const { return maFillStyleList; }
    FillStyleList&           getBgFillStyleList() { return maBgFillStyleList; }
    const FillStyleList&     getBgFillStyleList() const { return maBgFillStyleList; }
    /** Returns the fill properties of the passed one-based themed style index. */
    const FillProperties*           getFillStyle( sal_Int32 nIndex ) const;

    LineStyleList&           getLineStyleList() { return maLineStyleList; }
    const LineStyleList&     getLineStyleList() const { return maLineStyleList; }
    /** Returns the line properties of the passed one-based themed style index. */
    const LineProperties*           getLineStyle( sal_Int32 nIndex ) const;

    EffectStyleList&         getEffectStyleList() { return maEffectStyleList; }
    const EffectStyleList&   getEffectStyleList() const { return maEffectStyleList; }
    const EffectProperties*         getEffectStyle( sal_Int32 nIndex ) const;

    FontScheme&              getFontScheme() { return maFontScheme; }
    const FontScheme&        getFontScheme() const { return maFontScheme; }
    /** Returns theme font properties by scheme type (major/minor). */
    const TextCharacterProperties*  getFontStyle( sal_Int32 nSchemeType ) const;
    /** Returns theme font by placeholder name, e.g. the major latin theme font for the font name '+mj-lt'. */
    const TextFont*                 resolveFont( const OUString& rName ) const;

    Shape&                   getSpDef() { return maSpDef; }
    const Shape&             getSpDef() const { return maSpDef; }

    Shape&                   getLnDef() { return maLnDef; }
    const Shape&             getLnDef() const { return maLnDef; }

    Shape&                   getTxDef() { return maTxDef; }
    const Shape&             getTxDef() const { return maTxDef; }

    void                            setFragment( const ::com::sun::star::uno::Reference<
                                                    ::com::sun::star::xml::dom::XDocument>& xRef ) { mxFragment=xRef; }
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::dom::XDocument>& getFragment() const { return mxFragment; }

private:
    OUString     maStyleName;
    ClrScheme           maClrScheme;
    FillStyleList       maFillStyleList;
    FillStyleList       maBgFillStyleList;
    LineStyleList       maLineStyleList;
    EffectStyleList     maEffectStyleList;
    FontScheme          maFontScheme;
    Shape               maSpDef;
    Shape               maLnDef;
    Shape               maTxDef;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::dom::XDocument> mxFragment;
};

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
