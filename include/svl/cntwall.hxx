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
#ifndef INCLUDED_SVL_CNTWALL_HXX
#define INCLUDED_SVL_CNTWALL_HXX

#include <svl/svldllapi.h>

#include <tools/rtti.hxx>
#include <tools/color.hxx>
#include <svl/poolitem.hxx>

class SvStream;

class SVL_DLLPUBLIC CntWallpaperItem : public SfxPoolItem
{
private:
    OUString           _aURL;
    Color                   _nColor;
    sal_uInt16              _nStyle;

public:
                            TYPEINFO_OVERRIDE();

                            CntWallpaperItem( sal_uInt16 nWhich );
                            CntWallpaperItem( sal_uInt16 nWhich, SvStream& rStream, sal_uInt16 nVersion );
                            CntWallpaperItem( const CntWallpaperItem& rCpy );
                            virtual ~CntWallpaperItem();

    virtual sal_uInt16 GetVersion(sal_uInt16) const override;

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Create( SvStream&, sal_uInt16 nItemVersion ) const override;
    virtual SvStream&       Store( SvStream&, sal_uInt16 nItemVersion ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const override;

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal,
                                         sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue  ( const com::sun::star::uno::Any& rVal,
                                         sal_uInt8 nMemberId ) override;

    void                    SetBitmapURL( const OUString& rURL ) { _aURL = rURL; }
    void                    SetColor( Color nColor ) { _nColor = nColor; }
    void                    SetStyle( sal_uInt16 nStyle ) { _nStyle = nStyle; }

    const OUString&    GetBitmapURL() const { return _aURL; }
    Color                   GetColor() const { return _nColor; }
    sal_uInt16              GetStyle() const { return _nStyle; }
};



#endif // INCLUDED_SVL_CNTWALL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
