/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: groupupdate.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:15:00 $
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

#ifndef CONFIGMGR_API_GROUPUPDATE_HXX_
#define CONFIGMGR_API_GROUPUPDATE_HXX_

#include "groupaccess.hxx"

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif

//........................................................................
namespace configmgr
{
//........................................................................
    namespace configapi { class NodeGroupAccess; }

//==========================================================================
//= BasicGroup
//==========================================================================

/** base class for configuration nodes which are dynamic sets of complex types (trees)
*/
    class BasicGroup
    : public BasicGroupAccess
    , public css::container::XNameReplace
    {
    protected:
    // Destructors
        virtual ~BasicGroup() {}

    public:
    // Base class Interface methods
        // XInterface joining
        uno::Any SAL_CALL queryInterface( uno::Type const& rType ) throw (uno::RuntimeException );

        // XTypeProvider joining
        uno::Sequence< uno::Type > SAL_CALL getTypes( ) throw (uno::RuntimeException );
        uno::Sequence< sal_Int8 > SAL_CALL getImplementationId( ) throw (uno::RuntimeException ) = 0;

        // XElementAccess forwarding
        virtual uno::Type SAL_CALL getElementType(  ) throw(uno::RuntimeException)
        { return BasicGroupAccess::getElementType(); }

        virtual sal_Bool SAL_CALL hasElements(  )  throw(uno::RuntimeException)
        { return BasicGroupAccess::hasElements(); }

        // XNameAccess forwarding
        virtual uno::Any SAL_CALL getByName( const OUString& aName )
            throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException)
         { return BasicGroupAccess::getByName(aName); }

        virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw( uno::RuntimeException)
         { return BasicGroupAccess::getElementNames(); }

        virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(uno::RuntimeException)
         { return BasicGroupAccess::hasByName(aName); }

    // New Interface methods
        // XNameReplace
        virtual void SAL_CALL
            replaceByName( const OUString& rName, const uno::Any& rElement )
                throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException);

    protected:
                configapi::NodeGroupAccess& getGroupNode();
        virtual configapi::NodeGroupAccess* maybeGetUpdateAccess() = 0;
    };

//........................................................................
} // namespace configmgr
//........................................................................

#endif // CONFIGMGR_API_GROUPUPDATE_HXX_


