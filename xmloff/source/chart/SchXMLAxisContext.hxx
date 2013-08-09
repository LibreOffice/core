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
#ifndef _SCH_XMLAXISCONTEXT_HXX_
#define _SCH_XMLAXISCONTEXT_HXX_

#include "SchXMLImport.hxx"
#include "transporttypes.hxx"

class SchXMLAxisContext : public SvXMLImportContext
{
public:
    SchXMLAxisContext( SchXMLImportHelper& rImpHelper,
                       SvXMLImport& rImport, const OUString& rLocalName,
                       ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram > xDiagram,
                       std::vector< SchXMLAxis >& aAxes,
                       OUString& rCategoriesAddress,
                       bool bAddMissingXAxisForNetCharts,
                       bool bAdaptWrongPercentScaleValues,
                       bool bAdaptXAxisOrientationForOld2DBarCharts,
                       bool& rbAxisPositionAttributeImported );
    virtual ~SchXMLAxisContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    static void CorrectAxisPositions( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xNewDoc,
                          const OUString& rChartTypeServiceName,
                          const OUString& rODFVersionOfFile,
                          bool bAxisPositionAttributeImported );

private:
    SchXMLImportHelper& m_rImportHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram > m_xDiagram;
    SchXMLAxis m_aCurrentAxis;
    std::vector< SchXMLAxis >& m_rAxes;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > m_xAxisProps;
    OUString m_aAutoStyleName;
    OUString& m_rCategoriesAddress;
    sal_Int32 m_nAxisType;//::com::sun::star::chart::ChartAxisType
    bool m_bAxisTypeImported;
    bool m_bDateScaleImported;
    bool m_bAddMissingXAxisForNetCharts; //to correct errors from older versions
    bool m_bAdaptWrongPercentScaleValues; //to correct errors from older versions
    bool m_bAdaptXAxisOrientationForOld2DBarCharts; //to correct different behaviour from older versions
    bool& m_rbAxisPositionAttributeImported;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > getTitleShape();
    void CreateGrid( OUString sAutoStyleName, bool bIsMajor );
    void CreateAxis();
    void SetAxisTitle();
};

#endif  // _SCH_XMLAXISCONTEXT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
