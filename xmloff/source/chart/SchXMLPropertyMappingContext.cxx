/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SchXMLPropertyMappingContext.hxx"
#include "SchXMLTools.hxx"
#include <utility>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlimp.hxx>
#include <sal/log.hxx>

#include <com/sun/star/chart2/data/XLabeledDataSequence2.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace ::xmloff::token;

namespace {

Reference< chart2::data::XLabeledDataSequence2 > createAndAddSequenceToSeries( const OUString& rRole
        , const OUString& rRange
        , const Reference< chart2::XChartDocument >& xChartDoc
        , const Reference< chart2::XDataSeries >& xSeries )
{
    Reference< chart2::data::XLabeledDataSequence2 > xLabeledSeq;

    Reference< chart2::data::XDataSource > xSeriesSource( xSeries,uno::UNO_QUERY );

    if( !(!rRange.isEmpty() && xChartDoc.is() && xSeriesSource.is()) )
        return xLabeledSeq;

    // create a new sequence
    xLabeledSeq = SchXMLTools::GetNewLabeledDataSequence();

    // set values at the new sequence
    Reference< chart2::data::XDataSequence > xSeq = SchXMLTools::CreateDataSequence( rRange, xChartDoc );
    Reference< beans::XPropertySet > xSeqProp( xSeq, uno::UNO_QUERY );
    if( xSeqProp.is())
        xSeqProp->setPropertyValue(u"Role"_ustr, uno::Any( rRole));
    xLabeledSeq->setValues( xSeq );

    Reference< chart2::data::XDataSink > xSink( xSeriesSource, uno::UNO_QUERY );
    if( xSink.is())
    {
        Sequence< Reference< chart2::data::XLabeledDataSequence > > aData( xSeriesSource->getDataSequences());
        aData.realloc( aData.getLength() + 1 );
        aData.getArray()[ aData.getLength() - 1 ] = xLabeledSeq;
        xSink->setData( aData );
    }

    return xLabeledSeq;
}

}

SchXMLPropertyMappingContext::SchXMLPropertyMappingContext(
        SvXMLImport& rImport,
        tSchXMLLSequencesPerIndex & rLSequencesPerIndex,
        uno::Reference<
        chart2::XDataSeries > xSeries ):
    SvXMLImportContext( rImport ),
    mxDataSeries(std::move(xSeries)),
    mrLSequencesPerIndex(rLSequencesPerIndex)
{

}

SchXMLPropertyMappingContext::~SchXMLPropertyMappingContext()
{
}

void SchXMLPropertyMappingContext::startFastElement (sal_Int32 /*nElement*/,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    OUString aRange;
    OUString aRole;
    // parse attributes
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        OUString aValue = aIter.toString();
        switch( aIter.getToken() )
        {
            case XML_ELEMENT(LO_EXT, XML_PROPERTY):
                aRole = aValue;
                break;
            case XML_ELEMENT(LO_EXT, XML_CELL_RANGE_ADDRESS):
                aRange = aValue;
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    if( !aRange.isEmpty() && !aRole.isEmpty() )
    {
        Reference< chart2::XChartDocument > xChartDoc( GetImport().GetModel(), uno::UNO_QUERY );
        Reference< chart2::data::XLabeledDataSequence2 > xSeq =
            createAndAddSequenceToSeries(aRole, aRange, xChartDoc, mxDataSeries);
        mrLSequencesPerIndex.emplace(
                    tSchXMLIndexWithPart( 0, SCH_XML_PART_VALUES),
                    xSeq);
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
