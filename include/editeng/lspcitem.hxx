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

#include <svl/eitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;

// class SvxLineSpacingItem ----------------------------------------------

/*  [Description]
    This item describes the distance between the lines.
*/

#define LINE_SPACE_DEFAULT_HEIGHT 200
class EDITENG_DLLPUBLIC SvxLineSpacingItem : public SfxEnumItemInterface
{
    short nInterLineSpace;
    sal_uInt16 nLineHeight;
    sal_uInt16 nPropLineSpace;
    SvxLineSpace eLineSpace;
    SvxInterLineSpace eInterLineSpace;

public:
    TYPEINFO_OVERRIDE();
    static SfxPoolItem* CreateDefault();

    // The writer relies on a default height of 200! Actually, I would
    // initialize all values to 0, but who can ignore the consequences in
    // writer? => Rather have a crooked vales as the default, but the
    // programmer sees that there's something special happening.

    SvxLineSpacingItem( sal_uInt16 nHeight /*= LINE_SPACE_DEFAULT_HEIGHT*/, const sal_uInt16 nId  );

    // "pure virtual Methods" from SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const override;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const override;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const override;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion ) const override;

    // Methods to query and edit. InterlineSpace is added to the height.
    inline short GetInterLineSpace() const { return nInterLineSpace; }
    inline void SetInterLineSpace( const short nSpace )
    {
        nInterLineSpace = nSpace;
        eInterLineSpace = SVX_INTER_LINE_SPACE_FIX;
    }

    // Determines the absolute or minimum row height.
    inline sal_uInt16 GetLineHeight() const { return nLineHeight; }
    inline void SetLineHeight( const sal_uInt16 nHeight )
    {
        nLineHeight = nHeight;
        eLineSpace = SVX_LINE_SPACE_MIN;
    }

    // To increase or decrease the row height.
    sal_uInt16 GetPropLineSpace() const { return nPropLineSpace; }
    inline void SetPropLineSpace( const sal_uInt8 nProp )
    {
        nPropLineSpace = nProp;
        eInterLineSpace = SVX_INTER_LINE_SPACE_PROP;
    }

    inline SvxLineSpace &GetLineSpaceRule() { return eLineSpace; }
    inline SvxLineSpace GetLineSpaceRule() const { return eLineSpace; }

    inline SvxInterLineSpace &GetInterLineSpaceRule() { return eInterLineSpace; }
    inline SvxInterLineSpace GetInterLineSpaceRule() const { return eInterLineSpace; }

    virtual sal_uInt16      GetValueCount() const override;
    virtual OUString   GetValueTextByPos( sal_uInt16 nPos ) const override;
    virtual sal_uInt16      GetEnumValue() const override;
    virtual void            SetEnumValue( sal_uInt16 nNewVal ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
