/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
 * $Revision$
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

#include "vbacontrols.hxx"
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <ooo/vba//XControlProvider.hpp>
#include <hash_map>

using namespace com::sun::star;
using namespace ooo::vba;


typedef ::cppu::WeakImplHelper2< container::XNameAccess, container::XIndexAccess > ArrayWrapImpl;

typedef  std::hash_map< rtl::OUString, sal_Int32, ::rtl::OUStringHash,
    ::std::equal_to< ::rtl::OUString >  > ControlIndexMap;
typedef  std::vector< uno::Reference< awt::XControl > > ControlVec;

class ControlArrayWrapper : public ArrayWrapImpl
{
    uno::Reference< awt::XControlContainer > mxDialog;
    uno::Sequence< ::rtl::OUString > msNames;
    ControlVec mControls;
    ControlIndexMap mIndices;

    rtl::OUString getControlName( const uno::Reference< awt::XControl >& xCtrl )
    {
        uno::Reference< beans::XPropertySet > xProp( xCtrl->getModel(), uno::UNO_QUERY );
        rtl::OUString sName;
        xProp->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ) ) >>= sName;
        return sName;
    }

public:

    ControlArrayWrapper( const uno::Reference< awt::XControl >& xDialog )
    {
        mxDialog.set( xDialog, uno::UNO_QUERY_THROW );
        uno::Sequence< uno::Reference< awt::XControl > > sXControls = mxDialog->getControls();

        msNames.realloc( sXControls.getLength() );
        for ( sal_Int32 i = 0; i < sXControls.getLength(); ++i )
        {
            uno::Reference< awt::XControl > xCtrl = sXControls[ i ];
            msNames[ i ] = getControlName( xCtrl );
            mControls.push_back( xCtrl );
            mIndices[ msNames[ i ] ] = i;
        }
    }

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
    {
        return awt::XControl::static_type(0);
    }

    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException)
    {
        return ( mControls.size() > 0 );
    }

    // XNameAcess
    virtual uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( !hasByName( aName ) )
            throw container::NoSuchElementException();
        return getByIndex( mIndices[ aName ] );
    }

    virtual uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException)
    {
        return msNames;
    }

    virtual ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (css::uno::RuntimeException)
    {
        ControlIndexMap::iterator it = mIndices.find( aName );
        return it != mIndices.end();
    }

    // XElementAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (css::uno::RuntimeException)
    {
        return mControls.size();
    }

    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
    {
        if ( Index < 0 || Index >= static_cast< sal_Int32 >( mControls.size() ) )
            throw lang::IndexOutOfBoundsException();
        return uno::makeAny( mControls[ Index ] );
    }
};


class ControlsEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference<XHelperInterface > m_xParent;
    uno::Reference<uno::XComponentContext > m_xContext;
    uno::Reference<container::XIndexAccess > m_xIndexAccess;
    uno::Reference<awt::XControl > m_xDlg;
    sal_Int32 nIndex;

public:

    ControlsEnumWrapper(  const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XIndexAccess >& xIndexAccess, const uno::Reference< awt::XControl >& xDlg ) :  m_xParent( xParent ), m_xContext( xContext), m_xIndexAccess( xIndexAccess ), m_xDlg( xDlg ), nIndex( 0 ) {}

    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return ( nIndex < m_xIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( nIndex < m_xIndexAccess->getCount() )
        {
            uno::Reference< frame::XModel > xModel;
            uno::Reference< awt::XControl > xControl;
            m_xIndexAccess->getByIndex( nIndex++ ) >>= xControl;

    uno::Reference<lang::XMultiComponentFactory > xServiceManager( m_xContext->getServiceManager(), uno::UNO_QUERY_THROW );
    uno::Reference< XControlProvider > xControlProvider( xServiceManager->createInstanceWithContext( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.ControlProvider" ) ), m_xContext ), uno::UNO_QUERY_THROW );

    uno::Reference< msforms::XControl > xVBAControl( xControlProvider->createUserformControl(  xControl, m_xDlg, xModel ) );
            return uno::makeAny( xVBAControl );
        }
        throw container::NoSuchElementException();
    }

};


uno::Reference<container::XIndexAccess >
lcl_controlsWrapper( const uno::Reference< awt::XControl >& xDlg )
{
    return new ControlArrayWrapper( xDlg );
}

ScVbaControls::ScVbaControls( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext,
                const css::uno::Reference< awt::XControl >& xDialog )
            : ControlsImpl_BASE( xParent, xContext, lcl_controlsWrapper( xDialog  ) )
{
    mxDialog.set( xDialog, uno::UNO_QUERY_THROW );
}

uno::Reference< container::XEnumeration >
ScVbaControls::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumeration > xEnum( new ControlsEnumWrapper( mxParent, mxContext, m_xIndexAccess, mxDialog ) );
    if ( !xEnum.is() )
        throw uno::RuntimeException();
    return xEnum;
}

uno::Any
ScVbaControls::createCollectionObject( const css::uno::Any& aSource )
{
    // Create control from awt::XControl
    uno::Reference< awt::XControl > xControl;
    aSource >>= xControl;
    uno::Reference< frame::XModel > xModel;
    uno::Reference<lang::XMultiComponentFactory > xServiceManager( mxContext->getServiceManager(), uno::UNO_QUERY_THROW );
    uno::Reference< XControlProvider > xControlProvider( xServiceManager->createInstanceWithContext( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ooo.vba.ControlProvider" ) ), mxContext ), uno::UNO_QUERY_THROW );

    uno::Reference< msforms::XControl > xVBAControl( xControlProvider->createUserformControl(  xControl, mxDialog, xModel ) );

    return uno::makeAny( xVBAControl );
}

void SAL_CALL
ScVbaControls::Move( double cx, double cy ) throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumeration > xEnum( createEnumeration() );
    while ( xEnum->hasMoreElements() )
    {
        uno::Reference< msforms::XControl > xControl( xEnum->nextElement(), uno::UNO_QUERY_THROW );
        xControl->setLeft( xControl->getLeft() + cx );
        xControl->setTop( xControl->getTop() + cy );
    }
}

uno::Type
ScVbaControls::getElementType() throw (uno::RuntimeException)
{
    return ooo::vba::msforms::XControl::static_type(0);
}
rtl::OUString&
ScVbaControls::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaControls") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaControls::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Controls" ) );
    }
    return aServiceNames;
}
