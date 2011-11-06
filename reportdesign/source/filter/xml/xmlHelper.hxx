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


#ifndef RPT_XMLHELPER_HXX
#define RPT_XMLHELPER_HXX

#include <xmloff/xmlprmap.hxx>
#include <xmloff/contextid.hxx>
#include <xmloff/controlpropertyhdl.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <memory>

#define CTF_RPT_NUMBERFORMAT                    (XML_DB_CTF_START + 1)

#define XML_STYLE_FAMILY_REPORT_ID              700
#define XML_STYLE_FAMILY_REPORT_NAME            "report-element"
#define XML_STYLE_FAMILY_REPORT_PREFIX          "rptelem"


class SvXMLImport;
class SvXMLExport;
class SvXMLStylesContext;
class SvXMLTokenMap;
namespace rptxml
{
    class OPropertyHandlerFactory : public ::xmloff::OControlPropertyHandlerFactory
    {
        OPropertyHandlerFactory(const OPropertyHandlerFactory&);
        void operator =(const OPropertyHandlerFactory&);
    protected:
        mutable ::std::auto_ptr<XMLConstantsPropertyHandler>    m_pDisplayHandler;
        mutable ::std::auto_ptr<XMLPropertyHandler>             m_pTextAlignHandler;
    public:
        OPropertyHandlerFactory();
        virtual ~OPropertyHandlerFactory();

        virtual const XMLPropertyHandler* GetPropertyHandler(sal_Int32 _nType) const;
    };

    class OXMLHelper
    {
    public:
        static UniReference < XMLPropertySetMapper > GetCellStylePropertyMap(bool _bOldFormat = false);

        static const SvXMLEnumMapEntry* GetReportPrintOptions();
        static const SvXMLEnumMapEntry* GetForceNewPageOptions();
        static const SvXMLEnumMapEntry* GetKeepTogetherOptions();
        static const SvXMLEnumMapEntry* GetCommandTypeOptions();
        static const SvXMLEnumMapEntry* GetImageScaleOptions();

        static const XMLPropertyMapEntry* GetTableStyleProps();
        static const XMLPropertyMapEntry* GetColumnStyleProps();

        static const XMLPropertyMapEntry* GetRowStyleProps();

        static void copyStyleElements(const bool _bOld,const ::rtl::OUString& _sStyleName,const SvXMLStylesContext* _pAutoStyles,const com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet>& _xProp);
        static com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet> createBorderPropertySet();

        static SvXMLTokenMap* GetReportElemTokenMap();
        static SvXMLTokenMap* GetSubDocumentElemTokenMap();

    };
// -----------------------------------------------------------------------------
} // rptxml
// -----------------------------------------------------------------------------
#endif // RPT_XMLHELPER_HXX

