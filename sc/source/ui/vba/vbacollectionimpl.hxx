/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbacollectionimpl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:03:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef SC_VBA_COLLECTION_IMPL_HXX
#define SC_VBA_COLLECTION_IMPL_HXX

#include <org/openoffice/vba/XCollection.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/script/XDefaultMethod.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase1.hxx>

#include "vbahelper.hxx"


typedef ::cppu::WeakImplHelper1< css::container::XEnumeration > EnumerationHelper_BASE;

class EnumerationHelperImpl : public EnumerationHelper_BASE
{
protected:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::container::XEnumeration > m_xEnumeration;
public:

    EnumerationHelperImpl( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XEnumeration >& xEnumeration ) throw ( css::uno::RuntimeException ) : m_xContext( xContext ),  m_xEnumeration( xEnumeration ) { }
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (css::uno::RuntimeException) { return m_xEnumeration->hasMoreElements(); }
    //virtual css::uno::Any SAL_CALL nextElement(  ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException) = 0;
};



typedef ::cppu::WeakImplHelper3<oo::vba::XCollection,
css::container::XEnumerationAccess, css::script::XDefaultMethod > XCollection_BASE;

class ScVbaCollectionBaseImpl : public XCollection_BASE
{
protected:
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    css::uno::Reference< css::container::XIndexAccess > m_xIndexAccess;
    css::uno::Reference< css::container::XNameAccess > m_xNameAccess;

    virtual css::uno::Any getItemByStringIndex( const rtl::OUString& sIndex ) throw (css::uno::RuntimeException);

    virtual css::uno::Any getItemByIntIndex( const sal_Int32 nIndex ) throw (css::uno::RuntimeException);
public:
    ScVbaCollectionBaseImpl( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess ) throw( css::uno::RuntimeException );

    //XCollection
    virtual ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getParent() throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getCreator() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< oo::excel::XApplication > SAL_CALL getApplication() throw (css::uno::RuntimeException);

    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index ) throw (css::uno::RuntimeException);
    // XDefaultMethod
    ::rtl::OUString SAL_CALL getDefaultMethodName(  ) throw (css::uno::RuntimeException);

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) = 0;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) = 0;

    virtual ::sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException);

    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) = 0;

};
#endif //SC_VBA_COLLECTION_IMPL_HXX
