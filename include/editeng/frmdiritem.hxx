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
#ifndef INCLUDED_EDITENG_FRMDIRITEM_HXX
#define INCLUDED_EDITENG_FRMDIRITEM_HXX

#include <svl/intitem.hxx>
#include <editeng/frmdir.hxx>
#include <editeng/editengdllapi.h>

// class SvxFrameDirectionItem ----------------------------------------------

/* [Description]

    This item defines a frame direction, which place the content inside
    a frame. It exist different kind of directions which are used to the
    layout text for Western, CJK and CTL languages.
*/

class EDITENG_DLLPUBLIC SvxFrameDirectionItem : public SfxUInt16Item
{
public:
    TYPEINFO_OVERRIDE();

    SvxFrameDirectionItem( SvxFrameDirection nValue /*= FRMDIR_HORI_LEFT_TOP*/,
                            sal_uInt16 nWhich  );
    virtual ~SvxFrameDirectionItem();

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const override;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const override;
    virtual SvStream&       Store(SvStream & rStrm, sal_uInt16 nIVer) const override;
    virtual sal_uInt16          GetVersion( sal_uInt16 nFileVersion ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper * = 0 ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    inline SvxFrameDirectionItem& operator=( const SvxFrameDirectionItem& rItem )
    {
        SetValue( rItem.GetValue() );
        return *this;
    }
};

#endif // INCLUDED_EDITENG_FRMDIRITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
