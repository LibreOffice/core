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

#include <svl/cenumitm.hxx>
#include <svl/poolitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/editengdllapi.h>

// class SvxAdjustItem ---------------------------------------------------

/*
[Description]
This item describes the row orientation.
*/
constexpr sal_uInt16 ADJUST_LASTBLOCK_VERSION = 0x0001;

class EDITENG_DLLPUBLIC SvxAdjustItem final : public SfxEnumItemInterface
{
    bool    bLeft      : 1;
    bool    bRight     : 1;
    bool    bCenter    : 1;
    bool    bBlock     : 1;

    // only active when bBlock
    bool    bOneBlock : 1;
    bool    bLastCenter : 1;
    bool    bLastBlock : 1;

protected:
    virtual ItemInstanceManager* getItemInstanceManager() const override;

public:
    static SfxPoolItem* CreateDefault();

    DECLARE_ITEM_TYPE_FUNCTION(SvxAdjustItem)
    SvxAdjustItem( const SvxAdjust eAdjst /*= SvxAdjust::Left*/,
                   const sal_uInt16 nId );

    // "pure virtual Methods" from SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool            supportsHashCode() const override { return true; }
    virtual size_t          hashCode() const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;
    virtual sal_uInt16       GetValueCount() const override;
    static OUString          GetValueTextByPos( sal_uInt16 nPos );
    virtual sal_uInt16       GetEnumValue() const override;
    virtual void             SetEnumValue( sal_uInt16 nNewVal ) override;
    virtual SvxAdjustItem*   Clone( SfxItemPool *pPool = nullptr ) const override;

    void SetOneWord( const SvxAdjust eType )
    {
        ASSERT_CHANGE_REFCOUNTED_ITEM;
        bOneBlock  = eType == SvxAdjust::Block;
    }

    void SetLastBlock( const SvxAdjust eType )
    {
        ASSERT_CHANGE_REFCOUNTED_ITEM;
        bLastBlock = eType == SvxAdjust::Block;
        bLastCenter = eType == SvxAdjust::Center;
    }

    void SetAdjust( const SvxAdjust eType )
    {
        ASSERT_CHANGE_REFCOUNTED_ITEM;
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

    sal_Int8 GetAsFlags() const
    {
        sal_Int8 nFlags = 0;
        if ( bOneBlock )
            nFlags |= 0x0001;
        if ( bLastCenter )
            nFlags |= 0x0002;
        if ( bLastBlock )
            nFlags |= 0x0004;
        return nFlags;
    }

    void SetAsFlags(sal_Int8 nFlags)
    {
        ASSERT_CHANGE_REFCOUNTED_ITEM;
        bOneBlock = 0 != (nFlags & 0x0001);
        bLastCenter = 0 != (nFlags & 0x0002);
        bLastBlock = 0 != (nFlags & 0x0004);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
