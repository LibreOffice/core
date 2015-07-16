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

#ifndef INCLUDED_SVX_SDASAITM_HXX
#define INCLUDED_SVX_SDASAITM_HXX

#include <vector>

#include <svl/poolitem.hxx>

class SdrCustomShapeAdjustmentValue
{
    sal_uInt32  nValue;

    friend class SdrCustomShapeAdjustmentItem;
};

class SdrCustomShapeAdjustmentItem : public SfxPoolItem
{
            std::vector<SdrCustomShapeAdjustmentValue>  aAdjustmentValueList;

    public :

            TYPEINFO_OVERRIDE();
            SVX_DLLPUBLIC SdrCustomShapeAdjustmentItem();
            SdrCustomShapeAdjustmentItem( SvStream& rIn, sal_uInt16 nVersion );
            SVX_DLLPUBLIC virtual ~SdrCustomShapeAdjustmentItem();

            virtual bool                operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
            virtual bool GetPresentation(SfxItemPresentation ePresentation,
                                            SfxMapUnit eCoreMetric, SfxMapUnit ePresentationMetric,
                                            OUString &rText, const IntlWrapper * = 0) const SAL_OVERRIDE;
            virtual SfxPoolItem*        Create( SvStream&, sal_uInt16 nItem ) const SAL_OVERRIDE;
            virtual SvStream&           Store( SvStream&, sal_uInt16 nVersion ) const SAL_OVERRIDE;
            virtual SfxPoolItem*        Clone( SfxItemPool* pPool = NULL ) const SAL_OVERRIDE;
            virtual sal_uInt16          GetVersion( sal_uInt16 nFileFormatVersion ) const SAL_OVERRIDE;

            virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;
            virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;

            sal_uInt32                          GetCount() const { return aAdjustmentValueList.size(); };
            SVX_DLLPUBLIC const SdrCustomShapeAdjustmentValue&  GetValue( sal_uInt32 nIndex ) const;
            SVX_DLLPUBLIC void                              SetValue( sal_uInt32 nIndex,
                                                        const SdrCustomShapeAdjustmentValue& rVal );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
