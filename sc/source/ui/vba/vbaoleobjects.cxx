/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbaoleobjects.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:56:50 $
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

#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <org/openoffice/excel/XOLEObject.hpp>

#include "vbaoleobject.hxx"
#include "vbaoleobjects.hxx"

using namespace com::sun::star;
using namespace org::openoffice;

typedef ::cppu::WeakImplHelper1< container::XIndexAccess > XIndexAccess_BASE;

class IndexAccessWrapper : public XIndexAccess_BASE
{
typedef std::vector< uno::Reference< drawing::XControlShape > > OLEObjects;
    OLEObjects vObjects;
public:
        IndexAccessWrapper(  const uno::Reference< container::XIndexAccess >& xIndexAccess )
    {
        sal_Int32 nLen = xIndexAccess->getCount();
        for ( sal_Int32 index = 0; index < nLen; ++index )
        {
                uno::Reference< drawing::XControlShape > xControlShape( xIndexAccess->getByIndex( index), uno::UNO_QUERY);
            if ( xControlShape.is() )
                vObjects.push_back( xControlShape );
        }
    }

    virtual ::sal_Int32 SAL_CALL getCount() throw (uno::RuntimeException)
    {
        return vObjects.size();
    }

    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( Index < 0 || Index >= getCount() )
            throw lang::IndexOutOfBoundsException();
        return uno::makeAny( vObjects[ Index ] );
    }

        // Methods XElementAcess
        virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException)
        {
            return drawing::XControlShape::static_type(0);
        }

        virtual ::sal_Bool SAL_CALL hasElements() throw (uno::RuntimeException)
        {
            return ( getCount() > 0 );
        }

};

class EnumWrapper : public EnumerationHelper_BASE
{

        uno::Reference<vba::XHelperInterface > m_xParent;
        uno::Reference<uno::XComponentContext > m_xContext;
        uno::Reference<container::XIndexAccess > m_xIndexAccess;
        sal_Int32 nIndex;
public:
        EnumWrapper(  const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, uno::Reference< container::XIndexAccess >& xIndexAccess ) :  m_xParent( xParent ), m_xContext( xContext), m_xIndexAccess( xIndexAccess ), nIndex( 0 ) {}

        virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
        {
                return ( nIndex < m_xIndexAccess->getCount() );
        }

        virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
        {
                if ( nIndex < m_xIndexAccess->getCount() )
        {
            uno::Reference< drawing::XControlShape > xControlShape (  m_xIndexAccess->getByIndex( nIndex++ ), uno::UNO_QUERY_THROW );
                return uno::makeAny( uno::Reference< oo::excel::XOLEObject >( new ScVbaOLEObject( m_xParent, m_xContext, xControlShape ) ) );
        }
                throw container::NoSuchElementException();
        }
};

uno::Reference< container::XIndexAccess > oleObjectIndexWrapper( const uno::Reference< container::XIndexAccess >& xIndexAccess )
{
    return new IndexAccessWrapper( xIndexAccess );
}

ScVbaOLEObjects::ScVbaOLEObjects( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext,
                const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess )
            : OLEObjectsImpl_BASE( xParent, xContext, oleObjectIndexWrapper( xIndexAccess  ) )
{
}
uno::Reference< container::XEnumeration >
ScVbaOLEObjects::createEnumeration() throw (uno::RuntimeException)
{
    return new EnumWrapper( getParent(), mxContext, m_xIndexAccess );
}

uno::Any
ScVbaOLEObjects::createCollectionObject( const css::uno::Any& aSource )
{
    if( aSource.hasValue() )
    {
        uno::Reference< drawing::XControlShape > xControlShape( aSource, uno::UNO_QUERY_THROW );
    // parent of OLEObject is the same parent as the collection ( e.g. the sheet )
        return uno::makeAny( uno::Reference< oo::excel::XOLEObject >( new ScVbaOLEObject( getParent(), mxContext, xControlShape ) ) );
    }
    return uno::Any();
}

uno::Any
ScVbaOLEObjects::getItemByStringIndex( const rtl::OUString& sIndex ) throw (uno::RuntimeException)
{
    try
    {
        return OLEObjectsImpl_BASE::getItemByStringIndex( sIndex );
    }
    catch( uno::RuntimeException )
    {
        uno::Reference< container::XIndexAccess > xIndexAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
        sal_Int32 nCount = xIndexAccess->getCount();
        for( int index = 0; index < nCount; index++ )
        {
            uno::Any aUnoObj =  xIndexAccess->getByIndex( index );
            uno::Reference< drawing::XControlShape > xControlShape( aUnoObj, uno::UNO_QUERY_THROW );
            uno::Reference< awt::XControlModel > xControlModel( xControlShape->getControl() );
            uno::Reference< container::XNamed > xNamed( xControlModel, uno::UNO_QUERY_THROW );
            if( sIndex.equals( xNamed->getName() ))
            {
                return createCollectionObject( aUnoObj );
            }

        }
        return uno::Any();
    }
}

uno::Type
ScVbaOLEObjects::getElementType() throw (uno::RuntimeException)
{
    return org::openoffice::excel::XOLEObject::static_type(0);
}
rtl::OUString&
ScVbaOLEObjects::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaOLEObjects") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaOLEObjects::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.OLEObjects" ) );
    }
    return aServiceNames;
}
