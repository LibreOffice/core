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
#include "precompiled_svx.hxx"
#include "svx/svxgrahicitem.hxx"
TYPEINIT1(SvxGraphicItem,SfxPoolItem);
SvxGraphicItem::SvxGraphicItem( sal_uInt16 _nWhich, const Graphic& rGraphic )
    : SfxPoolItem( _nWhich ), aGraphic( rGraphic )
{

}
SvxGraphicItem::SvxGraphicItem( const SvxGraphicItem& rItem)
    : SfxPoolItem( rItem.Which() ), aGraphic( rItem.aGraphic )
{
}

int SvxGraphicItem::operator==( const SfxPoolItem& rItem) const
{
    return ((SvxGraphicItem&)rItem).aGraphic == aGraphic;
}

SfxPoolItem* SvxGraphicItem::Clone( SfxItemPool * ) const
{
    return new SvxGraphicItem( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
