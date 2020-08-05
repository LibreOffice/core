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

#include <WrappedDefaultProperty.hxx>

#include <memory>

namespace chart::wrapper { class Chart2ModelContact; }

namespace chart::wrapper
{

class WrappedBarPositionProperty_Base : public WrappedDefaultProperty
{
public:
    WrappedBarPositionProperty_Base(
              const OUString& rOuterName
            , const OUString& rInnerSequencePropertyName
            , sal_Int32 nDefaultValue
            , const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact );
    virtual ~WrappedBarPositionProperty_Base() override;

    virtual void setPropertyValue( const css::uno::Any& rOuterValue, const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    virtual css::uno::Any getPropertyValue( const css::uno::Reference< css::beans::XPropertySet >& xInnerPropertySet ) const override;

    void setDimensionAndAxisIndex( sal_Int32 nDimensionIndex, sal_Int32 nAxisIndex );

protected:
    sal_Int32 m_nDimensionIndex;
    sal_Int32 m_nAxisIndex;
    std::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;

    sal_Int32                               m_nDefaultValue;
    OUString                                m_InnerSequencePropertyName;

    mutable css::uno::Any                   m_aOuterValue;
};

class WrappedGapwidthProperty : public WrappedBarPositionProperty_Base
{
public:
    explicit WrappedGapwidthProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);
    virtual ~WrappedGapwidthProperty() override;
};

class WrappedBarOverlapProperty : public WrappedBarPositionProperty_Base
{
public:
    explicit WrappedBarOverlapProperty(const std::shared_ptr<Chart2ModelContact>& spChart2ModelContact);
    virtual ~WrappedBarOverlapProperty() override;
};

} //  namespace chart::wrapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
