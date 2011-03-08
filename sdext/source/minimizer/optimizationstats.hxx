/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
