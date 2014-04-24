/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_SOURCE_DRAW_IMPORTFILTERBASE_HXX
#define INCLUDED_WRITERPERFECT_SOURCE_DRAW_IMPORTFILTERBASE_HXX

#include <libwpd-stream/libwpd-stream.h>

#include <libwpg/libwpg.h>

#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase4.hxx>

namespace com { namespace sun { namespace star {
namespace beans
{
    class PropertyValue;
}
namespace lang
{
    class XComponent;
}
namespace uno
{
    class XComponentContext;
}
namespace xml { namespace sax {
    class XDocumentHandler;
}
}
} } }

namespace writerperfect
{
namespace draw
{

/* This component will be instantiated for both import or export. Whether it calls
 * setSourceDocument or setTargetDocument determines which Impl function the filter
 * member calls */
class ImportFilterImpl : public cppu::WeakImplHelper4
    <
    com::sun::star::document::XFilter,
    com::sun::star::document::XImporter,
    com::sun::star::document::XExtendedFilterDetection,
    com::sun::star::lang::XInitialization
    >
{
public:
    ImportFilterImpl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rxContext );
    virtual ~ImportFilterImpl();

    // XFilter
    virtual sal_Bool SAL_CALL filter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
    throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL cancel(  )
    throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XImporter
    virtual void SAL_CALL setTargetDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
    throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //XExtendedFilterDetection
    virtual OUString SAL_CALL detect( com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& Descriptor )
    throw( com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
    throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    virtual bool doDetectFormat( WPXInputStream &rInput, OUString &rTypeName ) = 0;
    virtual bool doImportDocument( WPXInputStream &rInput, libwpg::WPGPaintInterface &rGenerator ) = 0;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > mxDoc;
    OUString msFilterName;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > mxHandler;
};

/** A base class for import filters.
 */
typedef cppu::ImplInheritanceHelper1<ImportFilterImpl, com::sun::star::lang::XServiceInfo> ImportFilterBase;

}
}

#endif // INCLUDED_WRITERPERFECT_SOURCE_DRAW_IMPORTFILTERBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
