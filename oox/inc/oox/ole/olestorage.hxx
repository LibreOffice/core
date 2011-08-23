/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef OOX_OLE_OLESTORAGE_HXX
#define OOX_OLE_OLESTORAGE_HXX

#include "oox/helper/storagebase.hxx"

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace container { class XNameContainer; }
} } }

namespace oox {
namespace ole {

// ============================================================================

/** Implements stream access for binary OLE storages. */
class OleStorage : public StorageBase
{
public:
    explicit            OleStorage(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream,
                            bool bBaseStreamAccess );

    explicit            OleStorage(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rxOutStream,
                            bool bBaseStreamAccess );

    virtual             ~OleStorage();

private:
    explicit            OleStorage(
                            const OleStorage& rParentStorage,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rxStorage,
                            const ::rtl::OUString& rElementName,
                            bool bReadOnly );
    explicit            OleStorage(
                            const OleStorage& rParentStorage,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rxOutStream,
                            const ::rtl::OUString& rElementName );

    /** Initializes the API storage object for input. */
    void                initStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream );
    /** Initializes the API storage object for input/output. */
    void                initStorage( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rxOutStream );
    
    /** Returns true, if the object represents a valid storage. */
    virtual bool        implIsStorage() const;

    /** Returns the com.sun.star.embed.XStorage interface of the current storage.

        @attention
            This function is not implemented for binary OLE storages.
     */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
                        implGetXStorage() const;

    /** Returns the names of all elements of this storage. */
    virtual void        implGetElementNames( ::std::vector< ::rtl::OUString >& orElementNames ) const;

    /** Opens and returns the specified sub storage from the storage. */
    virtual StorageRef  implOpenSubStorage( const ::rtl::OUString& rElementName, bool bCreateMissing );

    /** Opens and returns the specified input stream from the storage. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        implOpenInputStream( const ::rtl::OUString& rElementName );

    /** Opens and returns the specified output stream from the storage. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                        implOpenOutputStream( const ::rtl::OUString& rElementName );

    /** Commits the current storage. */
    virtual void        implCommit() const;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        mxFactory;          /// Factory for storage/stream creation.
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        mxStorage;          /// Access to elements of this sub storage.
    const OleStorage*   mpParentStorage;    /// Parent OLE storage that contains this storage.
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
