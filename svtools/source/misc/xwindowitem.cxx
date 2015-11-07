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


#include <svtools/xwindowitem.hxx>

#include <vcl/window.hxx>


using namespace ::com::sun::star;




XWindowItem::XWindowItem() :
    SfxPoolItem()
{
}


XWindowItem::XWindowItem( const XWindowItem &rItem ) :
    SfxPoolItem( Which() ),
    m_xWin( rItem.m_xWin )
{
}


XWindowItem::~XWindowItem()
{
}


SfxPoolItem * XWindowItem::Clone( SfxItemPool* /*pPool*/ ) const
{
    return new XWindowItem( *this );
}


bool XWindowItem::operator == ( const SfxPoolItem & rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const XWindowItem * pItem = dynamic_cast< const XWindowItem * >(&rAttr);
    return pItem && m_xWin == pItem->m_xWin;
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
