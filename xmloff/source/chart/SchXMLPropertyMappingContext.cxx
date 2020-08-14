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
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/namespacemap.hxx>
#include <SchXMLImport.hxx>

#include <com/sun/star/chart2/data/XLabeledDataSequence2.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>

using namespace com::sun::star;
using namespace com::sun::star::uno;

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
        xSeqProp->setPropertyValue("Role", uno::makeAny( rRole));
    xLabeledSeq->setValues( xSeq );

    Reference< chart2::data::XDataSink > xSink( xSeriesSource, uno::UNO_QUERY );
    if( xSink.is())
    {
        Sequence< Reference< chart2::data::XLabeledDataSequence > > aData( xSeriesSource->getDataSequences());
        aData.realloc( aData.getLength() + 1 );
        aData[ aData.getLength() - 1 ] = xLabeledSeq;
        xSink->setData( aData );
    }

    return xLabeledSeq;
}

}

SchXMLPropertyMappingContext::SchXMLPropertyMappingContext( SchXMLImportHelper& rImpHelper,
        SvXMLImport& rImport, const OUString& rLocalName,
        tSchXMLLSequencesPerIndex & rLSequencesPerIndex,
        uno::Reference<
        chart2::XDataSeries > const & xSeries ):
    SvXMLImportContext( rImport, XML_NAMESPACE_LO_EXT, rLocalName ),
    mrImportHelper( rImpHelper ),
    mxDataSeries(xSeries),
    mrLSequencesPerIndex(rLSequencesPerIndex)
{

}

SchXMLPropertyMappingContext::~SchXMLPropertyMappingContext()
{
}

void SchXMLPropertyMappingContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    OUString aRange;
    OUString aRole;
    // parse attributes
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetPropMappingAttrTokenMap();

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        OUString aValue = xAttrList->getValueByIndex( i );
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_PROPERTY_MAPPING_PROPERTY:
                aRole = aValue;
                break;
            case XML_TOK_PROPERTY_MAPPING_RANGE:
                aRange = aValue;
                break;
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
