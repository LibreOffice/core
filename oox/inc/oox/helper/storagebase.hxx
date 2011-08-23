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

#ifndef OOX_HELPER_STORAGEBASE_HXX
#define OOX_HELPER_STORAGEBASE_HXX

#include "oox/dllapi.h"
#include "oox/helper/containerhelper.hxx"

namespace com { namespace sun { namespace star {
    namespace embed { class XStorage; }
    namespace io { class XInputStream; }
    namespace io { class XOutputStream; }
    namespace io { class XStream; }
} } }

namespace oox {

// ============================================================================

class StorageBase;
typedef ::boost::shared_ptr< StorageBase > StorageRef;

/** Base class for storage access implementations.

    Derived classes will be used to encapsulate storage access implementations
    for ZIP storages containing XML streams, and OLE storages containing binary
    data streams.
 */
class OOX_DLLPUBLIC StorageBase
{
public:
    explicit            StorageBase(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream,
                            bool bBaseStreamAccess );

    explicit            StorageBase(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rxOutStream,
                            bool bBaseStreamAccess );

    virtual             ~StorageBase();

    /** Returns true, if the object represents a valid storage. */
    bool                isStorage() const;

    /** Returns true, if the object represents the root storage. */
    bool                isRootStorage() const;

    /** Returns true, if the storage operates in read-only mode (based on an
        input stream). */
    bool                isReadOnly() const;

    /** Returns the com.sun.star.embed.XStorage interface of the current storage. */
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
                        getXStorage() const;

    /** Returns the element name of this storage. */
    const ::rtl::OUString& getName() const;

    /** Returns the full path of this storage. */
    ::rtl::OUString     getPath() const;

    /** Fills the passed vector with the names of all direct elements of this
        storage. */
    void                getElementNames( ::std::vector< ::rtl::OUString >& orElementNames ) const;

    /** Opens and returns the specified sub storage from the storage.

        @param rStorageName
            The name of the embedded storage. The name may contain slashes to
            open storages from embedded substorages.
        @param bCreateMissing
            True = create missing sub storages (for export filters). Must be
            false for storages based on input streams.
     */
    StorageRef          openSubStorage( const ::rtl::OUString& rStorageName, bool bCreateMissing );

    /** Opens and returns the specified input stream from the storage.

        @param rStreamName
            The name of the embedded storage stream. The name may contain
            slashes to open streams from embedded substorages. If base stream
            access has been enabled in the constructor, the base stream can be
            accessed by passing an empty string as stream name.
     */
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        openInputStream( const ::rtl::OUString& rStreamName );

    /** Opens and returns the specified output stream from the storage.

        @param rStreamName
            The name of the embedded storage stream. The name may contain
            slashes to create and open streams in embedded substorages. If base
            stream access has been enabled in the constructor, the base stream
            can be accessed by passing an empty string as stream name.
     */
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                        openOutputStream( const ::rtl::OUString& rStreamName );

    /** Copies the specified element from this storage to the passed
         destination storage.

        @param rElementName
            The name of the embedded storage or stream. The name may contain
            slashes to specify an element in an embedded substorage. In this
            case, the element will be copied to the same substorage in the
            destination storage.
     */
    void                copyToStorage( StorageBase& rDestStrg, const ::rtl::OUString& rElementName );

    /** Copies all streams of this storage and of all substorages to the passed
        destination. */
    void                copyStorageToStorage( StorageBase& rDestStrg );

    /** Commits the changes to the storage and all substorages. */
    void                commit();

protected:
    /** Special constructor for sub storage objects. */
    explicit            StorageBase( const StorageBase& rParentStorage, const ::rtl::OUString& rStorageName, bool bReadOnly );

private:
                        StorageBase( const StorageBase& );
    StorageBase&        operator=( const StorageBase& );

    /** Returns true, if the object represents a valid storage. */
    virtual bool        implIsStorage() const = 0;

    /** Returns the com.sun.star.embed.XStorage interface of the current storage. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
                        implGetXStorage() const = 0;

    /** Returns the names of all elements of this storage. */
    virtual void        implGetElementNames( ::std::vector< ::rtl::OUString >& orElementNames ) const = 0;

    /** Implementation of opening a storage element. */
    virtual StorageRef  implOpenSubStorage( const ::rtl::OUString& rElementName, bool bCreate ) = 0;

    /** Implementation of opening an input stream element. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        implOpenInputStream( const ::rtl::OUString& rElementName ) = 0;

    /** Implementation of opening an output stream element. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                        implOpenOutputStream( const ::rtl::OUString& rElementName ) = 0;

    /** Commits the current storage. */
    virtual void        implCommit() const = 0;

    /** Helper that opens and caches the specified direct substorage. */
    StorageRef          getSubStorage( const ::rtl::OUString& rElementName, bool bCreateMissing );

private:
    typedef RefMap< ::rtl::OUString, StorageBase > SubStorageMap;

    SubStorageMap       maSubStorages;      /// Map of direct sub storages.
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        mxInStream;         /// Cached base input stream (to keep it alive).
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >
                        mxOutStream;        /// Cached base output stream (to keep it alive).
    ::rtl::OUString     maParentPath;       /// Full path of parent storage.
    ::rtl::OUString     maStorageName;      /// Name of this storage, if it is a substorage.
    bool                mbBaseStreamAccess; /// True = access base streams with empty stream name.
    bool                mbReadOnly;         /// True = storage opened read-only (based on input stream).
};

// ============================================================================

} // namespace oox

#endif
