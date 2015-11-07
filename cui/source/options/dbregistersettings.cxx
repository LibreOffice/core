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

#include "dbregistersettings.hxx"

#include <rtl/ustring.hxx>


namespace svx
{


    DatabaseMapItem::DatabaseMapItem( sal_uInt16 _nId, const DatabaseRegistrations& _rRegistrations )
        :SfxPoolItem( _nId )
        ,m_aRegistrations( _rRegistrations )
    {
    }


    bool DatabaseMapItem::operator==( const SfxPoolItem& _rCompare ) const
    {
        const DatabaseMapItem* pItem = dynamic_cast<const DatabaseMapItem*>( &_rCompare );
        if ( !pItem )
            return false;

        if ( m_aRegistrations.size() != pItem->m_aRegistrations.size() )
            return false;

        return m_aRegistrations == pItem->m_aRegistrations;
    }


    SfxPoolItem* DatabaseMapItem::Clone( SfxItemPool* ) const
    {
        return new DatabaseMapItem( Which(), m_aRegistrations );
    }




}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
