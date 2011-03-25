/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2011 Oracle and/or its affiliates.
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

#ifndef VBAHELPER_COLLECTIONBASE_HXX
#define VBAHELPER_COLLECTIONBASE_HXX

#include <vector>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XElementAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <ooo/vba/XCollectionBase.hpp>
#include <cppuhelper/implbase1.hxx>
#include <vbahelper/vbahelper.hxx>

namespace vbahelper {

// ============================================================================

typedef ::cppu::WeakImplHelper1< ov::XCollectionBase > CollectionBase_BASE;

/** Base class of VBA objects implementing the VBA collection concept.

    This base class intentionally does not include the interface
    XHelperInterface supported by all application VBA object. There may be
    other VBA objects that do not support the special methods provided by
    XHelperInterface.
 */
class VBAHELPER_DLLPUBLIC CollectionBase : public CollectionBase_BASE
{
public:
    /** Enumerates different container types a VBA collection can be based on. */
    enum ContainerType
    {
        /** Container elements are VBA items.

            The initial container contains the final VBA items provided by the
            VBA collection. No conversion takes place on item access.
         */
        CONTAINER_NATIVE_VBA,

        /** Container elements will be converted to VBA items on demand.

            The initial container contains intermediate objects (e.g. UNO
            objects) which will be converted to VBA items everytime the item is
            accessed (e.g. item access method, enumeration). Changes in the
            initial container are reflected by the collection.
         */
        CONTAINER_CONVERT_ON_DEMAND,
    };

    // ------------------------------------------------------------------------

    CollectionBase( const css::uno::Type& rElementType );

    // ------------------------------------------------------------------------

    // attributes
    virtual sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException);
    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException);
    // XDefaultMethod
    virtual ::rtl::OUString SAL_CALL getDefaultMethodName() throw (css::uno::RuntimeException);

    // ------------------------------------------------------------------------

    /** Associates this collection with the passed UNO container.

        @param rxElementAccess
            The UNO container with the elements of this collection. Shall
            support either XIndexAccess or XNameAccess, may support both.

            If the container does not support XIndexAccess, index access is
            simulated based on the order returned by the function
            XNameAccess::getElementNames().

            If the container does not support XNameAccess, name access is
            simulated by iterating the elements via index access and asking the
            elements for their name via the interface XNamed. If the elements
            do not support XNamed, the elements cannot be accessed by name.

        @param eContainerType
            Specifies the type of the passed container.
     */
    void initContainer(
        const css::uno::Reference< css::container::XElementAccess >& rxElementAccess,
        ContainerType eContainerType ) throw (css::uno::RuntimeException);

    /** Initializes this collection with copies of all elements in the passed
        temporary STL vector.

        @param rElements
            The STL vector with the named elements of this collection.
        @param eContainerType
            Specifies the type of the passed vector.
     */
    void initElements(
        const ::std::vector< css::uno::Reference< css::container::XNamed > >& rElements,
        ContainerType eContainerType ) throw (css::uno::RuntimeException);

    /** Initializes this collection with copies of all elements in the passed
        temporary STL vector.

        @param rElements
            The STL vector with the named elements of this collection.
        @param eContainerType
            Specifies the type of the passed vector.
     */
    void initElements(
        const ::std::vector< css::beans::NamedValue >& rElements,
        ContainerType eContainerType ) throw (css::uno::RuntimeException);

    /** Returns a VBA implementation object from the passed element.

        If the container type is CONTAINER_NATIVE_VBA, returns the passed
        object unmodified. If the container type is CONTAINER_CONVERT_ON_DEMAND,
        calls the virtual function implCreateCollectionItem() that implements
        creation of the VBA implmentation object.

        @param rElement
            The container element the VBA implementation object is based on.

        @param rIndex
            The index or name that has been used to access the item.
     */
    css::uno::Any createCollectionItem(
        const css::uno::Any& rElement,
        const css::uno::Any& rIndex ) throw (css::uno::RuntimeException);

    /** Returns a collection item specified by its one-based item index.

        @param nIndex
            The one-based index of the collection item.
    */
    css::uno::Any getItemByIndex( sal_Int32 nIndex ) throw (css::uno::RuntimeException);

    /** Returns a collection item specified by its name.

        @param rName
            The name of the collection item.
    */
    css::uno::Any getItemByName( const ::rtl::OUString& rName ) throw (css::uno::RuntimeException);

    /** Returns a collection item specified by its index or name.

        @param rIndex
            The index or name of the collection item. May be empty, in that
            case the entire collection is returned.
    */
    css::uno::Any getAnyItemOrThis( const css::uno::Any& rIndex ) throw (css::uno::RuntimeException);

    /** Returns a collection item of a specific type specified by its index or
        name.

        @param rIndex
            The index or name of the collection item.
    */
    template< typename XType >
    inline css::uno::Reference< XType > getAnyItem( const css::uno::Any& rIndex ) throw (css::uno::RuntimeException)
        { css::uno::Any aRet; if( rIndex.hasValue() ) aRet = getAnyItemOrThis( rIndex ); return css::uno::Reference< XType >( aRet, css::uno::UNO_QUERY_THROW ); }

protected:
    /** Derived classes implement creation of a VBA implementation object from
        the passed intermediate container element.

        May be kept unimplemented if container type is CONTAINER_NATIVE_VBA.

        @param rElement
            The container element the VBA implementation object is based on.

        @param rIndex
            The index or name used to access the item. Can be used by
            implementations as a hint how to find or convert the VBA object.
     */
    virtual css::uno::Any implCreateCollectionItem( const css::uno::Any& rElement, const css::uno::Any& rIndex ) throw (css::uno::RuntimeException);

private:
    css::uno::Reference< css::container::XIndexAccess > mxIndexAccess;
    css::uno::Reference< css::container::XNameAccess > mxNameAccess;
    css::uno::Type maElementType;
    bool mbConvertOnDemand;
};

// ============================================================================

} // namespace vbahelper

#endif
