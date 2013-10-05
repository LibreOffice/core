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

#ifndef OPTIMIZATIONSTATS_HXX
#define OPTIMIZATIONSTATS_HXX

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include "pppoptimizertoken.hxx"
#include <map>

class OptimizationStats
{
    struct Compare
    {
        bool operator()( const PPPOptimizerTokenEnum s1, const PPPOptimizerTokenEnum s2 ) const
        {
            return s1 < s2;
        }
    };
    std::map < PPPOptimizerTokenEnum, com::sun::star::uno::Any, Compare > maStats;

    public :

        OptimizationStats();
        ~OptimizationStats(){};

        void SetStatusValue( const PPPOptimizerTokenEnum eStat, const com::sun::star::uno::Any& rStatValue );
        const com::sun::star::uno::Any* GetStatusValue( const PPPOptimizerTokenEnum eStat ) const;

        void InitializeStatusValuesFromDocument( com::sun::star::uno::Reference< com::sun::star::frame::XModel > rxModel );
        void InitializeStatusValues( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rOptimizationStats );

        com::sun::star::beans::PropertyValues GetStatusSequence();
};


#endif // OPTIMIZATIONSTATS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
