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

class DataSequence
{
public:
    typedef mdds::multi_type_vector<mdds::mtv::element_block_func> DataSeriesType;

    // used for fast iteration through data series
    // allows to easily skip empty data ranges
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
    // we might want to switch to multi_type_vector for better memory usage
    // hopefully this vector is empty most of the time
    std::vector< PropertyMap > aPointProps;

    com::sun::star::chart::MissingValueTreatment eMissingValueTreatment;
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
    // length of the data series is min(aXValue.size(), aYValue.size());
    DataSequence aXValue;
    DataSequence aYValue;
    DataSeriesProperties aProperties;
    // also contains bubble chart bubble size
    // apply values to properties with functor
    std::map<OUString, DataSeries> aMappedProperties;
    Axis aXAxis;
    Axis aYAxis;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
