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

#include <drawingml/chart/datasourceconverter.hxx>

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <oox/drawingml/chart/chartconverter.hxx>
#include <oox/drawingml/chart/datasourcemodel.hxx>
#include <oox/token/properties.hxx>
#include <svl/zforlist.hxx>

namespace oox::drawingml::chart {

using namespace ::com::sun::star::chart2::data;
using namespace ::com::sun::star::uno;

DataSequenceConverter::DataSequenceConverter( const ConverterRoot& rParent, DataSequenceModel& rModel ) :
    ConverterBase< DataSequenceModel >( rParent, rModel )
{
}

DataSequenceConverter::~DataSequenceConverter()
{
}

Reference< XDataSequence > DataSequenceConverter::createDataSequence( const OUString& rRole )
{
    // create data sequence from data source model (virtual call at chart converter)
    Reference< XDataSequence > xDataSeq;
    // the internal data table does not support complex labels
    // this is only supported in Calc!!!
    // merge the labels into a single one
    if(rRole == "label")
    {
        mrModel.mnPointCount = std::min<sal_Int32>(mrModel.mnPointCount, 1);
        OUStringBuffer aTitle;
        bool bFirst = true;
        for (auto const& elem : mrModel.maData)
        {
            Any aAny = elem.second;
            if(aAny.has<OUString>())
            {
                if(!bFirst)
                    aTitle.append(" ");

                aTitle.append(aAny.get<OUString>());
                bFirst = false;
            }
        }

        if(!bFirst)
        {
            mrModel.maData.clear();
            mrModel.maData.insert(std::make_pair<sal_Int32, Any>(0, Any(aTitle.makeStringAndClear())));
        }
    }

    bool bDateCategories = (mrModel.meFormatType == SvNumFormatType::DATE) && (rRole == "categories");
    xDataSeq = getChartConverter().createDataSequence(getChartDocument()->getDataProvider(), mrModel,
        rRole, bDateCategories ? u"date"_ustr : u""_ustr);

    // set sequence role
    PropertySet aSeqProp( xDataSeq );
    aSeqProp.setProperty( PROP_Role, rRole );

    const sal_Int32 nKey = getFormatter().getNumberFormatKey(mrModel.maFormatCode);
    if (nKey >= 0)
        aSeqProp.setProperty(PROP_NumberFormatKey, nKey);

    return xDataSeq;
}

DataSourceConverter::DataSourceConverter( const ConverterRoot& rParent, DataSourceModel& rModel ) :
    ConverterBase< DataSourceModel >( rParent, rModel )
{
}

DataSourceConverter::~DataSourceConverter()
{
}

Reference< XDataSequence > DataSourceConverter::createDataSequence( const OUString& rRole )
{
    Reference< XDataSequence > xDataSeq;
    if( mrModel.mxDataSeq.is() )
    {
        DataSequenceConverter aDataSeqConv( *this, *mrModel.mxDataSeq );
        xDataSeq = aDataSeqConv.createDataSequence( rRole );
    }
    return xDataSeq;
}

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
