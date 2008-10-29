/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: setupdate.cxx,v $
 * $Revision: 1.7 $
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
#include "precompiled_configmgr.hxx"

#include "setupdate.hxx"
#include "accessimpl.hxx"
#include "updateimpl.hxx"
#include "apinodeupdate.hxx"
#include "apitypes.hxx"
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>

namespace configmgr
{
//////////////////////////////////////////////////////////////////////////////////
// classes BasicSet / BasicValueSet
//////////////////////////////////////////////////////////////////////////////////

// XInterface joining
//////////////////////////////////////////////////////////////////////////////////
uno::Any SAL_CALL BasicSet::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    uno::Any aRet = BasicSetAccess::queryInterface( rType );
    if (!aRet.hasValue())
    {
        aRet = cppu::queryInterface(rType
                    , static_cast< css::container::XNameContainer *>(this)
                    , static_cast< css::container::XNameReplace *>(this)
                    , static_cast< css::beans::XPropertyWithState *>(this)
                    , static_cast< css::lang::XSingleServiceFactory *>(this)
                    );
    }
    return aRet;
}
//..............................................................................

uno::Any SAL_CALL BasicValueSet::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    uno::Any aRet = BasicSetAccess::queryInterface( rType );
    if (!aRet.hasValue())
    {
        aRet = cppu::queryInterface(rType
                    , static_cast< css::container::XNameContainer *>(this)
                    , static_cast< css::container::XNameReplace *>(this)
                    , static_cast< css::beans::XPropertyWithState *>(this)
                    );
    }
    return aRet;
}

// XTypeProvider joining
//////////////////////////////////////////////////////////////////////////////////
uno::Sequence< uno::Type > SAL_CALL BasicSet::getTypes( ) throw (uno::RuntimeException )
{
    /*static ?*/
    cppu::OTypeCollection aTypes(
        configapi::getReferenceType(static_cast< css::container::XNameContainer *>(this)),
        configapi::getReferenceType(static_cast< css::container::XNameReplace *>(this)),
        configapi::getReferenceType(static_cast< css::beans::XPropertyWithState *>(this)),
        configapi::getReferenceType(static_cast< css::lang::XSingleServiceFactory *>(this)),
        BasicSetAccess::getTypes());

    return aTypes.getTypes();
}
//..............................................................................

uno::Sequence< uno::Type > SAL_CALL BasicValueSet::getTypes( ) throw (uno::RuntimeException )
{
    /*static ?*/
    cppu::OTypeCollection aTypes(
        configapi::getReferenceType(static_cast< css::container::XNameContainer *>(this)),
        configapi::getReferenceType(static_cast< css::container::XNameReplace *>(this)),
        configapi::getReferenceType(static_cast< css::beans::XPropertyWithState *>(this)),
        BasicSetAccess::getTypes());

    return aTypes.getTypes();
}

//uno::Sequence< sal_Int8 > SAL_CALL BasicSet::getImplementationId( ) throw (uno::RuntimeException ) = 0;
//uno::Sequence< sal_Int8 > SAL_CALL BasicValueSet::getImplementationId( ) throw (uno::RuntimeException ) = 0;

// safe write access
//////////////////////////////////////////////////////////////////////////////////
configapi::NodeTreeSetAccess& BasicSet::getSetNode()
{
    configapi::NodeTreeSetAccess* pAccess = maybeGetUpdateAccess();
    OSL_ENSURE(pAccess, "Write operation invoked on a read-only node access - failing with RuntimeException");

    if (!pAccess)
    {
        throw uno::RuntimeException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: Invalid Object - internal update-interface missing.")),
                static_cast< css::container::XNameReplace * >(this)
            );
    }
    return *pAccess;
}

configapi::NodeValueSetAccess& BasicValueSet::getSetNode()
{
    configapi::NodeValueSetAccess* pAccess = maybeGetUpdateAccess();
    OSL_ENSURE(pAccess, "Write operation invoked on a read-only node access - failing with RuntimeException");

    if (!pAccess)
    {
        throw uno::RuntimeException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: Invalid Object - internal update-interface missing.")),
                static_cast< css::container::XNameReplace* >(this)
            );
    }
    return *pAccess;
}

// New Interface methods
// XNameReplace
//////////////////////////////////////////////////////////////////////////////////
void SAL_CALL BasicSet::replaceByName( const rtl::OUString& rName, const uno::Any& rElement )
        throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implReplaceByName( getSetNode(), rName, rElement );
}
//..............................................................................

void SAL_CALL BasicValueSet::replaceByName( const rtl::OUString& rName, const uno::Any& rElement )
        throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implReplaceByName( getSetNode(), rName, rElement );
}

// XNameContainer
//////////////////////////////////////////////////////////////////////////////////
void SAL_CALL BasicSet::insertByName( const rtl::OUString& rName, const uno::Any& rElement)
        throw(css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implInsertByName( getSetNode(), rName, rElement );
}
//..............................................................................

void SAL_CALL BasicValueSet::insertByName( const rtl::OUString& rName, const uno::Any& rElement)
        throw(css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implInsertByName( getSetNode(), rName, rElement );
}

//----------------------------------------------------------------------------------
void SAL_CALL BasicSet::removeByName( const rtl::OUString& rName )
        throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implRemoveByName( getSetNode(), rName );
}
//..............................................................................

void SAL_CALL BasicValueSet::removeByName( const rtl::OUString& rName )
        throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implRemoveByName( getSetNode(), rName );
}

// XPropertyWithState
//////////////////////////////////////////////////////////////////////////////////

css::beans::PropertyState SAL_CALL BasicSet::getStateAsProperty() throw (uno::RuntimeException)
{
    return configapi::implGetStateAsProperty( getSetNode() );
}
//..............................................................................

css::beans::PropertyState SAL_CALL BasicValueSet::getStateAsProperty() throw (uno::RuntimeException)
{
    return configapi::implGetStateAsProperty( getSetNode() );
}
//-----------------------------------------------------------------------------------

void SAL_CALL BasicSet::setToDefaultAsProperty() throw (css::lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implSetToDefaultAsProperty( getSetNode() );
}
//..............................................................................

void SAL_CALL BasicValueSet::setToDefaultAsProperty() throw (css::lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implSetToDefaultAsProperty( getSetNode() );
}
//-----------------------------------------------------------------------------------

uno::Reference< uno::XInterface > SAL_CALL BasicSet::getDefaultAsProperty() throw (css::lang::WrappedTargetException, uno::RuntimeException)
{
    return configapi::implGetDefaultAsProperty( getSetNode() );
}
//..............................................................................

uno::Reference< uno::XInterface > SAL_CALL BasicValueSet::getDefaultAsProperty() throw (css::lang::WrappedTargetException, uno::RuntimeException)
{
    return configapi::implGetDefaultAsProperty( getSetNode() );
}

// XSingleServiceFactory (not for ValueSet)
//////////////////////////////////////////////////////////////////////////////////
uno::Reference< uno::XInterface > SAL_CALL BasicSet::createInstance(  )
        throw(uno::Exception, uno::RuntimeException)
{
    return configapi::implCreateElement( getSetNode() );
}

//----------------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL BasicSet::createInstanceWithArguments( const uno::Sequence< uno::Any >& aArguments )
        throw(uno::Exception, uno::RuntimeException)
{
    return configapi::implCreateElement( getSetNode(), aArguments );
}

//-----------------------------------------------------------------------------------
} // namespace configmgr


