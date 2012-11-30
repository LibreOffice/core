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

#ifndef SC_UNITCONV_HXX
#define SC_UNITCONV_HXX

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_map.hpp>

class ScUnitConverterData
{
    rtl::OUString maIndexString;
    double mfValue;
                            // not implemented
    ScUnitConverterData& operator=( const ScUnitConverterData& );

public:
    ScUnitConverterData( const rtl::OUString& rFromUnit,
        const rtl::OUString& rToUnit, double fValue = 1.0 );
    ScUnitConverterData( const ScUnitConverterData& );
    ~ScUnitConverterData();

    double GetValue() const;
    const rtl::OUString& GetIndexString() const;

    static rtl::OUString BuildIndexString(
        const rtl::OUString& rFromUnit, const rtl::OUString& rToUnit );
};


class ScUnitConverter : public boost::noncopyable
{
    typedef boost::ptr_map<rtl::OUString, ScUnitConverterData> MapType;
    MapType maData;

public:
    ScUnitConverter();
    ~ScUnitConverter();

    bool GetValue(
        double& fValue, const rtl::OUString& rFromUnit, const rtl::OUString& rToUnit ) const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
