/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "animationtypes.hxx"

#include <com/sun/star/animations/Timing.hpp>

#include "oox/helper/attributelist.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {


Any GetTime(  const OUString & val )
{
    Any aDuration;
    if( val.equalsAscii( "indefinite" ) )
    {
        aDuration <<= Timing_INDEFINITE;
    }
    else
    {
        aDuration <<= val.toFloat() / 1000.0;
    }
    return aDuration;
}



Any GetTimeAnimateValueTime( const OUString & val )
{
    Any aPercent;
    if( val.equalsAscii( "indefinite" ) )
    {
        aPercent <<= Timing_INDEFINITE;
    }
    else
    {
        aPercent <<= val.toFloat() / 100000.0;
    }
    return aPercent;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
