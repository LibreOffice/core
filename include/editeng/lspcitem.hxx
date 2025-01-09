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
#ifndef INCLUDED_EDITENG_LSPCITEM_HXX
#define INCLUDED_EDITENG_LSPCITEM_HXX

#include <svl/cenumitm.hxx>
#include <tools/mapunit.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/editengdllapi.h>

// class SvxLineSpacingItem ----------------------------------------------

/*  [Description]
    This item describes the distance between the lines.
*/

#define LINE_SPACE_DEFAULT_HEIGHT 200
class EDITENG_DLLPUBLIC SvxLineSpacingItem final : public SfxEnumItemInterface
{
    short                 nInterLineSpace;
    sal_uInt16            nLineHeight;
    sal_uInt16            nPropLineSpace;
    SvxLineSpaceRule      eLineSpaceRule;
    SvxInterLineSpaceRule eInterLineSpaceRule;

public:
    static SfxPoolItem* CreateDefault();

    // The writer relies on a default height of 200! Actually, I would
    // initialize all values to 0, but who can ignore the consequences in
    // writer? => Rather have a crooked vales as the default, but the
    // programmer sees that there's something special happening.

    DECLARE_ITEM_TYPE_FUNCTION(SvxLineSpacingItem)
    SvxLineSpacingItem( sal_uInt16 nLineHeight /*= LINE_SPACE_DEFAULT_HEIGHT*/, const sal_uInt16 nId  );

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

    virtual SvxLineSpacingItem* Clone( SfxItemPool *pPool = nullptr ) const override;

    // Methods to query and edit. InterlineSpace is added to the height.
    short GetInterLineSpace() const { return nInterLineSpace; }
    void SetInterLineSpace( const short nSpace )
    {
        ASSERT_CHANGE_REFCOUNTED_ITEM;
        nInterLineSpace = nSpace;
        eInterLineSpaceRule = SvxInterLineSpaceRule::Fix;
    }

    // Determines the absolute or minimum row height.
    sal_uInt16 GetLineHeight() const { return nLineHeight; }
    void SetLineHeight( const sal_uInt16 nHeight )
    {
        ASSERT_CHANGE_REFCOUNTED_ITEM;
        nLineHeight = nHeight;
        eLineSpaceRule = SvxLineSpaceRule::Min;
    }

    // To increase or decrease the row height.
    sal_uInt16 GetPropLineSpace() const { return nPropLineSpace; }
    void SetPropLineSpace( const sal_uInt16 nProp )
    {
        ASSERT_CHANGE_REFCOUNTED_ITEM;
        nPropLineSpace = nProp;
        eInterLineSpaceRule = SvxInterLineSpaceRule::Prop;
    }

    void SetLineSpaceRule(SvxLineSpaceRule e) { ASSERT_CHANGE_REFCOUNTED_ITEM; eLineSpaceRule = e; }
    SvxLineSpaceRule GetLineSpaceRule() const { return eLineSpaceRule; }

    void SetInterLineSpaceRule(SvxInterLineSpaceRule e) { ASSERT_CHANGE_REFCOUNTED_ITEM; eInterLineSpaceRule = e; }
    SvxInterLineSpaceRule GetInterLineSpaceRule() const { return eInterLineSpaceRule; }

    virtual sal_uInt16      GetValueCount() const override;
    virtual sal_uInt16      GetEnumValue() const override;
    virtual void            SetEnumValue( sal_uInt16 nNewVal ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
