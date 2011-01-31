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
#ifndef _SCH_XMLAXISCONTEXT_HXX_
#define _SCH_XMLAXISCONTEXT_HXX_

#include "SchXMLImport.hxx"
#include "transporttypes.hxx"

// ----------------------------------------

class SchXMLAxisContext : public SvXMLImportContext
{
public:
    SchXMLAxisContext( SchXMLImportHelper& rImpHelper,
                       SvXMLImport& rImport, const rtl::OUString& rLocalName,
                       ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram > xDiagram,
                       std::vector< SchXMLAxis >& aAxes,
                       ::rtl::OUString& rCategoriesAddress,
                       bool bAddMissingXAxisForNetCharts,
                       bool bAdaptWrongPercentScaleValues,
                       bool bAdaptXAxisOrientationForOld2DBarCharts,
                       bool& rbAxisPositionAttributeImported );
    virtual ~SchXMLAxisContext();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
    virtual SvXMLImportContext* CreateChildContext(
        USHORT nPrefix,
        const rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    static void CorrectAxisPositions( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& xNewDoc,
                          const ::rtl::OUString& rChartTypeServiceName,
                          const ::rtl::OUString& rODFVersionOfFile,
                          bool bAxisPositionAttributeImported );

private:
    SchXMLImportHelper& m_rImportHelper;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart::XDiagram > m_xDiagram;
    SchXMLAxis m_aCurrentAxis;
    std::vector< SchXMLAxis >& m_rAxes;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > m_xAxisProps;
    rtl::OUString m_aAutoStyleName;
    rtl::OUString& m_rCategoriesAddress;
    sal_Int32 m_nAxisType;//::com::sun::star::chart::ChartAxisType
    bool m_bAxisTypeImported;
    bool m_bDateScaleImported;
    bool m_bAddMissingXAxisForNetCharts; //to correct errors from older versions
    bool m_bAdaptWrongPercentScaleValues; //to correct errors from older versions
    bool m_bAdaptXAxisOrientationForOld2DBarCharts; //to correct different behaviour from older versions
    bool& m_rbAxisPositionAttributeImported;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > getTitleShape();
    void CreateGrid( ::rtl::OUString sAutoStyleName, bool bIsMajor );
    void CreateAxis();
    void SetAxisTitle();
};

#endif  // _SCH_XMLAXISCONTEXT_HXX_
