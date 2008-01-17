/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filterbase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:44 $
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

#ifndef OOX_CORE_FILTERBASE_HXX
#define OOX_CORE_FILTERBASE_HXX

#include <memory>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <cppuhelper/implbase5.hxx>
#include "oox/helper/helper.hxx"
#include "oox/helper/storagebase.hxx"

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace frame { class XModel; }
    namespace task { class XStatusIndicator; }
    namespace task { class XInteractionHandler; }
    namespace io { class XInputStream; }
    namespace io { class XOutputStream; }
} } }

namespace oox {
namespace core {

// ============================================================================

struct FilterBaseImpl;

class FilterBase : public ::cppu::WeakImplHelper5<
                            ::com::sun::star::lang::XServiceInfo,
                            ::com::sun::star::lang::XInitialization,
                            ::com::sun::star::document::XImporter,
                            ::com::sun::star::document::XExporter,
                            ::com::sun::star::document::XFilter >
{
public:
    explicit            FilterBase(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory );

    virtual             ~FilterBase();

    /** Returns true, if filter is an import filter. */
    bool                isImportFilter() const;
    /** Returns true, if filter is an export filter. */
    bool                isExportFilter() const;

    /** Derived classes implement import of the entire document. */
    virtual bool        importDocument() = 0;

    /** Derived classes implement export of the entire document. */
    virtual bool        exportDocument() = 0;

    // ------------------------------------------------------------------------

    /** Returns the service factory passed in the constructor. */
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&
                        getServiceFactory() const;

    /** Returns the document model. */
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >&
                        getModel() const;

    /** Returns the status indicator. */
    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >&
                        getStatusIndicator() const;

    /** Returns the status interaction handler. */
    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >&
                        getInteractionHandler() const;

    /** Returns the URL of the imported or exported file. */
    const ::rtl::OUString& getFileUrl() const;

    /** Returns an absolute URL for the passed relative or absolute URL. */
    ::rtl::OUString     getAbsoluteUrl( const ::rtl::OUString& rUrl ) const;

    /** Returns the base storage of the imported/exported file. */
    StorageRef          getStorage() const;

    /** Opens and returns the specified sub storage from the base storage.

        @param rStorageName
            The name of the embedded storage. The name may contain slashes to
            open storages from embedded substorages.
        @param bCreate
            True = create missing sub storages (for export filters).
     */
    StorageRef          openSubStorage(
                            const ::rtl::OUString& rStorageName,
                            bool bCreate );

    /** Opens and returns the specified input stream from the base storage.

        @param rStreamName
            The name of the embedded storage stream. The name may contain
            slashes to open streams from embedded substorages. If base stream
            access has been enabled in the storage, the base stream can be
            accessed by passing an empty string as stream name.
     */
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        openInputStream( const ::rtl::OUString& rStreamName );

    /** Opens and returns the specified output stream from the base storage.

        @param rStreamName
            The name of the embedded storage stream. The name may contain
            slashes to open streams from embedded substorages. If base stream
            access has been enabled in the storage, the base stream can be
            accessed by passing an empty string as stream name.
     */
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                        openOutputStream( const ::rtl::OUString& rStreamName );

    // com.sun.star.lang.XServiceInfo interface -------------------------------

    virtual ::rtl::OUString SAL_CALL
                        getImplementationName()
                            throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL
                        supportsService( const ::rtl::OUString& rServiceName )
                            throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
                        getSupportedServiceNames()
                            throw( ::com::sun::star::uno::RuntimeException );

    // com.sun.star.lang.XInitialization interface ----------------------------

    virtual void SAL_CALL initialize(
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rArgs )
                            throw(  ::com::sun::star::uno::Exception,
                                    ::com::sun::star::uno::RuntimeException );

    // com.sun.star.document.XImporter interface ------------------------------

    virtual void SAL_CALL setTargetDocument(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& rxDocument )
                            throw(  ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException );

    // com.sun.star.document.XExporter interface ------------------------------

    virtual void SAL_CALL setSourceDocument(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& rxDocument )
                            throw(  ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException );

    // com.sun.star.document.XFilter interface --------------------------------

    virtual sal_Bool SAL_CALL filter(
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rDescriptor )
                            throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL cancel()
                            throw( ::com::sun::star::uno::RuntimeException );

    // ------------------------------------------------------------------------
private:
    virtual ::rtl::OUString implGetImplementationName() const = 0;

    virtual StorageRef  implCreateStorage(
                            ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream,
                            ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rxOutStream ) const = 0;

private:
    ::std::auto_ptr< FilterBaseImpl > mxImpl;
};

// ============================================================================

} // namespace core
} // namespace oox

#endif

