/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: filterbase.hxx,v $
 * $Revision: 1.4.20.1 $
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
#include <com/sun/star/io/XStream.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/implbase5.hxx>
#include "oox/helper/binarystreambase.hxx"
#include "oox/helper/storagebase.hxx"
#include "oox/dllapi.h"

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace awt { struct DeviceInfo; }
    namespace frame { class XModel; }
    namespace task { class XStatusIndicator; }
    namespace task { class XInteractionHandler; }
    namespace io { class XInputStream; }
    namespace io { class XOutputStream; }
    namespace io { class XStream; }
    namespace graphic { class XGraphic; }
} } }

namespace comphelper {
    class IDocPasswordVerifier;
}

namespace oox {
    class GraphicHelper;
    class MediaDescriptor;
    class ModelObjectHelper;
}

namespace oox { namespace ole {
    class OleObjectHelper;
} }

namespace oox {
namespace core {

// ============================================================================

struct FilterBaseImpl;

typedef ::cppu::WeakImplHelper5<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::document::XImporter,
        ::com::sun::star::document::XExporter,
        ::com::sun::star::document::XFilter >
    FilterBaseBase;

class OOX_DLLPUBLIC FilterBase : public FilterBaseBase, public ::cppu::BaseMutex
{
public:
    explicit            FilterBase(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxGlobalFactory );

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

    /** Returns the specified argument passed through the XInitialization interface. */
    ::com::sun::star::uno::Any getArgument( const ::rtl::OUString& rArgName ) const;

    /** Returns the global service factory passed in the filter constructor (always existing). */
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&
                        getGlobalFactory() const;

    /** Returns the document model (always existing). */
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >&
                        getModel() const;

    /** Returns the service factory provided by the document model (always existing). */
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&
                        getModelFactory() const;

    /** Returns the status indicator (may be null). */
    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >&
                        getStatusIndicator() const;

    /** Returns the status interaction handler (may be null). */
    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >&
                        getInteractionHandler() const;

    /** Returns the media descriptor. */
    MediaDescriptor&    getMediaDescriptor() const;

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
                            bool bCreate ) const;

    /** Opens and returns the specified input stream from the base storage.

        @param rStreamName
            The name of the embedded storage stream. The name may contain
            slashes to open streams from embedded substorages. If base stream
            access has been enabled in the storage, the base stream can be
            accessed by passing an empty string as stream name.
     */
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        openInputStream( const ::rtl::OUString& rStreamName ) const;

    /** Opens and returns the specified output stream from the base storage.

        @param rStreamName
            The name of the embedded storage stream. The name may contain
            slashes to open streams from embedded substorages. If base stream
            access has been enabled in the storage, the base stream can be
            accessed by passing an empty string as stream name.
     */
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                        openOutputStream( const ::rtl::OUString& rStreamName ) const;

    /** Commits changes to base storage (and substorages) */
    void                commitStorage() const;

    // helpers ----------------------------------------------------------------

    /** Returns a helper for the handling of graphics and graphic objects. */
    GraphicHelper&      getGraphicHelper() const;

    /** Returns a helper with containers for various named drawing objects for
        the imported document. */
    ModelObjectHelper&  getModelObjectHelper() const;

    /** Returns a helper for the handling of OLE obejcts. */
    ::oox::ole::OleObjectHelper& getOleObjectHelper() const;

    /** Returns information about the output device. */
    const ::com::sun::star::awt::DeviceInfo& getDeviceInfo() const;
    /** Converts the passed value from horizontal screen pixels to 1/100 mm. */
    sal_Int32           convertScreenPixelX( double fPixelX ) const;
    /** Converts the passed value from vertical screen pixels to 1/100 mm. */
    sal_Int32           convertScreenPixelY( double fPixelY ) const;

    /** Returns a system color specified by the passed XML token identifier. */
    sal_Int32           getSystemColor( sal_Int32 nToken, sal_Int32 nDefaultRgb = API_RGB_TRANSPARENT ) const;
    /** Derived classes may implement to resolve a scheme color from the passed XML token identifier. */
    virtual sal_Int32   getSchemeColor( sal_Int32 nToken ) const;
    /** Derived classes may implement to resolve a palette index to an RGB color. */
    virtual sal_Int32   getPaletteColor( sal_Int32 nPaletteIdx ) const;

    /** Requests a password from the media descriptor or from the user. On
        success, the password will be inserted into the media descriptor. */
    ::rtl::OUString     requestPassword( ::comphelper::IDocPasswordVerifier& rVerifier ) const;

    /** Imports the raw binary data from the specified stream.
        @return  True, if the data could be imported from the stream. */
    bool                importBinaryData( StreamDataSequence& orDataSeq, const ::rtl::OUString& rStreamName );

    /** Imports a graphic from the storage stream with the passed path and name. */
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >
                        importEmbeddedGraphic( const ::rtl::OUString& rStreamName ) const;
    /** Imports a graphic object from the storage stream with the passed path and name. */
    ::rtl::OUString     importEmbeddedGraphicObject( const ::rtl::OUString& rStreamName ) const;

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

    /** Receives user defined arguments.

        @param rArgs
            the sequence of arguments passed to the filter. The implementation
            expects one or two arguments. The first argument shall be the
            com.sun.star.lang.XMultiServiceFactory interface of the global
            service factory. The optional second argument may contain a
            sequence of com.sun.star.beans.NamedValue objects. The different
            filter implemetations may support different arguments.
     */
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
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rMediaDescSeq )
                            throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL cancel()
                            throw( ::com::sun::star::uno::RuntimeException );

    // ------------------------------------------------------------------------
protected:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        implGetInputStream( MediaDescriptor& rMediaDesc ) const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >
                        implGetOutputStream( MediaDescriptor& rMediaDesc ) const;

private:
    void                setMediaDescriptor(
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rMediaDescSeq );

    virtual ::rtl::OUString implGetImplementationName() const = 0;

    virtual StorageRef  implCreateStorage(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream ) const = 0;
    virtual StorageRef  implCreateStorage(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rxOutStream ) const = 0;

private:
    ::std::auto_ptr< FilterBaseImpl > mxImpl;
};

// ============================================================================

} // namespace core
} // namespace oox

#endif

