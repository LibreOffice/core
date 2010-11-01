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

#ifndef _SFX_DOCUMENTMETADATAACCESS_HXX_
#define _SFX_DOCUMENTMETADATAACCESS_HXX_

#include <sal/config.h>

#include <sfx2/dllapi.h>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>
#include <com/sun/star/rdf/XRepositorySupplier.hpp>

#include <cppuhelper/implbase1.hxx>

#include <boost/utility.hpp>

#include <memory>


/** Implementation of the interface com.sun.star.rdf.XDocumentMetadataAccess

    This is not a service only because it needs some kind of XML ID registry
    from the document, and i do not like defining an API for that.
    Also, the implementation does _no_ locking, so make sure access is
    protected externally.

    @author mst
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
::com::sun::star::uno::Reference< ::com::sun::star::rdf::XURI> SFX2_DLLPUBLIC
createBaseURI(
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>
        const & i_xContext,
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage>
        const & i_xStorage,
    ::rtl::OUString const & i_rPkgURI,
    ::rtl::OUString const & i_rSubDocument = ::rtl::OUString());


class IXmlIdRegistrySupplier;
struct DocumentMetadataAccess_Impl;


class SFX2_DLLPUBLIC DocumentMetadataAccess :
    private boost::noncopyable,
    public ::cppu::WeakImplHelper1<
        ::com::sun::star::rdf::XDocumentMetadataAccess>
{
public:
    explicit DocumentMetadataAccess(::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext > const & i_xContext,
                IXmlIdRegistrySupplier const & i_rRegistrySupplier,
                ::rtl::OUString const & i_rBaseURI);
    // N.B.: in contrast to previous, this constructor does _not_ initialize!
    //       caller must immediately call loadFromStorage/Medium!
    explicit DocumentMetadataAccess(::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext > const & i_xContext,
                IXmlIdRegistrySupplier const & i_rRegistrySupplier);
    virtual ~DocumentMetadataAccess();

    // ::com::sun::star::rdf::XNode:
    virtual ::rtl::OUString SAL_CALL getStringValue()
        throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::rdf::XURI:
    virtual ::rtl::OUString SAL_CALL getNamespace()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLocalName()
        throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::rdf::XRepositorySupplier:
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::rdf::XRepository > SAL_CALL getRDFRepository()
        throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::rdf::XDocumentMetadataAccess:
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::rdf::XMetadatable > SAL_CALL
        getElementByMetadataReference(
            const ::com::sun::star::beans::StringPair & i_rReference)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::rdf::XMetadatable > SAL_CALL
        getElementByURI(const ::com::sun::star::uno::Reference<
            ::com::sun::star::rdf::XURI > & i_xURI)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::lang::IllegalArgumentException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference<
            ::com::sun::star::rdf::XURI > > SAL_CALL getMetadataGraphsWithType(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::rdf::XURI > & i_xType)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::lang::IllegalArgumentException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::rdf::XURI> SAL_CALL
        addMetadataFile(const ::rtl::OUString & i_rFileName,
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference< ::com::sun::star::rdf::XURI >
                > & i_rTypes)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::container::ElementExistException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::rdf::XURI> SAL_CALL
        importMetadataFile(::sal_Int16 i_Format,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::io::XInputStream > & i_xInStream,
            const ::rtl::OUString & i_rFileName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::rdf::XURI > & i_xBaseURI,
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference< ::com::sun::star::rdf::XURI >
                > & i_rTypes)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::datatransfer::UnsupportedFlavorException,
            ::com::sun::star::container::ElementExistException,
            ::com::sun::star::rdf::ParseException,
            ::com::sun::star::io::IOException);
    virtual void SAL_CALL removeMetadataFile(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::rdf::XURI > & i_xGraphName)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::container::NoSuchElementException);
    virtual void SAL_CALL addContentOrStylesFile(
            const ::rtl::OUString & i_rFileName)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::container::ElementExistException);
    virtual void SAL_CALL removeContentOrStylesFile(
            const ::rtl::OUString & i_rFileName)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::container::NoSuchElementException);

    virtual void SAL_CALL loadMetadataFromStorage(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::embed::XStorage > & i_xStorage,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::rdf::XURI > & i_xBaseURI,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::task::XInteractionHandler> & i_xHandler)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::lang::WrappedTargetException);
    virtual void SAL_CALL storeMetadataToStorage(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::embed::XStorage > & i_xStorage)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::lang::WrappedTargetException);
    virtual void SAL_CALL loadMetadataFromMedium(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyValue > & i_rMedium)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::lang::WrappedTargetException);
    virtual void SAL_CALL storeMetadataToMedium(
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyValue > & i_rMedium)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::lang::WrappedTargetException);

private:
    std::auto_ptr<DocumentMetadataAccess_Impl> m_pImpl;
};

} // namespace sfx2

#endif // _SFX_DOCUMENTMETADATAACCESS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
