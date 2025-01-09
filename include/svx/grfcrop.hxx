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

#ifndef INCLUDED_SVX_GRFCROP_HXX
#define INCLUDED_SVX_GRFCROP_HXX

#include <svl/poolitem.hxx>
#include <svx/svxdllapi.h>

class SVXCORE_DLLPUBLIC SvxGrfCrop : public SfxPoolItem
{
    sal_Int32   m_nLeft, m_nRight, m_nTop, m_nBottom;
public:
    DECLARE_ITEM_TYPE_FUNCTION(SvxGrfCrop)
    SvxGrfCrop( TypedWhichId<SvxGrfCrop>);
    SvxGrfCrop( sal_Int32 nLeft,    sal_Int32 nRight,
                sal_Int32 nTop,     sal_Int32 nBottom,
                TypedWhichId<SvxGrfCrop> );
    virtual ~SvxGrfCrop() override;

    SvxGrfCrop(SvxGrfCrop const &) = default;
    SvxGrfCrop(SvxGrfCrop &&) = default;
    SvxGrfCrop & operator =(SvxGrfCrop const &) = delete; // due to SfxPoolItem
    SvxGrfCrop & operator =(SvxGrfCrop &&) = delete; // due to SfxPoolItem

    // "pure virtual methods" from SfxPoolItem
    virtual bool                operator==( const SfxPoolItem& ) const override;
    virtual bool supportsHashCode() const override { return true; }
    virtual size_t hashCode() const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;
    virtual bool QueryValue( css::uno::Any& rVal,
                             sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal,
                           sal_uInt8 nMemberId ) override;

    void SetLeft( sal_Int32 nVal )      { ASSERT_CHANGE_REFCOUNTED_ITEM; m_nLeft = nVal; }
    void SetRight( sal_Int32 nVal )     { ASSERT_CHANGE_REFCOUNTED_ITEM; m_nRight = nVal; }
    void SetTop( sal_Int32 nVal )       { ASSERT_CHANGE_REFCOUNTED_ITEM; m_nTop = nVal; }
    void SetBottom( sal_Int32 nVal )    { ASSERT_CHANGE_REFCOUNTED_ITEM; m_nBottom = nVal; }

    sal_Int32 GetLeft() const           { return m_nLeft; }
    sal_Int32 GetRight() const          { return m_nRight; }
    sal_Int32 GetTop() const            { return m_nTop; }
    sal_Int32 GetBottom() const         { return m_nBottom; }
};

#endif  // INCLUDED_SVX_GRFCROP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
