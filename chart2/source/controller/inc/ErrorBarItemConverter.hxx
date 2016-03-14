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

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_INC_ERRORBARITEMCONVERTER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_INC_ERRORBARITEMCONVERTER_HXX

#include "ItemConverter.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include "NumberFormatterWrapper.hxx"

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <memory>
#include <vector>

class SdrModel;

namespace chart
{
namespace wrapper
{

class ErrorBarItemConverter : public ItemConverter
{
public:
    ErrorBarItemConverter(
        const css::uno::Reference< css::frame::XModel > & xChartModel,
        const css::uno::Reference< css::beans::XPropertySet > & rPropertySet,
        SfxItemPool& rItemPool,
        SdrModel& rDrawModel,
        const css::uno::Reference< css::lang::XMultiServiceFactory > & xNamedPropertyContainerFactory );
    virtual ~ErrorBarItemConverter();

    virtual void FillItemSet( SfxItemSet & rOutItemSet ) const override;
    virtual bool ApplyItemSet( const SfxItemSet & rItemSet ) override;

protected:
    virtual const sal_uInt16 * GetWhichPairs() const override;
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const override;

    virtual void FillSpecialItem( sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
        throw( css::uno::Exception ) override;
    virtual bool ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
        throw( css::uno::Exception ) override;

private:
    std::shared_ptr< ItemConverter >  m_spGraphicConverter;
    css::uno::Reference< css::frame::XModel >  m_xModel;
};

} //  namespace wrapper
} //  namespace chart

#endif // INCLUDED_CHART2_SOURCE_CONTROLLER_INC_ERRORBARITEMCONVERTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
