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

#include <vbahelper/collectionbase.hxx>

#include <map>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <cppuhelper/implbase2.hxx>

namespace vbahelper {

using namespace ::com::sun::star;
using namespace ::ooo::vba;

// ============================================================================

namespace {

// ----------------------------------------------------------------------------

class CollectionEnumeration : public ::cppu::WeakImplHelper1< container::XEnumeration >
{
public:
    explicit CollectionEnumeration( const ::rtl::Reference< CollectionBase >& rxCollection );
    virtual sal_Bool SAL_CALL hasMoreElements() throw (uno::RuntimeException);
    virtual uno::Any SAL_CALL nextElement() throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);

private:
    ::rtl::Reference< CollectionBase > mxCollection;
    sal_Int32 mnCurrIndex;
};

CollectionEnumeration::CollectionEnumeration( const ::rtl::Reference< CollectionBase >& rxCollection ) :
    mxCollection( rxCollection ),
    mnCurrIndex( 1 )    // collection expects one-based indexes
{
}

sal_Bool SAL_CALL CollectionEnumeration::hasMoreElements() throw (uno::RuntimeException)
{
    return mnCurrIndex <= mxCollection->getCount();
}

uno::Any SAL_CALL CollectionEnumeration::nextElement() throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( hasMoreElements() )
        return mxCollection->getItemByIndex( mnCurrIndex++ );
    throw container::NoSuchElementException();
}

// ----------------------------------------------------------------------------

struct IsLessIgnoreCase
{
    inline bool operator()( const ::rtl::OUString& rName1, const ::rtl::OUString& rName2 ) const
        { return ::rtl_ustr_compareIgnoreAsciiCase_WithLength( rName1.getStr(), rName1.getLength(), rName2.getStr(), rName2.getLength() ) < 0; }
};

// ----------------------------------------------------------------------------

class SequenceToContainer : public ::cppu::WeakImplHelper2< container::XIndexAccess, container::XNameAccess >
{
public:
    explicit SequenceToContainer( const ::std::vector< uno::Reference< container::XNamed > >& rElements, const uno::Type& rElementType );
    explicit SequenceToContainer( const ::std::vector< beans::NamedValue >& rElements, const uno::Type& rElementType );
    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw (uno::RuntimeException);
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException);
    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const ::rtl::OUString& rName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);
    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames() throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& rName ) throw (uno::RuntimeException);
    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw (uno::RuntimeException);

private:
    typedef uno::Sequence< ::rtl::OUString > ElementNameSequence;
    typedef ::std::vector< uno::Any > ElementVector;
    typedef ::std::map< ::rtl::OUString, uno::Any, IsLessIgnoreCase > ElementMap;

    ElementNameSequence maElementNames;
    ElementVector maElements;
    ElementMap maElementMap;
    uno::Type maElementType;
};

SequenceToContainer::SequenceToContainer( const ::std::vector< uno::Reference< container::XNamed > >& rElements, const uno::Type& rElementType ) :
    maElementType( rElementType )
{
    maElementNames.realloc( static_cast< sal_Int32 >( rElements.size() ) );
    maElements.reserve( rElements.size() );
    ::rtl::OUString* pElementName = maElementNames.getArray();
    for( ::std::vector< uno::Reference< container::XNamed > >::const_iterator aIt = rElements.begin(), aEnd = rElements.end(); aIt != aEnd; ++aIt, ++pElementName )
    {
        uno::Reference< container::XNamed > xNamed = *aIt;
        *pElementName = xNamed->getName();
        maElements.push_back( uno::Any( xNamed ) );
        // same name may occur multiple times, VBA returns first occurance
        if( maElementMap.count( *pElementName ) == 0 )
            maElementMap[ *pElementName ] <<= xNamed;
    }
}

