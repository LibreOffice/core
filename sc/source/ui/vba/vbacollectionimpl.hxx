/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vbacollectionimpl.hxx,v $
 * $Revision: 1.5 $
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
#ifndef SC_VBA_COLLECTION_IMPL_HXX
#define SC_VBA_COLLECTION_IMPL_HXX

#include <ooo/vba/XCollection.hpp>
//#include <ooo/vba/XCollectionTest.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/script/XDefaultMethod.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase1.hxx>

#include "vbahelper.hxx"
#include "vbahelperinterface.hxx"
#include "vbaglobals.hxx"

#include <vector>

typedef ::cppu::WeakImplHelper1< css::container::XEnumeration > EnumerationHelper_BASE;

class EnumerationHelperImpl : public EnumerationHelper_BASE
{
protected:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::container::XEnumeration > m_xEnumeration;
public:

    EnumerationHelperImpl( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XEnumeration >& xEnumeration ) throw ( css::uno::RuntimeException ) : m_xContext( xContext ),  m_xEnumeration( xEnumeration ) { }
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (css::uno::RuntimeException) { return m_xEnumeration->hasMoreElements(); }
};

// a wrapper class for a providing a XIndexAccess, XNameAccess, XEnumerationAccess impl based on providing a vector of interfaces
// only requirement is the object needs to implement XName



typedef ::cppu::WeakImplHelper3< css::container::XNameAccess, css::container::XIndexAccess, css::container::XEnumerationAccess > XNamedCollectionHelper_BASE;

template< typename Ifc1 >
class XNamedObjectCollectionHelper : public XNamedCollectionHelper_BASE
{
public:
typedef std::vector< css::uno::Reference< Ifc1 > >  XNamedVec;
private:

    class XNamedEnumerationHelper : public EnumerationHelper_BASE
    {
        XNamedVec mXNamedVec;
        typename XNamedVec::iterator mIt;
    public:
            XNamedEnumerationHelper( const XNamedVec& sMap ) : mXNamedVec( sMap ), mIt( mXNamedVec.begin() ) {}

            virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (css::uno::RuntimeException)
            {
            return ( mIt != mXNamedVec.end() );
            }

            virtual css::uno::Any SAL_CALL nextElement(  ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException)
            {
                    if ( hasMoreElements() )
                return css::uno::makeAny( *mIt++ );
                    throw css::container::NoSuchElementException();
            }
    };

protected:
    XNamedVec mXNamedVec;
    typename XNamedVec::iterator cachePos;
public:
    XNamedObjectCollectionHelper( const XNamedVec& sMap ) : mXNamedVec( sMap ), cachePos(mXNamedVec.begin()) {}
    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw (css::uno::RuntimeException) { return  Ifc1::static_type(0); }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (css::uno::RuntimeException) { return ( mXNamedVec.size() > 0 ); }
    // XNameAcess
    virtual css::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException)
    {
        if ( !hasByName(aName) )
            throw css::container::NoSuchElementException();
        return css::uno::makeAny( *cachePos );
    }
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (css::uno::RuntimeException)
    {
        css::uno::Sequence< rtl::OUString > sNames( mXNamedVec.size() );
        rtl::OUString* pString = sNames.getArray();
        typename XNamedVec::iterator it = mXNamedVec.begin();
        typename XNamedVec::iterator it_end = mXNamedVec.end();

        for ( ; it != it_end; ++it, ++pString )
        {
            css::uno::Reference< css::container::XNamed > xName( *it, css::uno::UNO_QUERY_THROW );
            *pString = xName->getName();
        }
        return sNames;
    }
    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (css::uno::RuntimeException)
    {
        cachePos = mXNamedVec.begin();
        typename XNamedVec::iterator it_end = mXNamedVec.end();
        for ( ; cachePos != it_end; ++cachePos )
        {
            css::uno::Reference< css::container::XNamed > xName( *cachePos, css::uno::UNO_QUERY_THROW );
            if ( aName.equals( xName->getName() ) )
                break;
        }
        return ( cachePos != it_end );
    }

    // XElementAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (css::uno::RuntimeException) { return mXNamedVec.size(); }
    virtual css::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException )
    {
        if ( Index < 0 || Index >= getCount() )
            throw css::lang::IndexOutOfBoundsException();

        return css::uno::makeAny( mXNamedVec[ Index ] );

    }
    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration(  ) throw (css::uno::RuntimeException)
    {
        return new XNamedEnumerationHelper( mXNamedVec );
    }
};

