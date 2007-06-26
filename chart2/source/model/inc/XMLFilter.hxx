/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLFilter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-26 10:06:17 $
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

    volatile bool                                  m_bCancelOperation;
    ::osl::Mutex                                   m_aMutex;
};

} //  namespace chart

// CHART2_XMLFILTER_HXX
#endif
