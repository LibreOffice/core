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
#ifndef INCLUDED_SW_INC_FMTORNT_HXX
#define INCLUDED_SW_INC_FMTORNT_HXX

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include "swdllapi.h"
#include "hintids.hxx"
#include "swtypes.hxx"
#include "format.hxx"
#include <svl/poolitem.hxx>

class IntlWrapper;

/// Defines the vertical position of a fly frame.
///
/// For example: from top (orientation), by 1cm (relative position), to the entire page (relation).
class SW_DLLPUBLIC SwFormatVertOrient final : public SfxPoolItem
{
    SwTwips         m_nYPos;  ///< Contains *always* the current RelPos.
    sal_Int16       m_eOrient;
    sal_Int16       m_eRelation;
public:
    DECLARE_ITEM_TYPE_FUNCTION(SwFormatVertOrient)
    SwFormatVertOrient( SwTwips nY = 0, sal_Int16 eVert = css::text::VertOrientation::NONE,
                     sal_Int16 eRel = css::text::RelOrientation::PRINT_AREA );
    SwFormatVertOrient(SwFormatVertOrient const &) = default; // SfxPoolItem copy function dichotomy

    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual size_t          hashCode() const override;
    virtual SwFormatVertOrient* Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper& rIntl ) const override;
    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    sal_Int16 GetVertOrient() const { return m_eOrient; }
    sal_Int16 GetRelationOrient() const { return m_eRelation; }
    void   SetVertOrient( sal_Int16 eNew )
    { ASSERT_CHANGE_REFCOUNTED_ITEM; m_eOrient = eNew; }
    void   SetRelationOrient( sal_Int16 eNew )
    { ASSERT_CHANGE_REFCOUNTED_ITEM; m_eRelation = eNew; }

    SwTwips GetPos() const { return m_nYPos; }
    void    SetPos( SwTwips nNew )
    { ASSERT_CHANGE_REFCOUNTED_ITEM; m_nYPos = nNew; }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;

protected:
    virtual ItemInstanceManager* getItemInstanceManager() const override;
};

/// Defines the horizontal position of a fly frame.
///
/// For example: from left (orientation), by 1 cm (relative position), to the entire page
/// (relation).
class SW_DLLPUBLIC SwFormatHoriOrient final : public SfxPoolItem
{
    SwTwips         m_nXPos;              ///< Contains *always* the current RelPos.
    sal_Int16       m_eOrient;
    sal_Int16       m_eRelation;
    bool            m_bPosToggle : 1; ///< Flip position on even pages.
public:
    DECLARE_ITEM_TYPE_FUNCTION(SwFormatHoriOrient)
    SwFormatHoriOrient( SwTwips nX = 0, sal_Int16 eHori = css::text::HoriOrientation::NONE,
        sal_Int16 eRel = css::text::RelOrientation::PRINT_AREA, bool bPos = false );
    SwFormatHoriOrient(SwFormatHoriOrient const &) = default; // SfxPoolItem copy function dichotomy

    /// "Pure virtual methods" of SfxPoolItem.
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual size_t          hashCode() const override;
    virtual SwFormatHoriOrient* Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;
    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    sal_Int16 GetHoriOrient() const { return m_eOrient; }
    sal_Int16 GetRelationOrient() const { return m_eRelation; }
    void SetHoriOrient( sal_Int16 eNew ) { ASSERT_CHANGE_REFCOUNTED_ITEM; m_eOrient = eNew; }
    void SetRelationOrient( sal_Int16 eNew ) { ASSERT_CHANGE_REFCOUNTED_ITEM; m_eRelation = eNew; }

    SwTwips GetPos() const { return m_nXPos; }
    void    SetPos( SwTwips nNew ) { ASSERT_CHANGE_REFCOUNTED_ITEM; m_nXPos = nNew; }

    bool IsPosToggle() const { return m_bPosToggle; }
    void SetPosToggle( bool bNew ) { ASSERT_CHANGE_REFCOUNTED_ITEM; m_bPosToggle = bNew; }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;

protected:
    virtual ItemInstanceManager* getItemInstanceManager() const override;
};

inline const SwFormatVertOrient &SwAttrSet::GetVertOrient(bool bInP) const
    { return Get( RES_VERT_ORIENT,bInP); }
inline const SwFormatHoriOrient &SwAttrSet::GetHoriOrient(bool bInP) const
    { return Get( RES_HORI_ORIENT,bInP); }

inline const SwFormatVertOrient &SwFormat::GetVertOrient(bool bInP) const
    { return m_aSet.GetVertOrient(bInP); }
inline const SwFormatHoriOrient &SwFormat::GetHoriOrient(bool bInP) const
    { return m_aSet.GetHoriOrient(bInP); }

namespace sw {

    bool GetAtPageRelOrientation(sal_Int16 & rOrientation, bool const isIgnorePrintArea);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
