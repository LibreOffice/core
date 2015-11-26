/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLPROPERTYMAPPINGCONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLPROPERTYMAPPINGCONTEXT_HXX

#include "transporttypes.hxx"
#include "SchXMLChartContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/SchXMLImportHelper.hxx>

namespace com { namespace sun { namespace star {
    namespace chart2 {
        class XChartDocument;
        class XDataSeries;
    }
}}}

class SchXMLPropertyMappingContext : public SvXMLImportContext
{
public:

    SchXMLPropertyMappingContext( SchXMLImportHelper& rImpHelper,
                          SvXMLImport& rImport, const OUString& rLocalName,
                          tSchXMLLSequencesPerIndex& rLSequencesPerIndex,
                              css::uno::Reference<
                              css::chart2::XDataSeries > xSeries );

    virtual ~SchXMLPropertyMappingContext();

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
private:

    css::uno::Reference< css::chart2::XChartDocument > mxChartDocument;
    SchXMLImportHelper& mrImportHelper;
    css::uno::Reference< css::chart2::XDataSeries > mxDataSeries;

    tSchXMLLSequencesPerIndex& mrLSequencesPerIndex;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
