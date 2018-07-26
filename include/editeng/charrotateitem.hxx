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
#ifndef INCLUDED_EDITENG_CHARROTATEITEM_HXX
#define INCLUDED_EDITENG_CHARROTATEITEM_HXX

#include <svl/intitem.hxx>
#include <editeng/editengdllapi.h>

 // class SvxTextRotateItem ----------------------------------------------

 /* [Description]

 This item defines a text rotation value. Currently
 text can only be rotated 90,0 and 270,0 degrees.

 */

class EDITENG_DLLPUBLIC SvxTextRotateItem : public SfxUInt16Item
{
public:
    SvxTextRotateItem(sal_uInt16 nValue, const sal_uInt16 nId);

    virtual SfxPoolItem*    Clone(SfxItemPool *pPool = nullptr) const override;
    virtual sal_uInt16      GetVersion(sal_uInt16 nFileVersion) const override;

    virtual bool GetPresentation(SfxItemPresentation ePres,
        MapUnit eCoreMetric,
        MapUnit ePresMetric,
        OUString &rText,
        const IntlWrapper&) const override;

    virtual bool            QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
    virtual bool            PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;

    virtual bool operator==(const SfxPoolItem&) const override;

    // our currently only degree values
    void SetTopToBottom() { SetValue(2700); }
    void SetBottomToTop() { SetValue(900); }
    bool IsTopToBottom() const { return 2700 == GetValue(); }
    bool IsBottomToTop() const { return  900 == GetValue(); }
    bool IsVertical() const     { return IsTopToBottom() || IsBottomToTop(); }

    void dumpAsXml(struct _xmlTextWriter* pWriter) const override;
};


// class SvxCharRotateItem ----------------------------------------------

/* [Description]

    This item defines a character rotation value (0,1 degree). Currently
    character can only be rotated 90,0 and 270,0 degrees.
    The flag FitToLine defines only a UI-Information -
    if true it must also create a SvxCharScaleItem.

*/

class EDITENG_DLLPUBLIC SvxCharRotateItem : public SvxTextRotateItem
{
    bool bFitToLine;
public:
    static SfxPoolItem* CreateDefault();

    SvxCharRotateItem( sal_uInt16 nValue /*= 0*/,
                       bool bFitIntoLine /*= false*/,
                       const sal_uInt16 nId );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual sal_uInt16          GetVersion( sal_uInt16 nFileVersion ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper& ) const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool             operator==( const SfxPoolItem& ) const override;

    bool IsFitToLine() const                { return bFitToLine; }
    void SetFitToLine( bool b )             { bFitToLine = b; }

    void dumpAsXml(struct _xmlTextWriter* pWriter) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
