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

#ifndef _SFXENUMITEM_HXX
#define _SFXENUMITEM_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "bf_svtools/svtdllapi.h"
#endif

#ifndef _SVTOOLS_CENUMITM_HXX
#include <bf_svtools/cenumitm.hxx>
#endif

namespace binfilter
{

//============================================================================
class  SfxEnumItem: public CntEnumItem
{
protected:
    SfxEnumItem(USHORT which = 0, USHORT nValue = 0):
        CntEnumItem(which, nValue) {}

    SfxEnumItem(USHORT which, SvStream & rStream):
        CntEnumItem(which, rStream) {}

public:
    TYPEINFO();

};

//============================================================================
class  SfxBoolItem: public CntBoolItem
{
public:
    TYPEINFO();

    SfxBoolItem(USHORT which = 0, BOOL bValue = FALSE):
        CntBoolItem(which, bValue) {}

    SfxBoolItem(USHORT which, SvStream & rStream):
        CntBoolItem(which, rStream) {}

    virtual SfxPoolItem * Create(SvStream & rStream, USHORT) const
    { return new SfxBoolItem(Which(), rStream); }

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const
    { return new SfxBoolItem(*this); }
};

}

#endif //  _SFXENUMITEM_HXX

