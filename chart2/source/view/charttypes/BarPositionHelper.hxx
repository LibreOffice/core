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

#include <PlottingPositionHelper.hxx>
#include "CategoryPositionHelper.hxx"

namespace chart
{

class BarPositionHelper : public CategoryPositionHelper, public PlottingPositionHelper
{
public:
    explicit BarPositionHelper();
    BarPositionHelper( const BarPositionHelper& rSource );
    virtual ~BarPositionHelper() override;

    virtual std::unique_ptr<PlottingPositionHelper> clone() const override;

    void updateSeriesCount( double fSeriesCount ); /*only enter the size of x stacked series*/

    virtual double getScaledSlotPos( double fCategoryX, double fSeriesNumber ) const override;
    virtual void setScaledCategoryWidth( double fScaledCategoryWidth ) override;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
