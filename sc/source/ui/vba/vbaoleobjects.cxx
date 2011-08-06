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

#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <ooo/vba/excel/XOLEObject.hpp>

#include "vbaoleobject.hxx"
#include "vbaoleobjects.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

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

        uno::Reference<XHelperInterface > m_xParent;
        uno::Reference<uno::XComponentContext > m_xContext;
        uno::Reference<container::XIndexAccess > m_xIndexAccess;
        sal_Int32 nIndex;
public:
        EnumWrapper(  const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, uno::Reference< container::XIndexAccess >& xIndexAccess ) :  m_xParent( xParent ), m_xContext( xContext), m_xIndexAccess( xIndexAccess ), nIndex( 0 ) {}

        virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
        {
                return ( nIndex < m_xIndexAccess->getCount() );
        }

        virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
        {
                if ( nIndex < m_xIndexAccess->getCount() )
        {
            uno::Reference< drawing::XControlShape > xControlShape (  m_xIndexAccess->getByIndex( nIndex++ ), uno::UNO_QUERY_THROW );
                return uno::makeAny( uno::Reference< ov::excel::XOLEObject >( new ScVbaOLEObject( m_xParent, m_xContext, xControlShape ) ) );
        }
                throw container::NoSuchElementException();
        }
};

uno::Reference< container::XIndexAccess > oleObjectIndexWrapper( const uno::Reference< container::XIndexAccess >& xIndexAccess )
{
    return new IndexAccessWrapper( xIndexAccess );
}

ScVbaOLEObjects::ScVbaOLEObjects( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext,
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
        return uno::makeAny( uno::Reference< ov::excel::XOLEObject >( new ScVbaOLEObject( getParent(), mxContext, xControlShape ) ) );
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
    catch (const uno::RuntimeException&)
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
    return ooo::vba::excel::XOLEObject::static_type(0);
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
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.OLEObjects" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