// including a HelperInterface implementation
template< typename Ifc1 >
class ScVbaCollectionBase : public InheritedHelperInterfaceImpl< Ifc1 >
{
typedef InheritedHelperInterfaceImpl< Ifc1 > BaseColBase;
protected:
    css::uno::Reference< css::container::XIndexAccess > m_xIndexAccess;
    css::uno::Reference< css::container::XNameAccess > m_xNameAccess;

    virtual css::uno::Any getItemByStringIndex( const rtl::OUString& sIndex ) throw (css::uno::RuntimeException)
    {
        if ( !m_xNameAccess.is() )
            throw css::uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ScVbaCollectionBase string index access not supported by this object") ), css::uno::Reference< css::uno::XInterface >() );

        return createCollectionObject( m_xNameAccess->getByName( sIndex ) );
    }

    virtual css::uno::Any getItemByIntIndex( const sal_Int32 nIndex ) throw (css::uno::RuntimeException)
    {
        if ( !m_xIndexAccess.is() )
            throw css::uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ScVbaCollectionBase numeric index access not supported by this object") ), css::uno::Reference< css::uno::XInterface >() );
        if ( nIndex <= 0 )
        {
            throw  css::lang::IndexOutOfBoundsException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                "index is 0 or negative" ) ),
                css::uno::Reference< css::uno::XInterface >() );
        }
        // need to adjust for vba index ( for which first element is 1 )
        return createCollectionObject( m_xIndexAccess->getByIndex( nIndex - 1 ) );
    }
public:
    ScVbaCollectionBase( const css::uno::Reference< ov::XHelperInterface >& xParent,   const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess ) : BaseColBase( xParent, xContext ), m_xIndexAccess( xIndexAccess ){ m_xNameAccess.set(m_xIndexAccess, css::uno::UNO_QUERY); }
    //XCollection
    virtual ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException)
    {
        return m_xIndexAccess->getCount();
    }

    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index1, const css::uno::Any& /*not processed in this base class*/ ) throw (css::uno::RuntimeException)
    {
        if ( Index1.getValueTypeClass() != css::uno::TypeClass_STRING )
        {
            sal_Int32 nIndex = 0;

            if ( ( Index1 >>= nIndex ) != sal_True )
            {
                rtl::OUString message;
                message = rtl::OUString::createFromAscii(
                    "Couldn't convert index to Int32");
                throw  css::lang::IndexOutOfBoundsException( message,
                    css::uno::Reference< css::uno::XInterface >() );
            }
            return  getItemByIntIndex( nIndex );
        }
        rtl::OUString aStringSheet;

        Index1 >>= aStringSheet;
        return getItemByStringIndex( aStringSheet );
    }
    // XDefaultMethod
    ::rtl::OUString SAL_CALL getDefaultMethodName(  ) throw (css::uno::RuntimeException)
    {
        const static rtl::OUString sName( RTL_CONSTASCII_USTRINGPARAM("Item") );
        return sName;
    }
    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) = 0;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) = 0;
    // XElementAccess
    virtual ::sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException)
    {
        return ( m_xIndexAccess->getCount() > 0 );
    }
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) = 0;

};

typedef ::cppu::WeakImplHelper1<ov::XCollection> XCollection_InterfacesBASE;

typedef ScVbaCollectionBase< XCollection_InterfacesBASE > CollImplBase1;
// compatible with the old collections ( pre XHelperInterface base class ) ( some internal objects still use this )
class ScVbaCollectionBaseImpl : public CollImplBase1
{
public:
    ScVbaCollectionBaseImpl( const css::uno::Reference< ov::XHelperInterface > xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess ) throw( css::uno::RuntimeException ) : CollImplBase1( xParent, xContext, xIndexAccess){}

};

template <typename Ifc> // where Ifc must implement XCollectionTest
class CollTestImplHelper :  public ScVbaCollectionBase< ::cppu::WeakImplHelper1< Ifc > >
{
typedef ScVbaCollectionBase< ::cppu::WeakImplHelper1< Ifc >  > ImplBase1;

public:
    CollTestImplHelper( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext,  const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess ) throw( css::uno::RuntimeException ) : ImplBase1( xParent, xContext, xIndexAccess ) {}
};


#endif //SC_VBA_COLLECTION_IMPL_HXX
