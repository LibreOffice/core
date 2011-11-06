/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
        sal_uInt16 nPrefix,
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
