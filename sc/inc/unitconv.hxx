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

#ifndef INCLUDED_SC_INC_UNITCONV_HXX
#define INCLUDED_SC_INC_UNITCONV_HXX

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_map.hpp>

class ScUnitConverterData
{
    OUString maIndexString;
    double mfValue;
    ScUnitConverterData& operator=( const ScUnitConverterData& ) = delete;

public:
    ScUnitConverterData( const OUString& rFromUnit,
        const OUString& rToUnit, double fValue = 1.0 );
    ScUnitConverterData( const ScUnitConverterData& );
    ~ScUnitConverterData();

    double GetValue() const { return mfValue;}
    const OUString& GetIndexString() const { return maIndexString;}

    static OUString BuildIndexString(
        const OUString& rFromUnit, const OUString& rToUnit );
};

class ScUnitConverter : public boost::noncopyable
{
    typedef boost::ptr_map<OUString, ScUnitConverterData> MapType;
    MapType maData;

public:
    ScUnitConverter();
    ~ScUnitConverter();

    bool GetValue(
        double& fValue, const OUString& rFromUnit, const OUString& rToUnit ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
