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

#ifndef INCLUDED_OOX_HELPER_ZIPSTORAGE_HXX
#define INCLUDED_OOX_HELPER_ZIPSTORAGE_HXX

#include <oox/helper/storagebase.hxx>

namespace com { namespace sun { namespace star {
    namespace uno { class XComponentContext; }
} } }

namespace oox {



/** Implements stream access for ZIP storages containing XML streams. */
class ZipStorage : public StorageBase
{
public:
    explicit            ZipStorage(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStream );

    explicit            ZipStorage(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& rxStream );

    virtual             ~ZipStorage();

private:
    explicit            ZipStorage(
                            const ZipStorage& rParentStorage,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage,
                            const OUString& rElementName );

    /** Returns true, if the object represents a valid storage. */
    virtual bool        implIsStorage() const override;

    /** Returns the com.sun.star.embed.XStorage interface of the current storage. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
                        implGetXStorage() const override;

    /** Returns the names of all elements of this storage. */
    virtual void        implGetElementNames( ::std::vector< OUString >& orElementNames ) const override;

    /** Opens and returns the specified sub storage from the storage. */
    virtual StorageRef  implOpenSubStorage( const OUString& rElementName, bool bCreateMissing ) override;

    /** Opens and returns the specified input stream from the storage. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        implOpenInputStream( const OUString& rElementName ) override;

    /** Opens and returns the specified output stream from the storage. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                        implOpenOutputStream( const OUString& rElementName ) override;

    /** Commits the current storage. */
    virtual void        implCommit() const override;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
                        mxStorage;      ///< Storage based on input or output stream.
};



} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
