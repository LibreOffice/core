/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: groupupdate.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:14:46 $
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

#include "groupupdate.hxx"
#include "updateimpl.hxx"
#include "apinodeaccess.hxx"
#include "apitypes.hxx"

#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>

namespace configmgr
{
//////////////////////////////////////////////////////////////////////////////////

    using uno::Reference;
    using uno::Sequence;
    using uno::Any;
    using uno::RuntimeException;

//////////////////////////////////////////////////////////////////////////////////
// class BasicGroup
//////////////////////////////////////////////////////////////////////////////////

// XInterface joining
//////////////////////////////////////////////////////////////////////////////////
uno::Any SAL_CALL BasicGroup::queryInterface( uno::Type const& rType ) throw (uno::RuntimeException )
{
    Any aRet = BasicGroupAccess::queryInterface( rType );
    if (!aRet.hasValue())
    {
        aRet = cppu::queryInterface(rType
                    , static_cast< css::container::XNameReplace *>(this)
                    );
    }
    return aRet;
}

// XTypeProvider joining
//////////////////////////////////////////////////////////////////////////////////
uno::Sequence< uno::Type > SAL_CALL BasicGroup::getTypes( ) throw (uno::RuntimeException )
{
    /*static ?*/
    cppu::OTypeCollection aTypes(
        configapi::getReferenceType(static_cast< css::container::XNameReplace *>(this)),
        BasicGroupAccess::getTypes());

    return aTypes.getTypes();
}

//uno::Sequence< sal_Int8 > SAL_CALL BasicGroup::getImplementationId( ) throw (uno::RuntimeException ) = 0;

//////////////////////////////////////////////////////////////////////////////////

// safe write access
/////////////////////////////////////////////////////////////
configapi::NodeGroupAccess& BasicGroup::getGroupNode()
{
    configapi::NodeGroupAccess* pAccess = maybeGetUpdateAccess();
    OSL_ENSURE(pAccess, "Write operation invoked on a read-only node access - failing with RuntimeException");

    if (!pAccess)
    {
        throw uno::RuntimeException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Configuration: Invalid Object - internal update interface missing.")),
                static_cast< css::container::XNameReplace * >(this)
            );
    }
    return *pAccess;
}

// New Interface methods
// XNameReplace
//////////////////////////////////////////////////////////////////////////////////
void SAL_CALL BasicGroup::replaceByName( const OUString& rName, const uno::Any& rElement )
        throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException)
{
    configapi::implReplaceByName( getGroupNode(), rName, rElement );
}

//-----------------------------------------------------------------------------------
} // namespace configmgr


