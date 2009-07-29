/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xftshcit.hxx,v $
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

#ifndef _SVX_XFTSHCLIT_HXX
#define _SVX_XFTSHCLIT_HXX

#include <svx/xcolit.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* FormText-ShadowColorItem
|*
\************************************************************************/

class SVX_DLLPUBLIC XFormTextShadowColorItem : public XColorItem
{
public:
            TYPEINFO();
            XFormTextShadowColorItem() {}
            XFormTextShadowColorItem(INT32 nIndex, const Color& rTheColor);
            XFormTextShadowColorItem(const String& rName, const Color& rTheColor);
            XFormTextShadowColorItem(SvStream& rIn);

    virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    virtual SfxPoolItem*    Create(SvStream& rIn, USHORT nVer) const;
};

#endif
