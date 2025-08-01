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

#ifndef INCLUDED_SVX_XTEXTIT0_HXX
#define INCLUDED_SVX_XTEXTIT0_HXX

#include <svx/xenum.hxx>
#include <svl/eitem.hxx>
#include <svx/svxdllapi.h>

/*************************************************************************
|*
|* FormText-StyleItem
|*
\************************************************************************/

class SVXCORE_DLLPUBLIC XFormTextStyleItem final : public SfxEnumItem<XFormTextStyle>
{
public:
                            static SfxPoolItem* CreateDefault();
                            DECLARE_ITEM_TYPE_FUNCTION(XFormTextStyleItem)
                            XFormTextStyleItem(XFormTextStyle = XFormTextStyle::NONE);
    virtual XFormTextStyleItem* Clone(SfxItemPool* pPool = nullptr) const override;
    // #FontWork#
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
};

/*************************************************************************
|*
|* FormText-HideFormItem
|*
\************************************************************************/

class SVXCORE_DLLPUBLIC XFormTextHideFormItem final : public SfxBoolItem
{
public:
                            static SfxPoolItem* CreateDefault();
                            DECLARE_ITEM_TYPE_FUNCTION(XFormTextHideFormItem)
                            XFormTextHideFormItem(bool bHide = false);
    virtual XFormTextHideFormItem* Clone(SfxItemPool* pPool = nullptr) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
