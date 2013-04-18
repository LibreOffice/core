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

#ifndef _SVXE3DITEM_HXX
#define _SVXE3DITEM_HXX

#include <svl/poolitem.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include "svx/svxdllapi.h"

#ifndef _SVXVECT3DITEM_HXX
#define _SVXVECT3DITEM_HXX

class SvStream;

class SVX_DLLPUBLIC SvxB3DVectorItem : public SfxPoolItem
{
    basegfx::B3DVector  aVal;

public:
                            TYPEINFO();
                            SvxB3DVectorItem();
                            SvxB3DVectorItem( sal_uInt16 nWhich, const basegfx::B3DVector& rVal );
                            SvxB3DVectorItem( const SvxB3DVectorItem& );
                            ~SvxB3DVectorItem();

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16 nVersion) const;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion ) const;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    const basegfx::B3DVector&           GetValue() const { return aVal; }
            void            SetValue( const basegfx::B3DVector& rNewVal ) {
                                 DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
                                 aVal = rNewVal;
                             }

    virtual sal_uInt16 GetVersion (sal_uInt16 nFileFormatVersion) const;
};

#endif

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
