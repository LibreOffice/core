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

class SVX_DLLPUBLIC SdrEdgeNode1HorzDistItem: public SdrMetricItem {
public:
    SdrEdgeNode1HorzDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE1HORZDIST,nVal)  {}
    SdrEdgeNode1HorzDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE1HORZDIST,rIn) {}
    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
};

class SVX_DLLPUBLIC SdrEdgeNode1VertDistItem: public SdrMetricItem {
public:
    SdrEdgeNode1VertDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE1VERTDIST,nVal)  {}
    SdrEdgeNode1VertDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE1VERTDIST,rIn) {}
    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
};

class SVX_DLLPUBLIC SdrEdgeNode2HorzDistItem: public SdrMetricItem {
public:
    SdrEdgeNode2HorzDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE2HORZDIST,nVal)  {}
    SdrEdgeNode2HorzDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE2HORZDIST,rIn) {}
    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
};

class SVX_DLLPUBLIC SdrEdgeNode2VertDistItem: public SdrMetricItem {
public:
    SdrEdgeNode2VertDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE2VERTDIST,nVal)  {}
    SdrEdgeNode2VertDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE2VERTDIST,rIn) {}
    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
};

class SdrEdgeNode1GlueDistItem: public SdrMetricItem {
public:
    SdrEdgeNode1GlueDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE1GLUEDIST,nVal)  {}
    SdrEdgeNode1GlueDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE1GLUEDIST,rIn) {}
};

class SdrEdgeNode2GlueDistItem: public SdrMetricItem {
public:
    SdrEdgeNode2GlueDistItem(long nVal=0): SdrMetricItem(SDRATTR_EDGENODE2GLUEDIST,nVal)  {}
    SdrEdgeNode2GlueDistItem(SvStream& rIn): SdrMetricItem(SDRATTR_EDGENODE2GLUEDIST,rIn) {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
