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
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_scripting.hxx"

#include "DialogModelProvider.hxx"
#include "dlgprov.hxx"
#include <com/sun/star/resource/XStringResourceManager.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>


// component helper namespace
namespace comp_DialogModelProvider {

namespace css = ::com::sun::star;
using namespace ::com::sun::star;
using namespace awt;
using namespace lang;
using namespace uno;
using namespace script;
using namespace beans;


// component and service helper functions:
::rtl::OUString SAL_CALL _getImplementationName();
css::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames();
css::uno::Reference< css::uno::XInterface > SAL_CALL _create( css::uno::Reference< css::uno::XComponentContext > const & context );

} // closing component helper namespace



/// anonymous implementation namespace
namespace dlgprov {

namespace css = ::com::sun::star;
using namespace ::com::sun::star;
using namespace awt;
using namespace lang;
using namespace uno;
using namespace script;
using namespace beans;


DialogModelProvider::DialogModelProvider(Reference< XComponentContext > const & context) :
    m_xContext(context)
{}

// lang::XInitialization:
void SAL_CALL DialogModelProvider::initialize(const css::uno::Sequence< uno::Any > & aArguments) throw (css::uno::RuntimeException, css::uno::Exception)
{
    if ( aArguments.getLength() == 1 )
    {
        ::rtl::OUString sURL;
        if ( !( aArguments[ 0 ] >>= sURL ))
            throw css::lang::IllegalArgumentException();
         // Try any other URL with SimpleFileAccess
        Reference< XMultiComponentFactory > xSMgr( m_xContext->getServiceManager(), UNO_QUERY_THROW );
        Reference< ucb::XSimpleFileAccess > xSFI =
            Reference< ucb::XSimpleFileAccess >( xSMgr->createInstanceWithContext
            ( ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ), m_xContext ), UNO_QUERY );

        try
        {
            Reference< io::XInputStream > xInput = xSFI->openFileRead( sURL );
            Reference< resource::XStringResourceManager > xStringResourceManager;
            if ( xInput.is() )
            {
                xStringResourceManager = dlgprov::lcl_getStringResourceManager(m_xContext,sURL);
                Any aDialogSourceURLAny;
                aDialogSourceURLAny <<= sURL;

                m_xDialogModel.set( dlgprov::lcl_createDialogModel( m_xContext,xInput , xStringResourceManager, aDialogSourceURLAny  ), UNO_QUERY_THROW);
                m_xDialogModelProp.set(m_xDialogModel, UNO_QUERY_THROW);
            }
        }
        catch( Exception& )
        {}
        //m_sURL = sURL;
    }
}

// container::XElementAccess:
uno::Type SAL_CALL DialogModelProvider::getElementType() throw (css::uno::RuntimeException)
{
    return m_xDialogModel->getElementType();
}

::sal_Bool SAL_CALL DialogModelProvider::hasElements() throw (css::uno::RuntimeException)
{
    return m_xDialogModel->hasElements();
}

// container::XNameAccess:
uno::Any SAL_CALL DialogModelProvider::getByName(const ::rtl::OUString & aName) throw (css::uno::RuntimeException, css::container::NoSuchElementException, css::lang::WrappedTargetException)
{
    return m_xDialogModel->getByName(aName);
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL DialogModelProvider::getElementNames() throw (css::uno::RuntimeException)
{
    return m_xDialogModel->getElementNames();
}

::sal_Bool SAL_CALL DialogModelProvider::hasByName(const ::rtl::OUString & aName) throw (css::uno::RuntimeException)
{
    return m_xDialogModel->hasByName(aName);
}

// container::XNameReplace:
void SAL_CALL DialogModelProvider::replaceByName(const ::rtl::OUString & aName, const uno::Any & aElement) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException)
{
    m_xDialogModel->replaceByName(aName,aElement);
}

// container::XNameContainer:
void SAL_CALL DialogModelProvider::insertByName(const ::rtl::OUString & aName, const uno::Any & aElement) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException)
{
    m_xDialogModel->insertByName(aName,aElement);
}

void SAL_CALL DialogModelProvider::removeByName(const ::rtl::OUString & aName) throw (css::uno::RuntimeException, css::container::NoSuchElementException, css::lang::WrappedTargetException)
{
    m_xDialogModel->removeByName(aName);
}
uno::Reference< beans::XPropertySetInfo > SAL_CALL DialogModelProvider::getPropertySetInfo(  ) throw (uno::RuntimeException)
{
    return m_xDialogModelProp->getPropertySetInfo();
}
void SAL_CALL DialogModelProvider::setPropertyValue( const ::rtl::OUString&, const uno::Any& ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
}
uno::Any SAL_CALL DialogModelProvider::getPropertyValue( const ::rtl::OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return m_xDialogModelProp->getPropertyValue(PropertyName);
}
void SAL_CALL DialogModelProvider::addPropertyChangeListener( const ::rtl::OUString& , const uno::Reference< beans::XPropertyChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL DialogModelProvider::removePropertyChangeListener( const ::rtl::OUString& , const uno::Reference< beans::XPropertyChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL DialogModelProvider::addVetoableChangeListener( const ::rtl::OUString& , const uno::Reference< beans::XVetoableChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}
void SAL_CALL DialogModelProvider::removeVetoableChangeListener( const ::rtl::OUString& ,const uno::Reference< beans::XVetoableChangeListener >& ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

// com.sun.star.uno.XServiceInfo:
::rtl::OUString SAL_CALL DialogModelProvider::getImplementationName() throw (css::uno::RuntimeException)
{
    return comp_DialogModelProvider::_getImplementationName();
}

::sal_Bool SAL_CALL DialogModelProvider::supportsService(::rtl::OUString const & serviceName) throw (css::uno::RuntimeException)
{
    css::uno::Sequence< ::rtl::OUString > serviceNames = comp_DialogModelProvider::_getSupportedServiceNames();
    for (::sal_Int32 i = 0; i < serviceNames.getLength(); ++i) {
        if (serviceNames[i] == serviceName)
            return sal_True;
    }
    return sal_False;
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL DialogModelProvider::getSupportedServiceNames() throw (css::uno::RuntimeException)
{
    return comp_DialogModelProvider::_getSupportedServiceNames();
}

} // closing anonymous implementation namespace

