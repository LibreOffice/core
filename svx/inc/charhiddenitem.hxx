/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: charhiddenitem.hxx,v $
 * $Revision: 1.6 $
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
#ifndef _SVX_CHARHIDDENITEM_HXX
#define _SVX_CHARHIDDENITEM_HXX

// include ---------------------------------------------------------------

#include <svtools/poolitem.hxx>
#include <svtools/eitem.hxx>
#include "svx/svxdllapi.h"

// class SvxCharHiddenItem -------------------------------------------------
/*
    [Description]
    This item marks text as hidden
*/

class SVX_DLLPUBLIC SvxCharHiddenItem : public SfxBoolItem
{
public:
    TYPEINFO();

    SvxCharHiddenItem( const BOOL bHidden /*= FALSE*/, const USHORT nId );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    inline SvxCharHiddenItem& operator=(const SvxCharHiddenItem& rHidden) {
            SetValue(rHidden.GetValue());
            return *this;
        }
};

#endif
