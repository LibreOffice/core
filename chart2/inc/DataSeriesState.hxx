/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef DATASERIES_HXX
#define DATASERIES_HXX

#include <rtl/ustring.hxx>
#include <mdds/multi_type_vector.hpp>
#include <mdds/multi_type_vector_trait.hpp>
#include <vector>
#include <map>

#include <com/sun/star/uno/Any.h>

namespace chart {

class DataSeries
{
public:
    typedef mdds::multi_type_vector<mdds::mtv::element_block_func> DataSeriesType;

    DataSeriesType getDataSeries();

    size_t size();
    double getValue(size_t nIndex);

private:
    OUString maLabel;
    DataSeriesType maDataSeries;

};

/**
 * point properties overwrite series properties
 */
struct DataSeriesProperties
{
    typedef std::map< OUString, com::sun::star::uno::Any > PropertyMap;
    PropertyMap aSeriesProps;
    std::vector< PropertyMap > aPointProps;
};

struct Axis
{
    double nMin;
    double nMax;
    bool bLog;
    bool bInverseDirection;
};

struct DataSeriesState
{
    DataSeries aXValue;
    DataSeries aYValue;
    DataSeriesProperties aProperties;
    std::map<OUString, DataSeries> aMapProperties;
    Axis aXAxis;
    Axis aYAxis;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
