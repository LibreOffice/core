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

#ifndef _SDGMOITM_HXX
#define _SDGMOITM_HXX

#include <svtools/grfmgr.hxx>
#include <svl/eitem.hxx>
#include <svx/svddef.hxx>
#include "svx/svxdllapi.h"

//------------------
// SdrGrafModeItem -
//------------------

class SVX_DLLPUBLIC SdrGrafModeItem : public SfxEnumItem
{
public:

                            TYPEINFO();

                            SdrGrafModeItem( GraphicDrawMode eMode = GRAPHICDRAWMODE_STANDARD ) : SfxEnumItem( SDRATTR_GRAFMODE, (sal_uInt16)eMode ) {}
                            SdrGrafModeItem( SvStream& rIn ) : SfxEnumItem( SDRATTR_GRAFMODE, rIn ) {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const;
    virtual sal_uInt16          GetValueCount() const;
    GraphicDrawMode         GetValue() const { return (GraphicDrawMode) SfxEnumItem::GetValue(); }

    virtual String          GetValueTextByPos( sal_uInt16 nPos ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                                 SfxMapUnit eCoreMetric,
                                                 SfxMapUnit ePresMetric,
                                                 String& rText, const IntlWrapper * = 0) const;

};

#endif // _SDGMOITM_HXX
