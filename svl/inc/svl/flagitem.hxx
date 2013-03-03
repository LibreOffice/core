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
#ifndef _SFXFLAGITEM_HXX
#define _SFXFLAGITEM_HXX

#include "svl/svldllapi.h"
#include <tools/solar.h>
#include <tools/rtti.hxx>
#include <svl/poolitem.hxx>

class SvStream;

// -----------------------------------------------------------------------

DBG_NAMEEX_VISIBILITY(SfxFlagItem, SVL_DLLPUBLIC)

class SVL_DLLPUBLIC SfxFlagItem: public SfxPoolItem
{
    sal_uInt16                   nVal;

public:
                             TYPEINFO();

    explicit                 SfxFlagItem( sal_uInt16 nWhich = 0, sal_uInt16 nValue = 0 );
                             SfxFlagItem( const SfxFlagItem& );

                             ~SfxFlagItem() {
                                DBG_DTOR(SfxFlagItem, 0); }

    virtual sal_uInt8            GetFlagCount() const;

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16 nVersion) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString & rText,
                                    const IntlWrapper * = 0 ) const;
            sal_uInt16           GetValue() const { return nVal; }
            void             SetValue( sal_uInt16 nNewVal ) {
                                 DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
                                 nVal = nNewVal;
                             }
            bool             GetFlag( sal_uInt8 nFlag ) const { return ( (nVal & ( 1<<nFlag))); }
            void             SetFlag( sal_uInt8 nFlag, bool bVal ) { if(bVal) { nVal |= (1<<nFlag); } else { nVal &= ~(1<<nFlag);};}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
