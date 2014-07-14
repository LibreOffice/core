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

#ifndef _SDPROPLS_HXX
#define _SDPROPLS_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <xmloff/xmlnume.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/prhdlfac.hxx>
#include <xmloff/xmlprmap.hxx>
#include "xmloff/XMLTextListAutoStylePool.hxx"
#include <xmloff/xmlexppr.hxx>
#include <xmlsdtypes.hxx>

//////////////////////////////////////////////////////////////////////////////
// entry list for graphic properties

extern const XMLPropertyMapEntry aXMLSDProperties[];

//////////////////////////////////////////////////////////////////////////////
// entry list for presentation page properties

extern const XMLPropertyMapEntry aXMLSDPresPageProps[];
extern const XMLPropertyMapEntry aXMLSDPresPageProps_onlyHeadersFooter[];

//////////////////////////////////////////////////////////////////////////////
// enum maps for attributes

extern SvXMLEnumMapEntry aXML_ConnectionKind_EnumMap[];
extern SvXMLEnumMapEntry aXML_CircleKind_EnumMap[];

//////////////////////////////////////////////////////////////////////////////

/** contains the attribute to property mapping for a drawing layer table */
extern const XMLPropertyMapEntry aXMLTableShapeAttributes[];

//////////////////////////////////////////////////////////////////////////////
// factory for own graphic properties

class SvXMLExport;
class SvXMLImport;

class XMLSdPropHdlFactory : public XMLPropertyHandlerFactory
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > mxModel;
    SvXMLExport* mpExport;
    SvXMLImport* mpImport;

public:
    XMLSdPropHdlFactory( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >, SvXMLExport& rExport );
    XMLSdPropHdlFactory( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >, SvXMLImport& rImport );
    virtual ~XMLSdPropHdlFactory();
    virtual const XMLPropertyHandler* GetPropertyHandler( sal_Int32 nType ) const;
};

class XMLShapePropertySetMapper : public XMLPropertySetMapper
{
public:
    XMLShapePropertySetMapper(const UniReference< XMLPropertyHandlerFactory >& rFactoryRef);
    XMLShapePropertySetMapper(const UniReference< XMLPropertyHandlerFactory >& rFactoryRef, sal_uInt16 nOffset);
    ~XMLShapePropertySetMapper();
};

class XMLShapeExportPropertyMapper : public SvXMLExportPropertyMapper
{
private:
    XMLTextListAutoStylePool *mpListAutoPool;
    SvXMLExport& mrExport;
    SvxXMLNumRuleExport maNumRuleExp;
    sal_Bool mbIsInAutoStyles;

    const rtl::OUString msCDATA;
    const rtl::OUString msTrue;
    const rtl::OUString msFalse;

protected:
    virtual void ContextFilter(
        ::std::vector< XMLPropertyState >& rProperties,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > rPropSet ) const;
public:
    XMLShapeExportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper, XMLTextListAutoStylePool *pListAutoPool, SvXMLExport& rExport );
    virtual ~XMLShapeExportPropertyMapper();

    virtual void        handleElementItem(
                            SvXMLExport& rExport,
                            const XMLPropertyState& rProperty,
                            sal_uInt16 nFlags,
                            const ::std::vector< XMLPropertyState >* pProperties = 0,
                            sal_uInt32 nIdx = 0
                            ) const;

    void SetAutoStyles( sal_Bool bIsInAutoStyles ) { mbIsInAutoStyles = bIsInAutoStyles; }

    virtual void handleSpecialItem(
            SvXMLAttributeList& rAttrList,
            const XMLPropertyState& rProperty,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap,
            const ::std::vector< XMLPropertyState > *pProperties = 0,
            sal_uInt32 nIdx = 0 ) const;
};

class XMLPagePropertySetMapper : public XMLPropertySetMapper
{
public:
    XMLPagePropertySetMapper(const UniReference< XMLPropertyHandlerFactory >& rFactoryRef);
    ~XMLPagePropertySetMapper();
};

class XMLPageExportPropertyMapper : public SvXMLExportPropertyMapper
{
private:
    SvXMLExport& mrExport;

    const rtl::OUString msCDATA;
    const rtl::OUString msTrue;
    const rtl::OUString msFalse;

protected:
    virtual void ContextFilter(
        ::std::vector< XMLPropertyState >& rProperties,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > rPropSet ) const;
public:
    XMLPageExportPropertyMapper( const UniReference< XMLPropertySetMapper >& rMapper, SvXMLExport& rExport );
    virtual ~XMLPageExportPropertyMapper();

    virtual void        handleElementItem(
                            SvXMLExport& rExport,
                            const XMLPropertyState& rProperty,
                            sal_uInt16 nFlags,
                            const ::std::vector< XMLPropertyState >* pProperties = 0,
                            sal_uInt32 nIdx = 0
                            ) const;
};

#endif  //  _SDPROPLS_HXX
