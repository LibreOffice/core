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

#ifndef INCLUDED_OOX_OLE_OLESTORAGE_HXX
#define INCLUDED_OOX_OLE_OLESTORAGE_HXX

#include <oox/helper/storagebase.hxx>

namespace com { namespace sun { namespace star {
    namespace container { class XNameContainer; }
    namespace uno { class XComponentContext; }
} } }

namespace oox {
namespace ole {



/** Implements stream access for binary OLE storages. */
class OOX_DLLPUBLIC OleStorage : public StorageBase
{
public:
    explicit            OleStorage(
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            const css::uno::Reference< css::io::XInputStream >& rxInStream,
                            bool bBaseStreamAccess );

    explicit            OleStorage(
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            const css::uno::Reference< css::io::XStream >& rxOutStream,
                            bool bBaseStreamAccess );

    virtual             ~OleStorage();

private:
    explicit            OleStorage(
                            const OleStorage& rParentStorage,
                            const css::uno::Reference< css::container::XNameContainer >& rxStorage,
                            const OUString& rElementName,
                            bool bReadOnly );
    explicit            OleStorage(
                            const OleStorage& rParentStorage,
                            const css::uno::Reference< css::io::XStream >& rxOutStream,
                            const OUString& rElementName );

    /** Initializes the API storage object for input. */
    void                initStorage( const css::uno::Reference< css::io::XInputStream >& rxInStream );
    /** Initializes the API storage object for input/output. */
    void                initStorage( const css::uno::Reference< css::io::XStream >& rxOutStream );

    /** Returns true, if the object represents a valid storage. */
    virtual bool        implIsStorage() const override;

    /** Returns the com.sun.star.embed.XStorage interface of the current storage.

        @attention
            This function is not implemented for binary OLE storages.
     */
    virtual css::uno::Reference< css::embed::XStorage >
                        implGetXStorage() const override;

    /** Returns the names of all elements of this storage. */
    virtual void        implGetElementNames( ::std::vector< OUString >& orElementNames ) const override;

    /** Opens and returns the specified sub storage from the storage. */
    virtual StorageRef  implOpenSubStorage( const OUString& rElementName, bool bCreateMissing ) override;

    /** Opens and returns the specified input stream from the storage. */
    virtual css::uno::Reference< css::io::XInputStream >
                        implOpenInputStream( const OUString& rElementName ) override;

    /** Opens and returns the specified output stream from the storage. */
    virtual css::uno::Reference< css::io::XOutputStream >
                        implOpenOutputStream( const OUString& rElementName ) override;

    /** Commits the current storage. */
    virtual void        implCommit() const override;

private:
    css::uno::Reference< css::uno::XComponentContext >
                        mxContext;          ///< Component context with service manager.
    css::uno::Reference< css::container::XNameContainer >
                        mxStorage;          ///< Access to elements of this sub storage.
    const OleStorage*   mpParentStorage;    ///< Parent OLE storage that contains this storage.
};



} // namespace ole
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
