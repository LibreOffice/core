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

#ifndef INCLUDED_SFX2_DOCUMENTMETADATAACCESS_HXX
#define INCLUDED_SFX2_DOCUMENTMETADATAACCESS_HXX

#include <sal/config.h>

#include <sfx2/dllapi.h>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>
#include <com/sun/star/rdf/XRepositorySupplier.hpp>

#include <cppuhelper/implbase1.hxx>

#include <memory>


/** Implementation of the interface com.sun.star.rdf.XDocumentMetadataAccess

    This is not a service only because it needs some kind of XML ID registry
    from the document, and i do not like defining an API for that.
    Also, the implementation does _no_ locking, so make sure access is
    protected externally.
 */

namespace com { namespace sun { namespace star { namespace embed {
    class XStorage;
} } } }

namespace sfx2 {


/** create a base URI for loading metadata from an ODF (sub)document.

    @param i_xContext   component context
    @param i_xStorage   storage for the document; FileSystemStorage is allowed
    @param i_rPkgURI    the URI for the package
    @param i_rSubDocument   (optional) path of the subdocument in package

    @return a base URI suitable for XDocumentMetadataAccess::loadFromStorage
 */
css::uno::Reference< css::rdf::XURI> SFX2_DLLPUBLIC
createBaseURI(
    css::uno::Reference< css::uno::XComponentContext> const & i_xContext,
    css::uno::Reference< css::embed::XStorage> const & i_xStorage,
    OUString const & i_rPkgURI,
    OUString const & i_rSubDocument = OUString());


class IXmlIdRegistrySupplier;
struct DocumentMetadataAccess_Impl;


class SFX2_DLLPUBLIC DocumentMetadataAccess :
    public ::cppu::WeakImplHelper1< css::rdf::XDocumentMetadataAccess>
{
    DocumentMetadataAccess(const DocumentMetadataAccess&) = delete;
    DocumentMetadataAccess& operator=( const DocumentMetadataAccess& ) = delete;
public:
    explicit DocumentMetadataAccess(css::uno::Reference< css::uno::XComponentContext > const & i_xContext,
                IXmlIdRegistrySupplier const & i_rRegistrySupplier,
                OUString const & i_rBaseURI);
    // N.B.: in contrast to previous, this constructor does _not_ initialize!
    //       caller must immediately call loadFromStorage/Medium!
    explicit DocumentMetadataAccess(css::uno::Reference< css::uno::XComponentContext > const & i_xContext,
                IXmlIdRegistrySupplier const & i_rRegistrySupplier);
    virtual ~DocumentMetadataAccess();

    // css::rdf::XNode:
    virtual OUString SAL_CALL getStringValue()
        throw (css::uno::RuntimeException, std::exception) override;

    // css::rdf::XURI:
    virtual OUString SAL_CALL getNamespace()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getLocalName()
        throw (css::uno::RuntimeException, std::exception) override;

    // css::rdf::XRepositorySupplier:
    virtual css::uno::Reference<
        css::rdf::XRepository > SAL_CALL getRDFRepository()
        throw (css::uno::RuntimeException, std::exception) override;

    // css::rdf::XDocumentMetadataAccess:
    virtual css::uno::Reference<
                css::rdf::XMetadatable > SAL_CALL
        getElementByMetadataReference(
            const css::beans::StringPair & i_rReference)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::rdf::XMetadatable > SAL_CALL
        getElementByURI(const css::uno::Reference< css::rdf::XURI > & i_xURI)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Reference< css::rdf::XURI > > SAL_CALL getMetadataGraphsWithType(
            const css::uno::Reference< css::rdf::XURI > & i_xType)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException, std::exception) override;
    virtual css::uno::Reference< css::rdf::XURI> SAL_CALL
        addMetadataFile(const OUString & i_rFileName,
            const css::uno::Sequence< css::uno::Reference< css::rdf::XURI > > & i_rTypes)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::container::ElementExistException, std::exception) override;
    virtual css::uno::Reference< css::rdf::XURI> SAL_CALL
        importMetadataFile(::sal_Int16 i_Format,
            const css::uno::Reference< css::io::XInputStream > & i_xInStream,
            const OUString & i_rFileName,
            const css::uno::Reference< css::rdf::XURI > & i_xBaseURI,
            const css::uno::Sequence< css::uno::Reference< css::rdf::XURI > > & i_rTypes)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::datatransfer::UnsupportedFlavorException,
            css::container::ElementExistException,
            css::rdf::ParseException,
            css::io::IOException, std::exception) override;
    virtual void SAL_CALL removeMetadataFile(
            const css::uno::Reference<
                css::rdf::XURI > & i_xGraphName)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::container::NoSuchElementException, std::exception) override;
    virtual void SAL_CALL addContentOrStylesFile(
            const OUString & i_rFileName)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::container::ElementExistException, std::exception) override;
    virtual void SAL_CALL removeContentOrStylesFile(
            const OUString & i_rFileName)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::container::NoSuchElementException, std::exception) override;

    virtual void SAL_CALL loadMetadataFromStorage(
            const css::uno::Reference<
                css::embed::XStorage > & i_xStorage,
            const css::uno::Reference<
                css::rdf::XURI > & i_xBaseURI,
            const css::uno::Reference<
                css::task::XInteractionHandler> & i_xHandler)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException, std::exception) override;
    virtual void SAL_CALL storeMetadataToStorage(
            const css::uno::Reference<
                css::embed::XStorage > & i_xStorage)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException, std::exception) override;
    virtual void SAL_CALL loadMetadataFromMedium(
            const css::uno::Sequence<
                css::beans::PropertyValue > & i_rMedium)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException, std::exception) override;
    virtual void SAL_CALL storeMetadataToMedium(
            const css::uno::Sequence<
                css::beans::PropertyValue > & i_rMedium)
        throw (css::uno::RuntimeException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException, std::exception) override;

private:
    std::unique_ptr<DocumentMetadataAccess_Impl> m_pImpl;
};

} // namespace sfx2

#endif // INCLUDED_SFX2_DOCUMENTMETADATAACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
