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
#ifndef INCLUDED_EDITENG_HYPHENZONEITEM_HXX
#define INCLUDED_EDITENG_HYPHENZONEITEM_HXX

#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;

// class SvxHyphenZoneItem -----------------------------------------------

/*  [Description]

    This item describes a hyphenation attribute  (automatic?, number of
    characters at the end of the line and start).
*/

class EDITENG_DLLPUBLIC SvxHyphenZoneItem : public SfxPoolItem
{
    bool      bHyphen  : 1;
    bool      bPageEnd : 1;
    sal_uInt8 nMinLead;
    sal_uInt8 nMinTrail;
    sal_uInt8 nMaxHyphens;

public:
    static SfxPoolItem* CreateDefault();

    SvxHyphenZoneItem( const bool bHyph /*= false*/,
                       const sal_uInt16 nId  );

    // "pure virtual Methods" from SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;

    void SetHyphen( const bool bNew ) { bHyphen = bNew; }
    bool IsHyphen() const { return bHyphen; }

    void SetPageEnd( const bool bNew ) { bPageEnd = bNew; }
    bool IsPageEnd() const { return bPageEnd; }

    sal_uInt8 &GetMinLead() { return nMinLead; }
    sal_uInt8 GetMinLead() const { return nMinLead; }

    sal_uInt8 &GetMinTrail() { return nMinTrail; }
    sal_uInt8 GetMinTrail() const { return nMinTrail; }

    sal_uInt8 &GetMaxHyphens() { return nMaxHyphens; }
    sal_uInt8 GetMaxHyphens() const { return nMaxHyphens; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
