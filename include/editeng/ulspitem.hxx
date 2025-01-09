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
#ifndef INCLUDED_EDITENG_ULSPITEM_HXX
#define INCLUDED_EDITENG_ULSPITEM_HXX

#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>


// class SvxULSpaceItem --------------------------------------------------

/*  [Description]

    This item describes the Upper- and Lower space of a page or paragraph.
*/

class EDITENG_DLLPUBLIC SvxULSpaceItem final : public SfxPoolItem
{
    sal_uInt16 nUpper;  // Upper space
    sal_uInt16 nLower;  // Lower space
    bool       bContext; // Contextual spacing?
    sal_uInt16 nPropUpper, nPropLower;      // relative or absolute (=100%)
public:
    static SfxPoolItem* CreateDefault();

    DECLARE_ITEM_TYPE_FUNCTION(SvxULSpaceItem)
    explicit SvxULSpaceItem( const sal_uInt16 nId  );
    SvxULSpaceItem( const sal_uInt16 nUp, const sal_uInt16 nLow,
                    const sal_uInt16 nId  );

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

    virtual SvxULSpaceItem*      Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual void                 ScaleMetrics( tools::Long nMult, tools::Long nDiv ) override;
    virtual bool                 HasMetrics() const override;

    inline void SetUpper( const sal_uInt16 nU, const sal_uInt16 nProp = 100 );
    inline void SetLower( const sal_uInt16 nL, const sal_uInt16 nProp = 100 );

    void SetUpperValue( const sal_uInt16 nU ) { ASSERT_CHANGE_REFCOUNTED_ITEM; nUpper = nU; }
    void SetLowerValue( const sal_uInt16 nL ) { ASSERT_CHANGE_REFCOUNTED_ITEM; nLower = nL; }
    void SetContextValue( const bool bC )     { ASSERT_CHANGE_REFCOUNTED_ITEM; bContext = bC; }
    void SetPropUpper( const sal_uInt16 nU )  { ASSERT_CHANGE_REFCOUNTED_ITEM; nPropUpper = nU; }
    void SetPropLower( const sal_uInt16 nL )  { ASSERT_CHANGE_REFCOUNTED_ITEM; nPropLower = nL; }

    sal_uInt16 GetUpper() const { return nUpper; }
    sal_uInt16 GetLower() const { return nLower; }
    bool GetContext() const { return bContext; }
    sal_uInt16 GetPropUpper() const { return nPropUpper; }
    sal_uInt16 GetPropLower() const { return nPropLower; }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
    virtual boost::property_tree::ptree dumpAsJSON() const override;
};

inline void SvxULSpaceItem::SetUpper( const sal_uInt16 nU, const sal_uInt16 nProp )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    nUpper = sal_uInt16((sal_uInt32(nU) * nProp ) / 100); nPropUpper = nProp;
}
inline void SvxULSpaceItem::SetLower( const sal_uInt16 nL, const sal_uInt16 nProp )
{
    ASSERT_CHANGE_REFCOUNTED_ITEM;
    nLower = sal_uInt16((sal_uInt32(nL) * nProp ) / 100); nPropLower = nProp;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
