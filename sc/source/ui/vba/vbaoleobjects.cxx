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

#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <ooo/vba/excel/XOLEObject.hpp>

#include "vbaoleobject.hxx"
#include "vbaoleobjects.hxx"
#include <cppuhelper/implbase.hxx>

using namespace com::sun::star;
using namespace ooo::vba;

typedef ::cppu::WeakImplHelper< container::XIndexAccess > XIndexAccess_BASE;

namespace {

class IndexAccessWrapper : public XIndexAccess_BASE
{
typedef std::vector< uno::Reference< drawing::XControlShape > > OLEObjects;
    OLEObjects vObjects;
public:
    explicit IndexAccessWrapper(  const uno::Reference< container::XIndexAccess >& xIndexAccess )
    {
        sal_Int32 nLen = xIndexAccess->getCount();
        for ( sal_Int32 index = 0; index < nLen; ++index )
        {
            uno::Reference< drawing::XControlShape > xControlShape( xIndexAccess->getByIndex( index), uno::UNO_QUERY);
            if ( xControlShape.is() )
                vObjects.push_back( xControlShape );
        }
    }

    virtual ::sal_Int32 SAL_CALL getCount() override
    {
        return vObjects.size();
    }

    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override
    {
        if ( Index < 0 || Index >= getCount() )
            throw lang::IndexOutOfBoundsException();
        return uno::makeAny( vObjects[ Index ] );
    }

        // Methods XElementAccess
        virtual uno::Type SAL_CALL getElementType() override
        {
            return cppu::UnoType<drawing::XControlShape>::get();
        }

        virtual sal_Bool SAL_CALL hasElements() override
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
        EnumWrapper( const uno::Reference< XHelperInterface >& xParent,
                     const uno::Reference< uno::XComponentContext >& xContext,
                     const uno::Reference< container::XIndexAccess >& xIndexAccess )
            :  m_xParent( xParent ), m_xContext( xContext), m_xIndexAccess( xIndexAccess ), nIndex( 0 ) {}

        virtual sal_Bool SAL_CALL hasMoreElements(  ) override
        {
                return ( nIndex < m_xIndexAccess->getCount() );
        }

        virtual uno::Any SAL_CALL nextElement(  ) override
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

}

ScVbaOLEObjects::ScVbaOLEObjects( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext,
                const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess )
            : OLEObjectsImpl_BASE( xParent, xContext, oleObjectIndexWrapper( xIndexAccess  ) )
{
}
uno::Reference< container::XEnumeration >
ScVbaOLEObjects::createEnumeration()
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
ScVbaOLEObjects::getItemByStringIndex( const OUString& sIndex )
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
            if( sIndex == xNamed->getName() )
            {
                return createCollectionObject( aUnoObj );
            }

        }
        return uno::Any();
    }
}

uno::Type
ScVbaOLEObjects::getElementType()
{
    return cppu::UnoType<ooo::vba::excel::XOLEObject>::get();
}

OUString
ScVbaOLEObjects::getServiceImplName()
{
    return OUString("ScVbaOLEObjects");
}

uno::Sequence< OUString >
ScVbaOLEObjects::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.excel.OLEObjects"
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
