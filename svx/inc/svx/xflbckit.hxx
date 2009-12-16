/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xflbckit.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SVX_XFLBCKIT_HXX
#define _SVX_XFLBCKIT_HXX

#include <svtools/eitem.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* Item to enable background for filled objects
|*
\************************************************************************/

class SVX_DLLPUBLIC XFillBackgroundItem : public SfxBoolItem
{
public:
                            TYPEINFO();

                            XFillBackgroundItem( BOOL bFill = FALSE );
                            SVX_DLLPRIVATE XFillBackgroundItem( SvStream& rIn );


    SVX_DLLPRIVATE virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const;
    SVX_DLLPRIVATE virtual SfxPoolItem*    Create( SvStream& rIn, USHORT nVer ) const;

    SVX_DLLPRIVATE virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;
};

#endif
