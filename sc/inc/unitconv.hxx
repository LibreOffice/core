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

#include <rtl/ustring.hxx>

#include <map>

class ScUnitConverterData
{
    OUString const maIndexString;
    double const mfValue;

public:
    ScUnitConverterData( const OUString& rFromUnit,
        const OUString& rToUnit, double fValue );
    ScUnitConverterData( const ScUnitConverterData& ) = default;
    ScUnitConverterData& operator=( const ScUnitConverterData& ) = delete;

    double GetValue() const { return mfValue;}
    const OUString& GetIndexString() const { return maIndexString;}

    static OUString BuildIndexString(
        const OUString& rFromUnit, const OUString& rToUnit );
};

class ScUnitConverter
{
    typedef std::map<OUString, ScUnitConverterData> MapType;
    MapType maData;

public:
    ScUnitConverter();
    ScUnitConverter(const ScUnitConverter&) = delete;
    const ScUnitConverter& operator=(const ScUnitConverter&) = delete;
    ~ScUnitConverter();

    bool GetValue(
        double& fValue, const OUString& rFromUnit, const OUString& rToUnit ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
