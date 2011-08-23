/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove


#include <bf_svtools/stritem.hxx>

namespace binfilter
{


//============================================================================
//
//  class SfxStringItem
//
//============================================================================

TYPEINIT1_AUTOFACTORY(SfxStringItem, CntUnencodedStringItem)

//============================================================================
// virtual
SfxStringItem::SfxStringItem(USHORT which, SvStream & rStream):
    CntUnencodedStringItem(which)
{
    UniString aValue;
    readByteString(rStream, aValue);
    SetValue(aValue);
}


//============================================================================
// virtual
SfxPoolItem * SfxStringItem::Create(SvStream & rStream, USHORT) const
{
    return new SfxStringItem(Which(), rStream);
}

//============================================================================
// virtual
SvStream & SfxStringItem::Store(SvStream & rStream, USHORT) const
{
    writeByteString(rStream, GetValue());
    return rStream;
}

//============================================================================
// virtual
SfxPoolItem * SfxStringItem::Clone(SfxItemPool *) const
{
    return new SfxStringItem(*this);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
