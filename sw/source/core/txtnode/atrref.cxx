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

#include <fmtrfmrk.hxx>

#include <hintids.hxx>
#include <hints.hxx>
#include <txtrfmrk.hxx>
#include <swfont.hxx>

SwFormatRefMark::~SwFormatRefMark( )
{
}

SwFormatRefMark::SwFormatRefMark( const OUString& rName )
    : SfxPoolItem(RES_TXTATR_REFMARK)
    , SwModify(nullptr)
    , pTextAttr(nullptr)
    , aRefName(rName)
{
}

SwFormatRefMark::SwFormatRefMark( const SwFormatRefMark& rAttr )
    : SfxPoolItem(RES_TXTATR_REFMARK)
    , SwModify(nullptr)
    , pTextAttr(nullptr)
    , aRefName(rAttr.aRefName)
{
}

bool SwFormatRefMark::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return aRefName == static_cast<const SwFormatRefMark&>(rAttr).aRefName;
}

SfxPoolItem* SwFormatRefMark::Clone( SfxItemPool* ) const
{
    return new SwFormatRefMark( *this );
}

void SwFormatRefMark::Modify(SfxPoolItem const* pOld, SfxPoolItem const* pNew)
{
    NotifyClients(pOld, pNew);
    if (pOld && (RES_REMOVE_UNO_OBJECT == pOld->Which()))
    {   // invalidate cached UNO object
        SetXRefMark(css::uno::Reference<css::text::XTextContent>(nullptr));
    }
}

void SwFormatRefMark::InvalidateRefMark()
{
    SwPtrMsgPoolItem const item(RES_REMOVE_UNO_OBJECT,
            &static_cast<SwModify&>(*this)); // cast to base class (void*)
    NotifyClients(&item, &item);
}

// Attribut fuer Inhalts-/Positions-Referenzen im Text

SwTextRefMark::SwTextRefMark( SwFormatRefMark& rAttr,
            sal_Int32 const nStartPos, sal_Int32 const*const pEnd)
    : SwTextAttr(rAttr, nStartPos)
    , SwTextAttrEnd( rAttr, nStartPos, nStartPos )
    , m_pTextNode( nullptr )
    , m_pEnd( nullptr )
{
    rAttr.pTextAttr = this;
    if ( pEnd )
    {
        m_nEnd = *pEnd;
        m_pEnd = & m_nEnd;
    }
    else
    {
        SetHasDummyChar(true);
    }
    SetDontMoveAttr( true );
    SetOverlapAllowedAttr( true );
}

sal_Int32* SwTextRefMark::GetEnd()
{
    return m_pEnd;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
