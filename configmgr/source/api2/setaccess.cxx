/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: setaccess.cxx,v $
 * $Revision: 1.6 $
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

#include "setaccess.hxx"
#include "accessimpl.hxx"
#include "apinotifierimpl.hxx"
#include "apinodeaccess.hxx"

namespace configmgr
{

// XHierarchicalName
//------------------------------------------------------------------------------------------------------------------
rtl::OUString SAL_CALL BasicSetAccess::getHierarchicalName(  ) throw(uno::RuntimeException)
{
    return configapi::implGetHierarchicalName( getNode() );
}

//------------------------------------------------------------------------------------------------------------------
rtl::OUString SAL_CALL BasicSetAccess::composeHierarchicalName( const rtl::OUString& sRelativeName )
    throw(css::lang::IllegalArgumentException, css::lang::NoSupportException, uno::RuntimeException)
{
    return configapi::implComposeHierarchicalName( getNode(), sRelativeName );
}

//------------------------------------------------------------------------------------------------------------------

// XElementAccess, base class of XNameAccess (and XHierarchicalNameAccess ? )
//-----------------------------------------------------------------------------------

uno::Type SAL_CALL BasicSetAccess::getElementType(  ) throw(uno::RuntimeException)
{
    return configapi::implGetElementType( getNode() );
}

//-----------------------------------------------------------------------------------
sal_Bool SAL_CALL BasicSetAccess::hasElements(  ) throw(uno::RuntimeException)
{
    return configapi::implHasElements( getNode() );
}

// XExactName
//-----------------------------------------------------------------------------------

rtl::OUString SAL_CALL BasicSetAccess::getExactName( const rtl::OUString& rApproximateName ) throw(uno::RuntimeException)
{
    return configapi::implGetExactName( getNode(), rApproximateName);
}

// XProperty
//-----------------------------------------------------------------------------------

css::beans::Property SAL_CALL BasicSetAccess::getAsProperty(  ) throw(uno::RuntimeException)
{
    return configapi::implGetAsProperty( getNode() );
}

// XPropertySetInfo
//-----------------------------------------------------------------------------------

uno::Sequence< css::beans::Property > SAL_CALL BasicSetAccess::getProperties(  ) throw (uno::RuntimeException)
{
    return configapi::implGetProperties( getNode() );
}

css::beans::Property SAL_CALL BasicSetAccess::getPropertyByName( const rtl::OUString& aName )
    throw (css::beans::UnknownPropertyException, uno::RuntimeException)
{
    return configapi::implGetPropertyByName( getNode(), aName );
}

sal_Bool SAL_CALL BasicSetAccess::hasPropertyByName( const rtl::OUString& name ) throw (uno::RuntimeException)
{
    return configapi::implHasPropertyByName( getNode(), name );
}


// XNameAccess
//-----------------------------------------------------------------------------------

sal_Bool SAL_CALL BasicSetAccess::hasByName( const rtl::OUString& sName ) throw(uno::RuntimeException)
{
    return configapi::implHasByName( getNode(), sName);
}

//-----------------------------------------------------------------------------------
uno::Any SAL_CALL BasicSetAccess::getByName( const rtl::OUString& sName )
    throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException)
{
    return configapi::implGetByName( getNode(), sName );
}

//-----------------------------------------------------------------------------------
uno::Sequence< rtl::OUString > SAL_CALL BasicSetAccess::getElementNames(  ) throw( uno::RuntimeException)
{
    return configapi::implGetElementNames( getNode() );
}

// XHierarchicalNameAccess
//-----------------------------------------------------------------------------------
sal_Bool SAL_CALL BasicSetAccess::hasByHierarchicalName( const rtl::OUString& sName ) throw(uno::RuntimeException)
{
    return configapi::implHasByHierarchicalName( getNode(), sName);
}

//-----------------------------------------------------------------------------------
uno::Any SAL_CALL BasicSetAccess::getByHierarchicalName( const rtl::OUString& sName )
    throw(css::container::NoSuchElementException, uno::RuntimeException)
{
    return configapi::implGetByHierarchicalName( getNode(), sName );
}


// XContainer
//-----------------------------------------------------------------------------------

void SAL_CALL BasicSetAccess::addContainerListener( const uno::Reference< css::container::XContainerListener >& xListener )
    throw(uno::RuntimeException)
{
    configapi::implAddListener( getNode(), xListener );
}

void SAL_CALL BasicSetAccess::removeContainerListener( const uno::Reference< css::container::XContainerListener >& xListener )
    throw(uno::RuntimeException)
{
    configapi::implRemoveListener( getNode(), xListener );
}

//-----------------------------------------------------------------------------------
// Set-specific interfaces
//-----------------------------------------------------------------------------------

// XTemplateContainer
//-----------------------------------------------------------------------------------
rtl::OUString SAL_CALL BasicSetAccess::getElementTemplateName(  )
    throw(uno::RuntimeException)
{
    return configapi::implGetElementTemplateName( getNode() );
}

// XStringEscape
//-----------------------------------------------------------------------------------
rtl::OUString SAL_CALL BasicSetAccess::escapeString( const rtl::OUString& aString )
    throw(css::lang::IllegalArgumentException, uno::RuntimeException)
{
    return aString;
}

rtl::OUString SAL_CALL BasicSetAccess::unescapeString( const rtl::OUString& aEscapedString )
    throw(css::lang::IllegalArgumentException, uno::RuntimeException)
{
    return aEscapedString;
}

//-----------------------------------------------------------------------------------
} // namespace configmgr


