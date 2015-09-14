/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_WRAPPEDGL3DPROPERTIES_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_CHARTAPIWRAPPER_WRAPPEDGL3DPROPERTIES_HXX

#include <memory>
#include <vector>

#include <com/sun/star/beans/Property.hpp>

namespace chart {

class WrappedProperty;

namespace wrapper {

class Chart2ModelContact;

class WrappedGL3DProperties
{
public:
    static void addProperties( std::vector<css::beans::Property> & rOutProps );
    static void addWrappedProperties(
        std::vector<WrappedProperty*>& rList, const std::shared_ptr<Chart2ModelContact>& pChart2ModelContact );
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
