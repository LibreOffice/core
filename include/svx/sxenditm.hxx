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
#ifndef INCLUDED_SVX_SXENDITM_HXX
#define INCLUDED_SVX_SXENDITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/svxdllapi.h>

class SVXCORE_DLLPUBLIC SdrEdgeNode1HorzDistItem final : public SdrMetricItem {
public:
    SdrEdgeNode1HorzDistItem(tools::Long nVal): SdrMetricItem(SDRATTR_EDGENODE1HORZDIST,nVal)  {}
    SAL_DLLPRIVATE virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    SAL_DLLPRIVATE virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    SAL_DLLPRIVATE virtual SdrEdgeNode1HorzDistItem* Clone(SfxItemPool* pPool=nullptr) const override;
};

class SVXCORE_DLLPUBLIC SdrEdgeNode1VertDistItem final : public SdrMetricItem {
public:
    SdrEdgeNode1VertDistItem(tools::Long nVal): SdrMetricItem(SDRATTR_EDGENODE1VERTDIST,nVal)  {}
    SAL_DLLPRIVATE virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    SAL_DLLPRIVATE virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    SAL_DLLPRIVATE virtual SdrEdgeNode1VertDistItem* Clone(SfxItemPool* pPool=nullptr) const override;
};

class SVXCORE_DLLPUBLIC SdrEdgeNode2HorzDistItem final : public SdrMetricItem {
public:
    SdrEdgeNode2HorzDistItem(tools::Long nVal): SdrMetricItem(SDRATTR_EDGENODE2HORZDIST,nVal)  {}
    SAL_DLLPRIVATE virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    SAL_DLLPRIVATE virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    SAL_DLLPRIVATE virtual SdrEdgeNode2HorzDistItem* Clone(SfxItemPool* pPool=nullptr) const override;
};

class SVXCORE_DLLPUBLIC SdrEdgeNode2VertDistItem final : public SdrMetricItem {
public:
    SdrEdgeNode2VertDistItem(tools::Long nVal): SdrMetricItem(SDRATTR_EDGENODE2VERTDIST,nVal)  {}
    SAL_DLLPRIVATE virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    SAL_DLLPRIVATE virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    SAL_DLLPRIVATE virtual SdrEdgeNode2VertDistItem* Clone(SfxItemPool* pPool=nullptr) const override;
};

class SdrEdgeNode1GlueDistItem final : public SdrMetricItem {
public:
    SdrEdgeNode1GlueDistItem(tools::Long nVal=0): SdrMetricItem(SDRATTR_EDGENODE1GLUEDIST,nVal)  {}
    virtual SdrEdgeNode1GlueDistItem* Clone(SfxItemPool* pPool=nullptr) const override;
};

class SdrEdgeNode2GlueDistItem final : public SdrMetricItem {
public:
    SdrEdgeNode2GlueDistItem(tools::Long nVal=0): SdrMetricItem(SDRATTR_EDGENODE2GLUEDIST,nVal)  {}
    virtual SdrEdgeNode2GlueDistItem* Clone(SfxItemPool* pPool=nullptr) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
