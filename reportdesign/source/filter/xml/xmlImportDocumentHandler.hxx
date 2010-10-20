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
#ifndef RPT_IMPORTDOCUMENTHANDLER_HXX_INCLUDED
#define RPT_IMPORTDOCUMENTHANDLER_HXX_INCLUDED

#include "sal/config.h"
#include "com/sun/star/uno/XComponentContext.hpp"
#include <cppuhelper/implbase3.hxx>
#include "com/sun/star/xml/sax/XDocumentHandler.hpp"
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include "com/sun/star/chart2/data/XDatabaseDataProvider.hpp"
#include <comphelper/uno3.hxx>
#include <memory>

class SvXMLTokenMap;
namespace rptxml
{
typedef ::cppu::WeakAggImplHelper3< ::com::sun::star::xml::sax::XDocumentHandler
                                ,   ::com::sun::star::lang::XInitialization
                                ,   ::com::sun::star::lang::XServiceInfo>   ImportDocumentHandler_BASE;

class ImportDocumentHandler : public ImportDocumentHandler_BASE
{
public:
    // XServiceInfo - static versions
    static ::rtl::OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                    create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);
public:
    explicit ImportDocumentHandler(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context);

private:
    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    DECLARE_XINTERFACE( )
    DECLARE_XTYPEPROVIDER( )

    // ::com::sun::star::xml::sax::XDocumentHandler:
    virtual void SAL_CALL startDocument() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL endDocument() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL startElement(const ::rtl::OUString & aName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttribs) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL endElement(const ::rtl::OUString & aName) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL characters(const ::rtl::OUString & aChars) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL ignorableWhitespace(const ::rtl::OUString & aWhitespaces) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL processingInstruction(const ::rtl::OUString & aTarget, const ::rtl::OUString & aData) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::xml::sax::SAXException);
    virtual void SAL_CALL setDocumentLocator(const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > & xLocator) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::xml::sax::SAXException);

    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

private:
    ImportDocumentHandler(ImportDocumentHandler &); // not defined
    void operator =(ImportDocumentHandler &); // not defined

    virtual ~ImportDocumentHandler();

    ::osl::Mutex                                                                        m_aMutex;
    bool                                                                                m_bImportedChart;
    ::std::vector< ::rtl::OUString>                                                     m_aMasterFields;
    ::std::vector< ::rtl::OUString>                                                     m_aDetailFields;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >           m_aArguments;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >    m_xDelegatee;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >             m_xProxy;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider >           m_xTypeProvider;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo >            m_xServiceInfo;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >        m_xModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDatabaseDataProvider >   m_xDatabaseDataProvider;

    ::std::auto_ptr<SvXMLTokenMap>                                                      m_pReportElemTokenMap;
};
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
#endif // RPT_IMPORTDOCUMENTHANDLER_HXX_INCLUDED
