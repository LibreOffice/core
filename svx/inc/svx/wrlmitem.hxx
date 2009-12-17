/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wrlmitem.hxx,v $
 * $Revision: 1.4 $
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
#ifndef _SVX_WRLMITEM_HXX
#define _SVX_WRLMITEM_HXX

// include ---------------------------------------------------------------

#include <svl/eitem.hxx>
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include "svx/svxdllapi.h"

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxWordLineModeItem ---------------------------------------------

/*  [Beschreibung]

    Dieses Item beschreibt, ob Unterstrichen und Durchgestrichen auf
    Wortgrenzen beschraenkt ist.
*/

class SVX_DLLPUBLIC SvxWordLineModeItem : public SfxBoolItem
{
public:
    TYPEINFO();

    SvxWordLineModeItem( const BOOL bWordLineMode /*= FALSE*/,
                     const USHORT nId  );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT) const;
    virtual SvStream&       Store(SvStream &, USHORT nItemVersion) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    inline SvxWordLineModeItem& operator=( const SvxWordLineModeItem& rWLM )
        {
            SetValue( rWLM.GetValue() );
            return *this;
        }
};

#endif

