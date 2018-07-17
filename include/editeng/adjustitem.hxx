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
#define ADJUST_LASTBLOCK_VERSION        (sal_uInt16(0x0001))

class EDITENG_DLLPUBLIC SvxAdjustItem : public SfxEnumItemInterface
{
    bool    bLeft      : 1;
    bool    bRight     : 1;
    bool    bCenter    : 1;
    bool    bBlock     : 1;

    // only active when bBlock
    bool    bOneBlock : 1;
    bool    bLastCenter : 1;
    bool    bLastBlock : 1;

public:
    static SfxPoolItem* CreateDefault();

    SvxAdjustItem( const SvxAdjust eAdjst /*= SvxAdjust::Left*/,
                   const sal_uInt16 nId );

    // "pure virtual Methods" from SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;
    virtual sal_uInt16       GetValueCount() const override;
    OUString                 GetValueTextByPos( sal_uInt16 nPos ) const;
    virtual sal_uInt16       GetEnumValue() const override;
    virtual void             SetEnumValue( sal_uInt16 nNewVal ) override;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const override;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion ) const override;
    virtual sal_uInt16       GetVersion( sal_uInt16 nFileVersion ) const override;

    void SetOneWord( const SvxAdjust eType )
    {
        bOneBlock  = eType == SvxAdjust::Block;
    }

    void SetLastBlock( const SvxAdjust eType )
    {
        bLastBlock = eType == SvxAdjust::Block;
        bLastCenter = eType == SvxAdjust::Center;
    }

    void SetAdjust( const SvxAdjust eType )
    {
        bLeft = eType == SvxAdjust::Left;
        bRight = eType == SvxAdjust::Right;
        bCenter = eType == SvxAdjust::Center;
        bBlock = eType == SvxAdjust::Block;
    }

    SvxAdjust GetLastBlock() const
    {
        SvxAdjust eRet = SvxAdjust::Left;

        if ( bLastBlock )
            eRet = SvxAdjust::Block;
        else if( bLastCenter )
            eRet = SvxAdjust::Center;
        return eRet;
    }

    SvxAdjust GetOneWord() const
    {
        SvxAdjust eRet = SvxAdjust::Left;

        if ( bBlock && bOneBlock )
            eRet = SvxAdjust::Block;
        return eRet;
    }

    SvxAdjust GetAdjust() const
    {
        SvxAdjust eRet = SvxAdjust::Left;

        if ( bRight )
            eRet = SvxAdjust::Right;
        else if ( bCenter )
            eRet = SvxAdjust::Center;
        else if ( bBlock )
            eRet = SvxAdjust::Block;
        return eRet;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
