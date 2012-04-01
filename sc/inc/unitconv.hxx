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
