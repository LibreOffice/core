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
#ifndef RPT_XMLFILTER_HXX
#define RPT_XMLFILTER_HXX

#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase5.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <osl/diagnose.h>
#include <unotools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <xmloff/xmlimp.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <xmloff/prhdlfac.hxx>
#include <xmloff/xmlprmap.hxx>

namespace rptui
{
    class OReportModel;
}
namespace rptxml
{
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::report;
using namespace ::com::sun::star::xml::sax;

// -------------
// - ORptFilter -
// -------------
class ORptFilter : public SvXMLImport
{
public:
    DECLARE_STL_USTRINGACCESS_MAP(Sequence<PropertyValue>,TPropertyNameMap);
    DECLARE_STL_USTRINGACCESS_MAP( Reference<XFunction> ,TGroupFunctionMap);
private:

    TGroupFunctionMap                               m_aFunctions;
    com::sun::star::uno::Any                        m_aViewSettings;
    Reference< XComponent >                         m_xSrcDoc;

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pDocElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pReportElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pGroupsElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pGroupElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pSectionElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pComponentElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pControlElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pFunctionElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pSubDocumentElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pFormatElemTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pColumnTokenMap;
    mutable ::std::auto_ptr<SvXMLTokenMap>          m_pCellElemTokenMap;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    UniReference < XMLPropertyHandlerFactory >      m_xPropHdlFactory;
    UniReference < XMLPropertySetMapper >           m_xCellStylesPropertySetMapper;
    UniReference < XMLPropertySetMapper >           m_xColumnStylesPropertySetMapper;
    UniReference < XMLPropertySetMapper >           m_xRowStylesPropertySetMapper;
    UniReference < XMLPropertySetMapper >           m_xTableStylesPropertySetMapper;

    Reference<XReportDefinition>                    m_xReportDefinition;
    ::boost::shared_ptr<rptui::OReportModel>        m_pReportModel;

    sal_Bool                            implImport( const Sequence< PropertyValue >& rDescriptor ) throw (RuntimeException);

    SvXMLImportContext* CreateStylesContext(const ::rtl::OUString& rLocalName,
                                     const Reference< XAttributeList>& xAttrList, sal_Bool bIsAutoStyle );
    SvXMLImportContext* CreateMetaContext(const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    SvXMLImportContext* CreateFontDeclsContext(const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
protected:
    // SvXMLImport
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                                      const ::rtl::OUString& rLocalName,
                                      const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual XMLShapeImportHelper* CreateShapeImport();

    virtual ~ORptFilter()  throw();
public:

    ORptFilter( const Reference< XMultiServiceFactory >& _rxMSF,sal_uInt16 nImportFlags = IMPORT_ALL );

    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& rDescriptor ) throw(RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);

    inline Reference< XMultiServiceFactory > getORB() { return SvXMLImport::getServiceFactory(); }
    inline Reference<XReportDefinition> getReportDefinition() const { return m_xReportDefinition; }
    /** return the SdrModel of the real model
    *
    * \return
    */
    ::boost::shared_ptr<rptui::OReportModel> getSdrModel() const { return m_pReportModel; }
    void FinishStyles();

    virtual void SAL_CALL startDocument(void)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL endDocument(void)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    const SvXMLTokenMap& GetDocElemTokenMap() const;
    const SvXMLTokenMap& GetReportElemTokenMap() const;
    const SvXMLTokenMap& GetGroupElemTokenMap() const;
    const SvXMLTokenMap& GetSectionElemTokenMap() const;
    const SvXMLTokenMap& GetComponentElemTokenMap() const;
    const SvXMLTokenMap& GetReportElementElemTokenMap() const;
    const SvXMLTokenMap& GetControlElemTokenMap() const;
    const SvXMLTokenMap& GetControlPropertyElemTokenMap() const;
    const SvXMLTokenMap& GetFunctionElemTokenMap() const;
    const SvXMLTokenMap& GetFormatElemTokenMap() const;
    const SvXMLTokenMap& GetSubDocumentElemTokenMap() const;
    const SvXMLTokenMap& GetColumnTokenMap() const;
    const SvXMLTokenMap& GetCellElemTokenMap() const;

    inline UniReference < XMLPropertySetMapper > GetCellStylesPropertySetMapper()      const   { return m_xCellStylesPropertySetMapper;    }
    inline UniReference < XMLPropertySetMapper > GetColumnStylesPropertySetMapper()    const   { return m_xColumnStylesPropertySetMapper;  }
    inline UniReference < XMLPropertySetMapper > GetRowStylesPropertySetMapper()       const   { return m_xRowStylesPropertySetMapper;     }
    inline UniReference < XMLPropertySetMapper > GetTableStylesPropertySetMapper()     const   { return m_xTableStylesPropertySetMapper;   }
    static ::rtl::OUString convertFormula(const ::rtl::OUString& _sFormula);
    /** inserts a new function
    *
    * \param _xFunction
    */
    void insertFunction(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunction > & _xFunction);
    void removeFunction(const ::rtl::OUString& _sFunctionName);
    inline const TGroupFunctionMap& getFunctions() const { return m_aFunctions; }

    virtual SvXMLImport&                getGlobalContext();

    virtual void                        enterEventContext();
    virtual void                        leaveEventContext();

    sal_Bool                            isOldFormat() const;
};

/** Imports only settings
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptImportHelper
{
public:
    static ::rtl::OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
};

/** Imports only content
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptContentImportHelper
{
public:
    static ::rtl::OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
};

/** Imports only styles
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptStylesImportHelper
{
public:
    static ::rtl::OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
};

/** Imports only meta data
 * \ingroup reportdesign_source_filter_xml
 *
 */
class ORptMetaImportHelper
{
public:
    static ::rtl::OUString getImplementationName_Static(  ) throw (::com::sun::star::uno::RuntimeException);
    static Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
};

// -----------------------------------------------------------------------------
} // rptxml
// -----------------------------------------------------------------------------
#endif // RPT_XMLFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
