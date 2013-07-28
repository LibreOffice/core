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
#ifndef _CHART2_TICKMARKS_DATES_HXX
#define _CHART2_TICKMARKS_DATES_HXX

#include "Tickmarks.hxx"

namespace chart
{

class DateTickFactory
{
public:
    DateTickFactory(
         const ExplicitScaleData& rScale
        , const ExplicitIncrementData& rIncrement );
    ~DateTickFactory();

    void getAllTicks( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const;
    void getAllTicksShifted( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const;

private: //methods
    void getAllTicks( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos, bool bShifted ) const;

private: //member
    ExplicitScaleData     m_aScale;
    ExplicitIncrementData m_aIncrement;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XScaling >
                                                m_xInverseScaling;

    //minimum and maximum of the visible range after scaling
    double    m_fScaledVisibleMin;
    double    m_fScaledVisibleMax;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
