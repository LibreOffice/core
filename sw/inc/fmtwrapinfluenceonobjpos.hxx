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
#ifndef INCLUDED_SW_INC_FMTWRAPINFLUENCEONOBJPOS_HXX
#define INCLUDED_SW_INC_FMTWRAPINFLUENCEONOBJPOS_HXX

#include "hintids.hxx"
#include "format.hxx"
#include "swtypes.hxx"
#include <svl/poolitem.hxx>
#include <com/sun/star/text/WrapInfluenceOnPosition.hpp>

/**
 * Allows positioning of floating screen objects without considering their own wrapping type and the
 * wrapping types of the other floating screen objects.
 *
 * See <http://www.openoffice.org/specs/writer/compatibility/obj-pos-without-wrapping.sxw> for
 * details.
 */
class SW_DLLPUBLIC SwFormatWrapInfluenceOnObjPos final : public SfxPoolItem
{
private:
    sal_Int16 mnWrapInfluenceOnPosition;
    /// Allow objects to overlap, permitted by default.
    bool mbAllowOverlap = true;
    /// Vertical offset added during positioning to avoid an overlap.
    SwTwips mnOverlapVertOffset = 0;

public:

    // #i35017# - constant name has changed
    DECLARE_ITEM_TYPE_FUNCTION(SwFormatWrapInfluenceOnObjPos)
    SwFormatWrapInfluenceOnObjPos(
            sal_Int16 _nWrapInfluenceOnPosition = css::text::WrapInfluenceOnPosition::ONCE_CONCURRENT );
    virtual ~SwFormatWrapInfluenceOnObjPos() override;

    SwFormatWrapInfluenceOnObjPos(SwFormatWrapInfluenceOnObjPos const &) = default;
    SwFormatWrapInfluenceOnObjPos(SwFormatWrapInfluenceOnObjPos &&) = default;
    SwFormatWrapInfluenceOnObjPos & operator =(SwFormatWrapInfluenceOnObjPos const &) = delete; // due to SfxPoolItem
    SwFormatWrapInfluenceOnObjPos & operator =(SwFormatWrapInfluenceOnObjPos &&) = delete; // due to SfxPoolItem

    /// pure virtual methods of class <SfxPoolItem>
    virtual bool operator==( const SfxPoolItem& _rAttr ) const override;
    virtual SwFormatWrapInfluenceOnObjPos* Clone( SfxItemPool* pPool = nullptr ) const override;

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    /// direct accessors to data
    void SetWrapInfluenceOnObjPos( sal_Int16 _nWrapInfluenceOnPosition );
    // #i35017# - add parameter <_bIterativeAsOnceConcurrent>
    /// to control, if value <ITERATIVE> has to be treated as <ONCE_CONCURRENT>
    sal_Int16 GetWrapInfluenceOnObjPos(
                        const bool _bIterativeAsOnceConcurrent = false ) const;

    void SetAllowOverlap(bool bAllowOverlap);
    bool GetAllowOverlap() const;
    void SetOverlapVertOffset(SwTwips nOverlapVertOffset);
    SwTwips GetOverlapVertOffset() const;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

inline const SwFormatWrapInfluenceOnObjPos& SwAttrSet::GetWrapInfluenceOnObjPos(bool bInP) const
    { return Get( RES_WRAP_INFLUENCE_ON_OBJPOS,bInP); }

 inline const SwFormatWrapInfluenceOnObjPos& SwFormat::GetWrapInfluenceOnObjPos(bool bInP) const
    { return m_aSet.GetWrapInfluenceOnObjPos(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
