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
#include <o3tl/hash_combine.hxx>
#include <svx/e3ditem.hxx>

using namespace ::com::sun::star;


SvxB3DVectorItem::~SvxB3DVectorItem()
{
}


SvxB3DVectorItem::SvxB3DVectorItem( TypedWhichId<SvxB3DVectorItem> _nWhich, const basegfx::B3DVector& rVal ) :
    SfxPoolItem( _nWhich ),
    m_aVal( rVal )
{
}


SvxB3DVectorItem::SvxB3DVectorItem( const SvxB3DVectorItem& rItem ) :
    SfxPoolItem( rItem ),
    m_aVal( rItem.m_aVal )
{
}

bool SvxB3DVectorItem::operator==( const SfxPoolItem &rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    return static_cast<const SvxB3DVectorItem&>(rItem).m_aVal == m_aVal;
}

SvxB3DVectorItem* SvxB3DVectorItem::Clone( SfxItemPool* /*pPool*/ ) const
{
    return new SvxB3DVectorItem( *this );
}

bool SvxB3DVectorItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    assert(!std::isnan(m_aVal.getX()) && !std::isnan(m_aVal.getY()) && !std::isnan(m_aVal.getZ()));

    drawing::Direction3D aDirection;

    // enter values
    aDirection.DirectionX = m_aVal.getX();
    aDirection.DirectionY = m_aVal.getY();
    aDirection.DirectionZ = m_aVal.getZ();

    rVal <<= aDirection;
    return true;
}


bool SvxB3DVectorItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    drawing::Direction3D aDirection;
    if(!(rVal >>= aDirection))
        return false;

    m_aVal.setX(aDirection.DirectionX);
    m_aVal.setY(aDirection.DirectionY);
    m_aVal.setZ(aDirection.DirectionZ);

    assert(!std::isnan(m_aVal.getX()) && !std::isnan(m_aVal.getY()) && !std::isnan(m_aVal.getZ()));

    return true;
}


void SvxB3DVectorItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SvxB3DVectorItem"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("x"), BAD_CAST(OString::number(m_aVal.getX()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("y"), BAD_CAST(OString::number(m_aVal.getY()).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("z"), BAD_CAST(OString::number(m_aVal.getZ()).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
