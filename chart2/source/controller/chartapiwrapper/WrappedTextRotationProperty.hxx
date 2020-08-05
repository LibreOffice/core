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

#include <WrappedProperty.hxx>

namespace chart
{

class WrappedTextRotationProperty final : public WrappedProperty
{
public:
    explicit WrappedTextRotationProperty( bool bDirectState=false );
    virtual ~WrappedTextRotationProperty() override;

    virtual css::beans::PropertyState getPropertyState( const css::uno::Reference< css::beans::XPropertyState >& xInnerPropertyState ) const override;

private:
    virtual css::uno::Any convertInnerToOuterValue( const css::uno::Any& rInnerValue ) const override;
    virtual css::uno::Any convertOuterToInnerValue( const css::uno::Any& rOuterValue ) const override;

    bool m_bDirectState;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
