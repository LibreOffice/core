/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: theme.hxx,v $
 * $Revision: 1.5 $
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

namespace oox {
namespace drawingml {

// ============================================================================

const sal_Int32 THEMED_INDEX_SUBTLE     = 0;
const sal_Int32 THEMED_INDEX_MODERATE   = 1;
const sal_Int32 THEMED_INDEX_INTENSE    = 2;

typedef RefVector< FillProperties >     FillStyleList;
typedef RefVector< LineProperties >     LineStyleList;
typedef RefVector< PropertyMap >        EffectStyleList;

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
    inline const FillProperties*    getFillStyle( sal_Int32 nIndex ) const { return maFillStyleList.get( nIndex ).get(); }

    inline LineStyleList&           getLineStyleList() { return maLineStyleList; }
    inline const LineStyleList&     getLineStyleList() const { return maLineStyleList; }
    inline const LineProperties*    getLineStyle( sal_Int32 nIndex ) const { return maLineStyleList.get( nIndex ).get(); }

    inline EffectStyleList&         getEffectStyleList() { return maEffectStyleList; }
    inline const EffectStyleList&   getEffectStyleList() const { return maEffectStyleList; }
    inline const PropertyMap*       getEffectStyle( sal_Int32 nIndex ) const { return maEffectStyleList.get( nIndex ).get(); }

    inline FillStyleList&           getBgFillStyleList() { return maBgFillStyleList; }
    inline const FillStyleList&     getBgFillStyleList() const { return maBgFillStyleList; }
    inline const FillProperties*    getBgFillStyle( sal_Int32 nIndex ) const { return maBgFillStyleList.get( nIndex ).get(); }

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
    LineStyleList       maLineStyleList;
    EffectStyleList     maEffectStyleList;
    FillStyleList       maBgFillStyleList;
    Shape               maSpDef;
    Shape               maLnDef;
    Shape               maTxDef;
};

// ============================================================================

} // namespace drawingml
} // namespace oox

#endif

