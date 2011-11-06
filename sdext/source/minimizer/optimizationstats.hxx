/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