SequenceToContainer::SequenceToContainer( const ::std::vector< beans::NamedValue >& rElements, const uno::Type& rElementType ) :
    maElementType( rElementType )
{
    maElementNames.realloc( static_cast< sal_Int32 >( rElements.size() ) );
    maElements.reserve( rElements.size() );
    ::rtl::OUString* pElementName = maElementNames.getArray();
    for( ::std::vector< beans::NamedValue >::const_iterator aIt = rElements.begin(), aEnd = rElements.end(); aIt != aEnd; ++aIt, ++pElementName )
    {
        *pElementName = aIt->Name;
        maElements.push_back( aIt->Value );
        // same name may occur multiple times, VBA returns first occurance
        if( maElementMap.count( *pElementName ) == 0 )
            maElementMap[ *pElementName ] = aIt->Value;
    }
}

sal_Int32 SAL_CALL SequenceToContainer::getCount() throw (uno::RuntimeException)
{
    return static_cast< sal_Int32 >( maElements.size() );
}

uno::Any SAL_CALL SequenceToContainer::getByIndex( sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( (0 <= nIndex) && (nIndex < getCount()) )
        return maElements[ static_cast< size_t >( nIndex ) ];
    throw lang::IndexOutOfBoundsException();
}

uno::Any SAL_CALL SequenceToContainer::getByName( const ::rtl::OUString& rName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    ElementMap::iterator aIt = maElementMap.find( rName );
    if( aIt != maElementMap.end() )
        return aIt->second;
    throw container::NoSuchElementException();
}

uno::Sequence< ::rtl::OUString > SAL_CALL SequenceToContainer::getElementNames() throw (uno::RuntimeException)
{
    return maElementNames;
}

sal_Bool SAL_CALL SequenceToContainer::hasByName( const ::rtl::OUString& rName ) throw (uno::RuntimeException)
{
    return maElementMap.count( rName ) > 0;
}

uno::Type SAL_CALL SequenceToContainer::getElementType() throw (uno::RuntimeException)
{
    return maElementType;
}

sal_Bool SAL_CALL SequenceToContainer::hasElements() throw (uno::RuntimeException)
{
    return !maElements.empty();
}

} // namespace

// ============================================================================

CollectionBase::CollectionBase( const uno::Type& rElementType ) :
    maElementType( rElementType ),
    mbConvertOnDemand( false )
{
}

sal_Int32 SAL_CALL CollectionBase::getCount() throw (uno::RuntimeException)
{
    if( mxIndexAccess.is() )
        return mxIndexAccess->getCount();
    if( mxNameAccess.is() )
        return mxNameAccess->getElementNames().getLength();
    throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "No element container set." ) ), 0 );
}

uno::Reference< container::XEnumeration > SAL_CALL CollectionBase::createEnumeration() throw (uno::RuntimeException)
{
    return new CollectionEnumeration( this );
}

uno::Type SAL_CALL CollectionBase::getElementType() throw (uno::RuntimeException)
{
    return maElementType;
}

sal_Bool SAL_CALL CollectionBase::hasElements() throw (uno::RuntimeException)
{
    if( mxIndexAccess.is() )
        return mxIndexAccess->hasElements();
    if( mxNameAccess.is() )
        return mxNameAccess->hasElements();
    throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "No element container set." ) ), 0 );
}

::rtl::OUString SAL_CALL CollectionBase::getDefaultMethodName() throw (uno::RuntimeException)
{
    static ::rtl::OUString saDefMethodName( RTL_CONSTASCII_USTRINGPARAM( "Item" ) );
    return saDefMethodName;
}

// ----------------------------------------------------------------------------

void CollectionBase::initContainer(
        const uno::Reference< container::XElementAccess >& rxElementAccess,
        ContainerType eContainerType ) throw (uno::RuntimeException)
{
    mxIndexAccess.set( rxElementAccess, uno::UNO_QUERY );
    mxNameAccess.set( rxElementAccess, uno::UNO_QUERY );
    switch( eContainerType )
    {
        case CONTAINER_NATIVE_VBA:
            mbConvertOnDemand = false;
        break;
        case CONTAINER_CONVERT_ON_DEMAND:
            mbConvertOnDemand = true;
        break;
    }
}

