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
#ifndef INCLUDED_EDITENG_ADJUSTITEM_HXX
#define INCLUDED_EDITENG_ADJUSTITEM_HXX

#include <svl/eitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;

// class SvxAdjustItem ---------------------------------------------------

/*
[Description]
This item describes the row orientation.
*/
#define ADJUST_LASTBLOCK_VERSION        ((sal_uInt16)0x0001)

class EDITENG_DLLPUBLIC SvxAdjustItem : public SfxEnumItemInterface
{
    bool    bLeft      : 1;
    bool    bRight     : 1;
    bool    bCenter    : 1;
    bool    bBlock     : 1;

    // only activ when bBlock
    bool    bOneBlock : 1;
    bool    bLastCenter : 1;
    bool    bLastBlock : 1;

public:
    TYPEINFO_OVERRIDE();

    SvxAdjustItem( const SvxAdjust eAdjst /*= SVX_ADJUST_LEFT*/,
                   const sal_uInt16 nId );

    // "pure virtual Methods" from SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const override;
    virtual sal_uInt16       GetValueCount() const override;
    virtual OUString         GetValueTextByPos( sal_uInt16 nPos ) const override;
    virtual sal_uInt16       GetEnumValue() const override;
    virtual void             SetEnumValue( sal_uInt16 nNewVal ) override;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const override;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const override;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion ) const override;
    virtual sal_uInt16       GetVersion( sal_uInt16 nFileVersion ) const override;

    inline void SetOneWord( const SvxAdjust eType )
    {
        bOneBlock  = eType == SVX_ADJUST_BLOCK;
    }

    inline void SetLastBlock( const SvxAdjust eType )
    {
        bLastBlock = eType == SVX_ADJUST_BLOCK;
        bLastCenter = eType == SVX_ADJUST_CENTER;
    }

    inline void SetAdjust( const SvxAdjust eType )
    {
        bLeft = eType == SVX_ADJUST_LEFT;
        bRight = eType == SVX_ADJUST_RIGHT;
        bCenter = eType == SVX_ADJUST_CENTER;
        bBlock = eType == SVX_ADJUST_BLOCK;
    }

    inline SvxAdjust GetLastBlock() const
    {
        SvxAdjust eRet = SVX_ADJUST_LEFT;

        if ( bLastBlock )
            eRet = SVX_ADJUST_BLOCK;
        else if( bLastCenter )
            eRet = SVX_ADJUST_CENTER;
        return eRet;
    }

    inline SvxAdjust GetOneWord() const
    {
        SvxAdjust eRet = SVX_ADJUST_LEFT;

        if ( bBlock && bOneBlock )
            eRet = SVX_ADJUST_BLOCK;
        return eRet;
    }

    inline SvxAdjust GetAdjust() const
    {
        SvxAdjust eRet = SVX_ADJUST_LEFT;

        if ( bRight )
            eRet = SVX_ADJUST_RIGHT;
        else if ( bCenter )
            eRet = SVX_ADJUST_CENTER;
        else if ( bBlock )
            eRet = SVX_ADJUST_BLOCK;
        return eRet;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
