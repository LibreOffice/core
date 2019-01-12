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
#ifndef INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLAXISCONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLAXISCONTEXT_HXX

#include <SchXMLImport.hxx>
#include "transporttypes.hxx"

#include <com/sun/star/chart/XChartDocument.hpp>

class SchXMLAxisContext : public SvXMLImportContext
{
public:
    SchXMLAxisContext( SchXMLImportHelper& rImpHelper,
                       SvXMLImport& rImport, const OUString& rLocalName,
                       css::uno::Reference< css::chart::XDiagram > const & xDiagram,
                       std::vector< SchXMLAxis >& aAxes,
                       OUString& rCategoriesAddress,
                       bool bAddMissingXAxisForNetCharts,
                       bool bAdaptWrongPercentScaleValues,
                       bool bAdaptXAxisOrientationForOld2DBarCharts,
                       bool& rbAxisPositionAttributeImported );
    virtual ~SchXMLAxisContext() override;

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual void EndElement() override;
    virtual SvXMLImportContextRef CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

    static void CorrectAxisPositions( const css::uno::Reference< css::chart2::XChartDocument >& xNewDoc,
                          const OUString& rChartTypeServiceName,
                          const OUString& rODFVersionOfFile,
                          bool bAxisPositionAttributeImported );

private:
    SchXMLImportHelper& m_rImportHelper;
    css::uno::Reference< css::chart::XDiagram > m_xDiagram;
    SchXMLAxis m_aCurrentAxis;
    std::vector< SchXMLAxis >& m_rAxes;
    css::uno::Reference< css::beans::XPropertySet > m_xAxisProps;
    OUString m_aAutoStyleName;
    OUString& m_rCategoriesAddress;
    sal_Int32 m_nAxisType;//css::chart::ChartAxisType
    bool m_bAxisTypeImported;
    bool m_bDateScaleImported;
    bool const m_bAddMissingXAxisForNetCharts; //to correct errors from older versions
    bool const m_bAdaptWrongPercentScaleValues; //to correct errors from older versions
    bool const m_bAdaptXAxisOrientationForOld2DBarCharts; //to correct different behaviour from older versions
    bool& m_rbAxisPositionAttributeImported;

    css::uno::Reference< css::drawing::XShape > getTitleShape();
    void CreateGrid( const OUString& sAutoStyleName, bool bIsMajor );
    void CreateAxis();
    void SetAxisTitle();
};

#endif // INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLAXISCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
