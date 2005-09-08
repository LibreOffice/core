/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: groupaccess.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:13:42 $
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

#include "groupaccess.hxx"
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
OUString SAL_CALL BasicGroupAccess::getHierarchicalName(  ) throw(RuntimeException)
{
    return configapi::implGetHierarchicalName( getNode() );
}

//------------------------------------------------------------------------------------------------------------------
OUString SAL_CALL BasicGroupAccess::composeHierarchicalName( const OUString& sRelativeName )
    throw(css::lang::IllegalArgumentException, css::lang::NoSupportException, RuntimeException)
{
    return configapi::implComposeHierarchicalName( getNode(), sRelativeName );
}

//------------------------------------------------------------------------------------------------------------------

// XElementAccess, base class of XNameAccess (and XHierarchicalNameAccess ? )
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
uno::Type SAL_CALL BasicGroupAccess::getElementType(  ) throw(RuntimeException)
{
    return configapi::implGetElementType( getNode() );
}

//-----------------------------------------------------------------------------------
sal_Bool SAL_CALL BasicGroupAccess::hasElements(  ) throw(RuntimeException)
{
    return configapi::implHasElements( getNode() );
}

// XExactName
//-----------------------------------------------------------------------------------

OUString SAL_CALL BasicGroupAccess::getExactName( const OUString& rApproximateName ) throw(RuntimeException)
{
    return configapi::implGetExactName( getNode(), rApproximateName);
}

// XProperty
//-----------------------------------------------------------------------------------

css::beans::Property SAL_CALL BasicGroupAccess::getAsProperty(  ) throw(uno::RuntimeException)
{
    return configapi::implGetAsProperty( getNode() );
}

// XNameAccess
//-----------------------------------------------------------------------------------

sal_Bool SAL_CALL BasicGroupAccess::hasByName( const OUString& sName ) throw(RuntimeException)
{
    return configapi::implHasByName( getNode(), sName);
}

//-----------------------------------------------------------------------------------
Any SAL_CALL BasicGroupAccess::getByName( const OUString& sName )
    throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, RuntimeException)
{
    return configapi::implGetByName( getNode(), sName );
}

//-----------------------------------------------------------------------------------
Sequence< OUString > SAL_CALL BasicGroupAccess::getElementNames(  ) throw( RuntimeException)
{
    return configapi::implGetElementNames( getNode() );
}

// XHierarchicalNameAccess
//-----------------------------------------------------------------------------------
sal_Bool SAL_CALL BasicGroupAccess::hasByHierarchicalName( const OUString& sName ) throw(RuntimeException)
{
    return configapi::implHasByHierarchicalName( getNode(), sName);
}

//-----------------------------------------------------------------------------------
Any SAL_CALL BasicGroupAccess::getByHierarchicalName( const OUString& sName )
    throw(css::container::NoSuchElementException, RuntimeException)
{
    return configapi::implGetByHierarchicalName( getNode(), sName );
}


// XContainer
//-----------------------------------------------------------------------------------

void SAL_CALL BasicGroupAccess::addContainerListener( const Reference< css::container::XContainerListener >& xListener )
    throw(RuntimeException)
{
    configapi::implAddListener( getNode(), xListener );
}

//-----------------------------------------------------------------------------------
void SAL_CALL BasicGroupAccess::removeContainerListener( const Reference< css::container::XContainerListener >& xListener )
    throw(RuntimeException)
{
    configapi::implRemoveListener( getNode(), xListener );
}



//-----------------------------------------------------------------------------------

} // namespace configmgr


