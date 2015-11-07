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
#ifndef INCLUDED_SFX2_ZOOMITEM_HXX
#define INCLUDED_SFX2_ZOOMITEM_HXX

#include <svl/intitem.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/dllapi.h>
#include <o3tl/typed_flags_set.hxx>


enum class SvxZoomType
{
    PERCENT,       // GetValue() is no special percentage value
    OPTIMAL,       // GetValue() corresponds to the optimal size
    WHOLEPAGE,     // GetValue() corresponds to the whole page
    PAGEWIDTH,      // GetValue() corresponds to the page width
    PAGEWIDTH_NOBORDER  // GetValue() pagewidth without border
};

enum class SvxZoomEnableFlags
{
    NONE         = 0x0000,
    N50          = 0x0001,
    N75          = 0x0002,
    N100         = 0x0004,
    N150         = 0x0008,
    N200         = 0x0010,
    OPTIMAL      = 0x1000,
    WHOLEPAGE    = 0x2000,
    PAGEWIDTH    = 0x4000,
    ALL          = 0x701F
};
namespace o3tl
{
    template<> struct typed_flags<SvxZoomEnableFlags> : is_typed_flags<SvxZoomEnableFlags, 0x701f> {};
}

class SFX2_DLLPUBLIC SvxZoomItem: public SfxUInt16Item
{
    SvxZoomEnableFlags      nValueSet;  // allowed values (see #defines below)
    SvxZoomType             eType;

public:
    TYPEINFO_OVERRIDE();
    static SfxPoolItem* CreateDefault();

    SvxZoomItem( SvxZoomType eZoomType = SvxZoomType::PERCENT,
                 sal_uInt16 nVal = 0, sal_uInt16 nWhich = SID_ATTR_ZOOM );
    SvxZoomItem( const SvxZoomItem& );
    virtual ~SvxZoomItem();

    void                    SetValueSet( SvxZoomEnableFlags nValues ) { nValueSet = nValues; }
    SvxZoomEnableFlags      GetValueSet() const { return nValueSet; }

    SvxZoomType             GetType() const { return eType; }
    void                    SetType( SvxZoomType eNewType ) { eType = eNewType; }

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const override;
    virtual SfxPoolItem*    Create( SvStream& rStrm, sal_uInt16 nVersion ) const override;
    virtual SvStream&       Store( SvStream& rStrm , sal_uInt16 nItemVersion ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
