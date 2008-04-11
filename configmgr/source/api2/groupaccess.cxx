/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: groupaccess.cxx,v $
 * $Revision: 1.5 $
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


