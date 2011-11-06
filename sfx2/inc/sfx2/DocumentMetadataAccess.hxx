/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

