/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLFilter.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:15:20 $
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
#ifndef CHART2_XMLFILTER_HXX
#define CHART2_XMLFILTER_HXX

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

// for APPHELPER_... macros
#include "ServiceMacros.hxx"

namespace com { namespace sun { namespace star {
namespace embed
{
    class XStorage;
}
namespace xml { namespace sax
{
    class XParser;
}}
namespace document
{
    class XGraphicObjectResolver;
}
}}}

namespace chart
{

class XMLFilter : public
    ::cppu::WeakImplHelper4<
        ::com::sun::star::document::XFilter,
        ::com::sun::star::document::XExporter,
        ::com::sun::star::document::XImporter,
        ::com::sun::star::lang::XServiceInfo >
{
public:
    explicit XMLFilter( ::com::sun::star::uno::Reference<
                            ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~XMLFilter();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( XMLFilter )
    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

protected:
    // ____ XFilter ____
    virtual sal_Bool SAL_CALL filter(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue >& aDescriptor )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancel()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XImporter ____
    virtual void SAL_CALL setTargetDocument(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent >& Document )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XExporter ____
    virtual void SAL_CALL setSourceDocument(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent >& Document )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);

    inline ::rtl::OUString getDocumentHandler() const { return m_sDocumentHandler; }
    inline void setDocumentHandler(const ::rtl::OUString& _sDocumentHandler) { m_sDocumentHandler = _sDocumentHandler; }

    virtual ::rtl::OUString getMediaType(bool _bOasis);

    /** fills the oasis flag only when a filtername was set
    *
    * \param _rMediaDescriptor
    * \param _rOutOASIS
    */
    virtual void isOasisFormat(const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue >& _rMediaDescriptor, bool & _rOutOASIS );

private:
    // methods

    /// @return a warning code, or 0 for successful operation
    sal_Int32 impl_Import( const ::com::sun::star::uno::Reference<
                               ::com::sun::star::lang::XComponent > & xDocumentComp,
                           const ::com::sun::star::uno::Sequence<
                               ::com::sun::star::beans::PropertyValue > & aMediaDescriptor );
    /// @return a warning code, or 0 for successful operation
    sal_Int32 impl_ImportStream(
        const ::rtl::OUString & rStreamName,
        const ::rtl::OUString & rServiceName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::embed::XStorage > & xStorage,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XParser > & xParser,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiComponentFactory > & xFactory,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XGraphicObjectResolver > & xGraphicObjectResolver,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >& xPropSet );

    /// @return a warning code, or 0 for successful operation
    sal_Int32 impl_Export( const ::com::sun::star::uno::Reference<
                               ::com::sun::star::lang::XComponent > & xDocumentComp,
                           const ::com::sun::star::uno::Sequence<
                               ::com::sun::star::beans::PropertyValue > & aMediaDescriptor );
    /// @return a warning code, or 0 for successful operation
    sal_Int32 impl_ExportStream(
        const ::rtl::OUString & rStreamName,
        const ::rtl::OUString & rServiceName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::embed::XStorage > & xStorage,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::io::XActiveDataSource >& xActiveDataSource,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & xFactory,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Any > & rFilterProperties );

    // members
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext > m_xContext;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XComponent >       m_xTargetDoc;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XComponent >       m_xSourceDoc;
    ::rtl::OUString                                m_sDocumentHandler; // when set it will be set as doc handler

    volatile bool                                  m_bCancelOperation;
    ::osl::Mutex                                   m_aMutex;
};

// =============================================================================
class XMLReportFilterHelper : public XMLFilter
{
    virtual void isOasisFormat(const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::beans::PropertyValue >& _rMediaDescriptor, bool & _rOutOASIS );
public:
    explicit XMLReportFilterHelper( ::com::sun::star::uno::Reference<
                            ::com::sun::star::uno::XComponentContext > const & _xContext )
                            :XMLFilter(_xContext)
    {}
    /// establish methods for factory instatiation
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL   create(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext) throw(::com::sun::star::uno::Exception)
    {
        return (::cppu::OWeakObject *)new XMLReportFilterHelper( xContext );
    }
    static ::rtl::OUString getImplementationName_Static()
    {
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.chart2.report.XMLFilter" ));
    }
protected:
    virtual ::rtl::OUString SAL_CALL
        getImplementationName()
            throw( ::com::sun::star::uno::RuntimeException )
    {
        return getImplementationName_Static();
    }
    // ____ XImporter ____
    virtual void SAL_CALL setTargetDocument(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent >& Document )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException)
    {
        setDocumentHandler(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.ImportDocumentHandler")));
        XMLFilter::setTargetDocument(Document);
    }

    // ____ XExporter ____
    virtual void SAL_CALL setSourceDocument(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent >& Document )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException)
    {
        setDocumentHandler(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.ExportDocumentHandler")));
        XMLFilter::setSourceDocument(Document);
    }

    virtual ::rtl::OUString getMediaType(bool _bOasis);
};

} //  namespace chart

// CHART2_XMLFILTER_HXX
#endif
