/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/drawing/Direction3D.hpp>
#include <libxml/xmlwriter.h>

#include <svx/e3ditem.hxx>

using namespace ::com::sun::star;


SvxB3DVectorItem::~SvxB3DVectorItem()
{
}


SvxB3DVectorItem::SvxB3DVectorItem( sal_uInt16 _nWhich, const basegfx::B3DVector& rVal ) :
    SfxPoolItem( _nWhich ),
    aVal( rVal )
{
}


SvxB3DVectorItem::SvxB3DVectorItem( const SvxB3DVectorItem& rItem ) :
    SfxPoolItem( rItem ),
    aVal( rItem.aVal )
{
}


bool SvxB3DVectorItem::operator==( const SfxPoolItem &rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    return static_cast<const SvxB3DVectorItem&>(rItem).aVal == aVal;
}

SvxB3DVectorItem* SvxB3DVectorItem::Clone( SfxItemPool* /*pPool*/ ) const
{
    return new SvxB3DVectorItem( *this );
}

bool SvxB3DVectorItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    assert(!std::isnan(aVal.getX()) && !std::isnan(aVal.getY()) && !std::isnan(aVal.getZ()));

    drawing::Direction3D aDirection;

    // enter values
    aDirection.DirectionX = aVal.getX();
    aDirection.DirectionY = aVal.getY();
    aDirection.DirectionZ = aVal.getZ();

    rVal <<= aDirection;
    return true;
}


bool SvxB3DVectorItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    drawing::Direction3D aDirection;
    if(!(rVal >>= aDirection))
        return false;

    aVal.setX(aDirection.DirectionX);
    aVal.setY(aDirection.DirectionY);
    aVal.setZ(aDirection.DirectionZ);

    assert(!std::isnan(aVal.getX()) && !std::isnan(aVal.getY()) && !std::isnan(aVal.getZ()));

    return true;
}


void SvxB3DVectorItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SvxB3DVectorItem"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("x"), BAD_CAST(OString::number(aVal.getX()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("y"), BAD_CAST(OString::number(aVal.getY()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("z"), BAD_CAST(OString::number(aVal.getZ()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
