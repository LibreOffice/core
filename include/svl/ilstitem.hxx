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

#ifndef INCLUDED_SVL_ILSTITEM_HXX
#define INCLUDED_SVL_ILSTITEM_HXX

#include <svl/svldllapi.h>
#include <svl/poolitem.hxx>
#include <vector>

namespace com::sun::star::uno { template <class E> class Sequence; }

class SVL_DLLPUBLIC SfxIntegerListItem final : public SfxPoolItem
{
    std::vector < sal_Int32 > m_aList;

public:
    static SfxPoolItem* CreateDefault();
    DECLARE_ITEM_TYPE_FUNCTION(SfxIntegerListItem)
    SfxIntegerListItem();
    SfxIntegerListItem( sal_uInt16 nWhich, std::vector < sal_Int32 >&& rList );
    SfxIntegerListItem( sal_uInt16 nWhich, const css::uno::Sequence < sal_Int32 >& rList );
    virtual ~SfxIntegerListItem() override;

    SfxIntegerListItem(SfxIntegerListItem const &) = default;
    SfxIntegerListItem(SfxIntegerListItem &&) = default;
    SfxIntegerListItem & operator =(SfxIntegerListItem const &) = delete; // due to SfxPoolItem
    SfxIntegerListItem & operator =(SfxIntegerListItem &&) = delete; // due to SfxPoolItem

    const std::vector< sal_Int32 >& GetList() const { return m_aList; }

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxIntegerListItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            PutValue  ( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
};

#endif // INCLUDED_SVL_ILSTITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
