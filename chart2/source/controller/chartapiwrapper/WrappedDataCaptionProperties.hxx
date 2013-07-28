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
#ifndef CHART_WRAPPED_DATACAPTION_PROPERTIES_HXX
#define CHART_WRAPPED_DATACAPTION_PROPERTIES_HXX

#include "WrappedProperty.hxx"
#include "Chart2ModelContact.hxx"

#include <boost/shared_ptr.hpp>
#include <vector>

namespace chart
{
namespace wrapper
{

class WrappedDataCaptionProperties
{
public:
    static void addProperties( ::std::vector< ::com::sun::star::beans::Property > & rOutProperties );
    static void addWrappedPropertiesForSeries( std::vector< WrappedProperty* >& rList
                                    , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    static void addWrappedPropertiesForDiagram( std::vector< WrappedProperty* >& rList
                                    , ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
};

} //namespace wrapper
} //namespace chart

// CHART_WRAPPED_DATACAPTION_PROPERTIES_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
