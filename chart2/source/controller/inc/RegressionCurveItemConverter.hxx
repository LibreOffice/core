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
#pragma once

#include "ItemConverter.hxx"
#include <rtl/ref.hxx>

namespace com::sun::star::lang { class XMultiServiceFactory; }
namespace chart { class DataSeries; }
class SdrModel;

namespace chart::wrapper
{

class RegressionCurveItemConverter final : public ItemConverter
{
public:
    RegressionCurveItemConverter(
        const css::uno::Reference< css::beans::XPropertySet > & rPropertySet,
        rtl::Reference< ::chart::DataSeries > xRegCurveCnt,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        const css::uno::Reference< css::lang::XMultiServiceFactory > & xNamedPropertyContainerFactory );
    virtual ~RegressionCurveItemConverter() override;

    virtual void FillItemSet( SfxItemSet & rOutItemSet ) const override;
    virtual bool ApplyItemSet( const SfxItemSet & rItemSet ) override;

protected:
    virtual const WhichRangesContainer& GetWhichPairs() const override;
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const override;

    virtual void FillSpecialItem( sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const override;
    virtual bool ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet & rItemSet ) override;

private:
    std::shared_ptr< ItemConverter >  m_spGraphicConverter;
    rtl::Reference< ::chart::DataSeries >  m_xCurveContainer;
};

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
