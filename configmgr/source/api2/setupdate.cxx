/*************************************************************************
 *
 *  $RCSfile: setupdate.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "setupdate.hxx"

#ifndef CONFIGMGR_API_BASEACCESSIMPL_HXX_
#include "accessimpl.hxx"
#endif
#ifndef CONFIGMGR_API_BASEUPDATEIMPL_HXX_
#include "updateimpl.hxx"
#endif

#ifndef CONFIGMGR_API_NODEUPDATE_HXX_
#include "apinodeupdate.hxx"
#endif
#ifndef CONFIGMGR_API_APITYPES_HXX_
#include "apitypes.hxx"
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

namespace configmgr
{
//////////////////////////////////////////////////////////////////////////////////

    using uno::Reference;
    using uno::Sequence;
    using uno::Any;
    using uno::RuntimeException;

//////////////////////////////////////////////////////////////////////////////////
// classes BasicSet / BasicValueSet
//////////////////////////////////////////////////////////////////////////////////

// XInterface joining
//////////////////////////////////////////////////////////////////////////////////
uno::Any SAL_CALL BasicSet::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    Any aRet = BasicSetAccess::queryInterface( rType );
    if (!aRet.hasValue())
    {
        aRet = cppu::queryInterface(rType
                    , static_cast< css::container::XNameContainer *>(this)
                    , static_cast< css::container::XNameReplace *>(this)
                    , static_cast< css::lang::XSingleServiceFactory *>(this)
                    );
    }
    return aRet;
}
//..............................................................................

uno::Any SAL_CALL BasicValueSet::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    Any aRet = BasicSetAccess::queryInterface( rType );
    if (!aRet.hasValue())
    {
        aRet = cppu::queryInterface(rType
                    , static_cast< css::container::XNameContainer *>(this)
                    , static_cast< css::container::XNameReplace *>(this)
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
                OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: Invalid Object - internal update-interface missing.")),
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
                OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: Invalid Object - internal update-interface missing.")),
                static_cast< css::container::XNameReplace* >(this)
            );
    }
    return *pAccess;
}

// New Interface methods
// XNameReplace
//////////////////////////////////////////////////////////////////////////////////
void SAL_CALL BasicSet::replaceByName( const OUString& rName, const uno::Any& rElement )
        throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implReplaceByName( getSetNode(), rName, rElement );
}
//..............................................................................

void SAL_CALL BasicValueSet::replaceByName( const OUString& rName, const uno::Any& rElement )
        throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implReplaceByName( getSetNode(), rName, rElement );
}

// XNameContainer
//////////////////////////////////////////////////////////////////////////////////
void SAL_CALL BasicSet::insertByName( const OUString& rName, const uno::Any& rElement)
        throw(css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implInsertByName( getSetNode(), rName, rElement );
}
//..............................................................................

void SAL_CALL BasicValueSet::insertByName( const OUString& rName, const uno::Any& rElement)
        throw(css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implInsertByName( getSetNode(), rName, rElement );
}

//----------------------------------------------------------------------------------
void SAL_CALL BasicSet::removeByName( const OUString& rName )
        throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implRemoveByName( getSetNode(), rName );
}
//..............................................................................

void SAL_CALL BasicValueSet::removeByName( const OUString& rName )
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


