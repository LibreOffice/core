/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlExportDocumentHandler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:03:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef RPT_EXPORTDOCUMENTHANDLER_HXX_INCLUDED
#define RPT_EXPORTDOCUMENTHANDLER_HXX_INCLUDED

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

namespace rptxml
{
typedef ::cppu::WeakAggImplHelper3< ::com::sun::star::xml::sax::XDocumentHandler
                                ,   ::com::sun::star::lang::XInitialization
                                ,   ::com::sun::star::lang::XServiceInfo>   ExportDocumentHandler_BASE;

class ExportDocumentHandler : public ExportDocumentHandler_BASE
{
public:
    // XServiceInfo - static versions
    static ::rtl::OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                    create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);
public:
    explicit ExportDocumentHandler(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context);

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
    ExportDocumentHandler(ExportDocumentHandler &); // not defined
    void operator =(ExportDocumentHandler &); // not defined

    virtual ~ExportDocumentHandler();

    ::osl::Mutex                                                                        m_aMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >        m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >    m_xDelegatee;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >             m_xProxy;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider >           m_xTypeProvider;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo >            m_xServiceInfo;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >        m_xModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDatabaseDataProvider >   m_xDatabaseDataProvider;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >                                  m_aColumns;
    sal_Int32                                                                           m_nCurrentCellIndex;
    bool m_bTableRowsStarted;
    bool m_bFirstRowExported;
};
// -----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
#endif // RPT_EXPORTDOCUMENTHANDLER_HXX_INCLUDED
