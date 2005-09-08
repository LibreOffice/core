/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: setaccess.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:20:02 $
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

#include "setaccess.hxx"
#include "accessimpl.hxx"
#include "apinotifierimpl.hxx"
#include "apinodeaccess.hxx"

namespace configmgr
{

    using uno::Reference;
    using uno::Sequence;
    using uno::Any;
    using uno::RuntimeException;

// XHierarchicalName
//------------------------------------------------------------------------------------------------------------------
OUString SAL_CALL BasicSetAccess::getHierarchicalName(  ) throw(RuntimeException)
{
    return configapi::implGetHierarchicalName( getNode() );
}

//------------------------------------------------------------------------------------------------------------------
OUString SAL_CALL BasicSetAccess::composeHierarchicalName( const OUString& sRelativeName )
    throw(css::lang::IllegalArgumentException, css::lang::NoSupportException, RuntimeException)
{
    return configapi::implComposeHierarchicalName( getNode(), sRelativeName );
}

//------------------------------------------------------------------------------------------------------------------

// XElementAccess, base class of XNameAccess (and XHierarchicalNameAccess ? )
//-----------------------------------------------------------------------------------

uno::Type SAL_CALL BasicSetAccess::getElementType(  ) throw(RuntimeException)
{
    return configapi::implGetElementType( getNode() );
}

//-----------------------------------------------------------------------------------
sal_Bool SAL_CALL BasicSetAccess::hasElements(  ) throw(RuntimeException)
{
    return configapi::implHasElements( getNode() );
}

// XExactName
//-----------------------------------------------------------------------------------

OUString SAL_CALL BasicSetAccess::getExactName( const OUString& rApproximateName ) throw(RuntimeException)
{
    return configapi::implGetExactName( getNode(), rApproximateName);
}

// XProperty
//-----------------------------------------------------------------------------------

css::beans::Property SAL_CALL BasicSetAccess::getAsProperty(  ) throw(RuntimeException)
{
    return configapi::implGetAsProperty( getNode() );
}

// XPropertySetInfo
//-----------------------------------------------------------------------------------

Sequence< css::beans::Property > SAL_CALL BasicSetAccess::getProperties(  ) throw (uno::RuntimeException)
{
    return configapi::implGetProperties( getNode() );
}

css::beans::Property SAL_CALL BasicSetAccess::getPropertyByName( const OUString& aName )
    throw (css::beans::UnknownPropertyException, RuntimeException)
{
    return configapi::implGetPropertyByName( getNode(), aName );
}

sal_Bool SAL_CALL BasicSetAccess::hasPropertyByName( const OUString& Name ) throw (RuntimeException)
{
    return configapi::implHasPropertyByName( getNode(), Name );
}


// XNameAccess
//-----------------------------------------------------------------------------------

sal_Bool SAL_CALL BasicSetAccess::hasByName( const OUString& sName ) throw(RuntimeException)
{
    return configapi::implHasByName( getNode(), sName);
}

//-----------------------------------------------------------------------------------
Any SAL_CALL BasicSetAccess::getByName( const OUString& sName )
    throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, RuntimeException)
{
    return configapi::implGetByName( getNode(), sName );
}

//-----------------------------------------------------------------------------------
Sequence< OUString > SAL_CALL BasicSetAccess::getElementNames(  ) throw( RuntimeException)
{
    return configapi::implGetElementNames( getNode() );
}

// XHierarchicalNameAccess
//-----------------------------------------------------------------------------------
sal_Bool SAL_CALL BasicSetAccess::hasByHierarchicalName( const OUString& sName ) throw(RuntimeException)
{
    return configapi::implHasByHierarchicalName( getNode(), sName);
}

//-----------------------------------------------------------------------------------
Any SAL_CALL BasicSetAccess::getByHierarchicalName( const OUString& sName )
    throw(css::container::NoSuchElementException, RuntimeException)
{
    return configapi::implGetByHierarchicalName( getNode(), sName );
}


// XContainer
//-----------------------------------------------------------------------------------

void SAL_CALL BasicSetAccess::addContainerListener( const Reference< css::container::XContainerListener >& xListener )
    throw(RuntimeException)
{
    configapi::implAddListener( getNode(), xListener );
}

void SAL_CALL BasicSetAccess::removeContainerListener( const Reference< css::container::XContainerListener >& xListener )
    throw(RuntimeException)
{
    configapi::implRemoveListener( getNode(), xListener );
}

//-----------------------------------------------------------------------------------
// Set-specific interfaces
//-----------------------------------------------------------------------------------

// XTemplateContainer
//-----------------------------------------------------------------------------------
OUString SAL_CALL BasicSetAccess::getElementTemplateName(  )
    throw(uno::RuntimeException)
{
    return configapi::implGetElementTemplateName( getNode() );
}

// XStringEscape
//-----------------------------------------------------------------------------------
OUString SAL_CALL BasicSetAccess::escapeString( const OUString& aString )
    throw(css::lang::IllegalArgumentException, RuntimeException)
{
    return aString;
}

OUString SAL_CALL BasicSetAccess::unescapeString( const OUString& aEscapedString )
    throw(css::lang::IllegalArgumentException, RuntimeException)
{
    return aEscapedString;
}

//-----------------------------------------------------------------------------------
} // namespace configmgr


