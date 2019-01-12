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
#ifndef INCLUDED_XMLOFF_SCHXMLIMPORTHELPER_HXX
#define INCLUDED_XMLOFF_SCHXMLIMPORTHELPER_HXX

#include <memory>

#include <salhelper/simplereferenceobject.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmltkmap.hxx>

namespace com { namespace sun { namespace star { namespace chart { class XChartDocument; } } } }
namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }

namespace com { namespace sun { namespace star {
    namespace frame {
        class XModel;
    }
    namespace xml {
        namespace sax {
            class XAttributeList;
        }
    }
    namespace chart2 {
        class XChartDocument;
        class XDataSeries;
    }
}}}

class SvXMLStylesContext;
class SvXMLImportContext;
class SvXMLImport;


/** With this class you can import a <chart:chart> element containing
    its data as <table:table> element or without internal table. In
    the latter case you have to provide a table address mapper that
    converts table addresses in XML format to the appropriate application
    format.
 */
class SchXMLImportHelper : public salhelper::SimpleReferenceObject
{
private:
    css::uno::Reference< css::chart::XChartDocument > mxChartDoc;
    SvXMLStylesContext* mpAutoStyles;

    std::unique_ptr<SvXMLTokenMap> mpChartDocElemTokenMap;
    std::unique_ptr<SvXMLTokenMap> mpTableElemTokenMap;
    std::unique_ptr<SvXMLTokenMap> mpChartElemTokenMap;
    std::unique_ptr<SvXMLTokenMap> mpPlotAreaElemTokenMap;
    std::unique_ptr<SvXMLTokenMap> mpSeriesElemTokenMap;

    std::unique_ptr<SvXMLTokenMap> mpChartAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap> mpPlotAreaAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap> mpCellAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap> mpSeriesAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap> mpPropMappingAttrTokenMap;
    std::unique_ptr<SvXMLTokenMap> mpRegEquationAttrTokenMap;

public:

    SchXMLImportHelper();

    /** get the context for reading the <chart:chart> element with subelements.
        The result is stored in the XModel given if it also implements
        XChartDocument
     */
    SvXMLImportContext* CreateChartContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::frame::XModel >& rChartModel,
        const css::uno::Reference< css::xml::sax::XAttributeList >& rAttrList );

    /** set the auto-style context that will be used to retrieve auto-styles
        used inside the following <chart:chart> element to parse
     */
    void SetAutoStylesContext( SvXMLStylesContext* pAutoStyles ) { mpAutoStyles = pAutoStyles; }
    SvXMLStylesContext* GetAutoStylesContext() const { return mpAutoStyles; }

    /// Fill in the autostyle.
    void FillAutoStyle(const OUString& rAutoStyleName, const css::uno::Reference<css::beans::XPropertySet>& rProp);

    const css::uno::Reference< css::chart::XChartDocument >& GetChartDocument()
        { return mxChartDoc; }

    const SvXMLTokenMap& GetDocElemTokenMap();
    const SvXMLTokenMap& GetTableElemTokenMap();
    const SvXMLTokenMap& GetChartElemTokenMap();
    const SvXMLTokenMap& GetPlotAreaElemTokenMap();
    const SvXMLTokenMap& GetSeriesElemTokenMap();
    const SvXMLTokenMap& GetPropMappingAttrTokenMap();

    const SvXMLTokenMap& GetChartAttrTokenMap();
    const SvXMLTokenMap& GetPlotAreaAttrTokenMap();
    const SvXMLTokenMap& GetCellAttrTokenMap();
    const SvXMLTokenMap& GetSeriesAttrTokenMap();
    const SvXMLTokenMap& GetRegEquationAttrTokenMap();

    static sal_uInt16 GetChartFamilyID() { return XML_STYLE_FAMILY_SCH_CHART_ID; }

    /** @param bPushLastChartType If </sal_False>, in case a new chart type has to
               be added (because it does not exist yet), it is appended at the
               end of the chart-type container.  When </sal_True>, a new chart type
               is added at one position before the last one, i.e. the formerly
               last chart type is pushed back, so that it remains the last one.

               This is needed when the global chart type is set to type A, but
               the first series has type B. Then B should appear before A (done
               by passing true).  Once a series of type A has been read,
               following new chart types are again be added at the end (by
               passing false).
     */
    static css::uno::Reference< css::chart2::XDataSeries > GetNewDataSeries(
                    const css::uno::Reference< css::chart2::XChartDocument > & xDoc,
                    sal_Int32 nCoordinateSystemIndex,
                    const OUString & rChartTypeName,
                    bool bPushLastChartType );

    static void DeleteDataSeries(
                    const css::uno::Reference< css::chart2::XDataSeries >& xSeries,
                    const css::uno::Reference< css::chart2::XChartDocument > & xDoc );
};

#endif // INCLUDED_XMLOFF_SCHXMLIMPORTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