void CollectionBase::initElements( const ::std::vector< uno::Reference< container::XNamed > >& rElements, ContainerType eContainerType ) throw (uno::RuntimeException)
{
    // SequenceToContainer derives twice from XElementAccess, need to resolve ambiguity
    initContainer( static_cast< container::XIndexAccess* >( new SequenceToContainer( rElements, maElementType ) ), eContainerType );
}

void CollectionBase::initElements( const ::std::vector< beans::NamedValue >& rElements, ContainerType eContainerType ) throw (uno::RuntimeException)
{
    // SequenceToContainer derives twice from XElementAccess, need to resolve ambiguity
    initContainer( static_cast< container::XIndexAccess* >( new SequenceToContainer( rElements, maElementType ) ), eContainerType );
}

uno::Any CollectionBase::createCollectionItem( const uno::Any& rElement, const uno::Any& rIndex ) throw (css::uno::RuntimeException)
{
    uno::Any aItem = mbConvertOnDemand ? implCreateCollectionItem( rElement, rIndex ) : rElement;
    if( aItem.hasValue() )
        return aItem;
    throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid item." ) ), 0 );
}

uno::Any CollectionBase::getItemByIndex( sal_Int32 nIndex ) throw (uno::RuntimeException)
{
    if( mxIndexAccess.is() )
    {
        if( (1 <= nIndex) && (nIndex <= mxIndexAccess->getCount()) )
            // createCollectionItem() will convert from container element to VBA item
            return createCollectionItem( mxIndexAccess->getByIndex( nIndex - 1 ), uno::Any( nIndex ) );
        throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Index out of bounds." ) ), 0 );
    }
    if( mxNameAccess.is() )
    {
        uno::Sequence< ::rtl::OUString > aElementNames = mxNameAccess->getElementNames();
        if( (1 <= nIndex) && (nIndex <= aElementNames.getLength()) )
            // createCollectionItem() will convert from container element to VBA item
            return createCollectionItem( mxNameAccess->getByName( aElementNames[ nIndex - 1 ] ), uno::Any( aElementNames[ nIndex - 1 ] ) );
        throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Index out of bounds." ) ), 0 );
    }
    throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "No element container set." ) ), 0 );
}

uno::Any CollectionBase::getItemByName( const ::rtl::OUString& rName ) throw (uno::RuntimeException)
{
    if( mxNameAccess.is() )
    {
        if( rName.getLength() > 0 )
            // createCollectionItem() will convert from container element to VBA item
            return createCollectionItem( mxNameAccess->getByName( rName ), uno::Any( rName ) );
        throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid item name." ) ), 0 );
    }
    if( mxIndexAccess.is() )
    {
        for( sal_Int32 nIndex = 0, nSize = mxIndexAccess->getCount(); nIndex < nSize; ++nIndex )
        {
            uno::Any aElement = mxIndexAccess->getByIndex( nIndex );
            uno::Reference< container::XNamed > xNamed( aElement, uno::UNO_QUERY );
            if( xNamed.is() && xNamed->getName().equalsIgnoreAsciiCase( rName ) )
                // createCollectionItem() will convert from container element to VBA item
                return createCollectionItem( aElement, uno::Any( nIndex ) );
        }
        throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid item name." ) ), 0 );
    }
    throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "No element container set." ) ), 0 );
}

uno::Any CollectionBase::getAnyItemOrThis( const uno::Any& rIndex ) throw (uno::RuntimeException)
{
    if( !rIndex.hasValue() )
        return uno::Any( uno::Reference< XCollectionBase >( this ) );
    if( rIndex.has< sal_Int32 >() )
        return getItemByIndex( rIndex.get< sal_Int32 >() );
    if( rIndex.has< ::rtl::OUString >() )
        return getItemByName( rIndex.get< ::rtl::OUString >() );
    throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid item index." ) ), 0 );
}

// protected ------------------------------------------------------------------

uno::Any CollectionBase::implCreateCollectionItem( const uno::Any& /*rElement*/, const uno::Any& /*rIndex*/ ) throw (uno::RuntimeException)
{
    throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Creation of VBA implementation object not implemented." ) ), 0 );
}

// ============================================================================

} // namespace vbahelper
