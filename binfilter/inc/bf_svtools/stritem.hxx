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

#ifndef _SFXSTRITEM_HXX
#define _SFXSTRITEM_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "bf_svtools/svtdllapi.h"
#endif

#ifndef _SVTOOLS_CUSTRITM_HXX
#include <bf_svtools/custritm.hxx>
#endif

namespace binfilter
{

//============================================================================
class  SfxStringItem: public CntUnencodedStringItem
{
public:
    TYPEINFO();

    SfxStringItem() {}

    SfxStringItem(USHORT which, const XubString & rValue):
        CntUnencodedStringItem(which, rValue) {}

    SfxStringItem(USHORT nWhich, SvStream & rStream);

    virtual SfxPoolItem * Create(SvStream & rStream, USHORT) const;

    virtual SvStream & Store(SvStream & rStream, USHORT) const;

    virtual SfxPoolItem * Clone(SfxItemPool * = 0) const;
};

}

#endif // _SFXSTRITEM_HXX

