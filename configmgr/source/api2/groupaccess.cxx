/*************************************************************************
 *
 *  $RCSfile: groupaccess.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-07 14:34:32 $
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

#include "groupaccess.hxx"
#include "accessimpl.hxx"
#include "apinotifierimpl.hxx"
#include "apinodeaccess.hxx"

namespace configmgr
{
    using namespace configapi;

    using uno::Reference;
    using uno::Sequence;
    using uno::Any;
    using uno::RuntimeException;

// XHierarchicalName
//------------------------------------------------------------------------------------------------------------------
OUString SAL_CALL BasicGroupAccess::getHierarchicalName(  ) throw(RuntimeException)
{
    return implGetHierarchicalName( getNode() );
}

//------------------------------------------------------------------------------------------------------------------
OUString SAL_CALL BasicGroupAccess::composeHierarchicalName( const OUString& sRelativeName )
    throw(css::lang::IllegalArgumentException, css::lang::NoSupportException, RuntimeException)
{
    return implComposeHierarchicalName( getNode(), sRelativeName );
}

//------------------------------------------------------------------------------------------------------------------

// XElementAccess, base class of XNameAccess (and XHierarchicalNameAccess ? )
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
uno::Type SAL_CALL BasicGroupAccess::getElementType(  ) throw(RuntimeException)
{
    return implGetElementType( getNode() );
}

//-----------------------------------------------------------------------------------
sal_Bool SAL_CALL BasicGroupAccess::hasElements(  ) throw(RuntimeException)
{
    return implHasElements( getNode() );
}

// XExactName
//-----------------------------------------------------------------------------------

OUString SAL_CALL BasicGroupAccess::getExactName( const OUString& rApproximateName ) throw(RuntimeException)
{
    return implGetExactName( getNode(), rApproximateName);
}

// XProperty
//-----------------------------------------------------------------------------------

css::beans::Property SAL_CALL BasicGroupAccess::getAsProperty(  ) throw(uno::RuntimeException)
{
    return implGetAsProperty( getNode() );
}

// XNameAccess
//-----------------------------------------------------------------------------------

sal_Bool SAL_CALL BasicGroupAccess::hasByName( const OUString& sName ) throw(RuntimeException)
{
    return implHasByName( getNode(), sName);
}

//-----------------------------------------------------------------------------------
Any SAL_CALL BasicGroupAccess::getByName( const OUString& sName )
    throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, RuntimeException)
{
    return implGetByName( getNode(), sName );
}

//-----------------------------------------------------------------------------------
Sequence< OUString > SAL_CALL BasicGroupAccess::getElementNames(  ) throw( RuntimeException)
{
    return implGetElementNames( getNode() );
}

// XHierarchicalNameAccess
//-----------------------------------------------------------------------------------
sal_Bool SAL_CALL BasicGroupAccess::hasByHierarchicalName( const OUString& sName ) throw(RuntimeException)
{
    return implHasByHierarchicalName( getNode(), sName);
}

//-----------------------------------------------------------------------------------
Any SAL_CALL BasicGroupAccess::getByHierarchicalName( const OUString& sName )
    throw(css::container::NoSuchElementException, RuntimeException)
{
    return implGetByHierarchicalName( getNode(), sName );
}


// XContainer
//-----------------------------------------------------------------------------------

void SAL_CALL BasicGroupAccess::addContainerListener( const Reference< css::container::XContainerListener >& xListener )
    throw(RuntimeException)
{
    implAddListener( getNode(), xListener );
}

//-----------------------------------------------------------------------------------
void SAL_CALL BasicGroupAccess::removeContainerListener( const Reference< css::container::XContainerListener >& xListener )
    throw(RuntimeException)
{
    implRemoveListener( getNode(), xListener );
}



//-----------------------------------------------------------------------------------

} // namespace configmgr


