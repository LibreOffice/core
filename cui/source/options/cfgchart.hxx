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

#pragma once

#include <svl/poolitem.hxx>
#include <svx/xtable.hxx>
#include <vector>

class SvxChartColorTable
{
private:
    std::vector< XColorEntry >     m_aColorEntries;

public:
    // accessors
    size_t size() const;
    const XColorEntry & operator[]( size_t _nIndex ) const;
    Color getColor( size_t _nIndex ) const;

    // mutators
    void clear();
    void append( const XColorEntry & _rEntry );
    void remove( size_t _nIndex );
    void replace( size_t _nIndex, const XColorEntry & _rEntry );
    void useDefault();
    static OUString getDefaultName(size_t _nIndex);

    // comparison
    bool operator==( const SvxChartColorTable & _rOther ) const;
};


// all options

namespace SvxChartOptions
{
    SvxChartColorTable GetDefaultColors();
    void SetDefaultColors( const SvxChartColorTable& aCol );
};


// items
// Make Item read-only (no non-const access methods). Two reasons:
// (1) Preparation for Item refactor
// (2) Dangerous due to SfxItem may not be what you expect (e.g. when
//     ::Set in SfxItemSet, not your instance may be used there, no control
//     about what will happen without deep knowledge about SfxItems/SfxItemSets)
class SvxChartColorTableItem : public SfxPoolItem
{
public:
    DECLARE_ITEM_TYPE_FUNCTION(SvxChartColorTableItem)
    SvxChartColorTableItem( sal_uInt16 nWhich, SvxChartColorTable );

    virtual SvxChartColorTableItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;

    const SvxChartColorTable & GetColorList() const  { return m_aColorTable;}

private:
    SvxChartColorTable      m_aColorTable;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
