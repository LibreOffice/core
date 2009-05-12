/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: setupdate.hxx,v $
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

#ifndef CONFIGMGR_API_SETUPDATE_HXX_
#define CONFIGMGR_API_SETUPDATE_HXX_

#include "setaccess.hxx"
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertyWithState.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

//........................................................................
namespace configmgr
{
//........................................................................
    namespace configapi { class NodeTreeSetAccess; class NodeValueSetAccess; }

//==========================================================================
//= BasicSet
//==========================================================================

/** base class for configuration nodes which are dynamic sets of complex types (trees)
*/
    class BasicSet
    : public BasicSetAccess
    , public css::container::XNameContainer
    , public css::beans::XPropertyWithState
    , public css::lang::XSingleServiceFactory
    {
    protected:
    // Destructors
        virtual ~BasicSet() {}

    public:
    // Base class Interface methods
        // XInterface joining
        uno::Any SAL_CALL queryInterface( uno::Type const& rType ) throw (uno::RuntimeException );

        // XTypeProvider joining
        uno::Sequence< uno::Type > SAL_CALL getTypes( ) throw (uno::RuntimeException );
        uno::Sequence< sal_Int8 > SAL_CALL getImplementationId( ) throw (uno::RuntimeException ) = 0;

        // XElementAccess forwarding
        virtual uno::Type SAL_CALL getElementType(  ) throw(uno::RuntimeException)
        { return BasicSetAccess::getElementType(); }

        virtual sal_Bool SAL_CALL hasElements(  )  throw(uno::RuntimeException)
        { return BasicSetAccess::hasElements(); }

        // XNameAccess forwarding
        virtual uno::Any SAL_CALL getByName( const rtl::OUString& aName )
            throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException)
         { return BasicSetAccess::getByName(aName); }

        virtual uno::Sequence< rtl::OUString > SAL_CALL getElementNames(  ) throw( uno::RuntimeException)
         { return BasicSetAccess::getElementNames(); }

        virtual sal_Bool SAL_CALL hasByName( const rtl::OUString& aName ) throw(uno::RuntimeException)
         { return BasicSetAccess::hasByName(aName); }

    // New Interface methods
        // XNameReplace
        virtual void SAL_CALL
            replaceByName( const rtl::OUString& rName, const uno::Any& rElement )
                throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException);

        // XNameContainer
        virtual void SAL_CALL
            insertByName( const rtl::OUString& rName, const uno::Any& rElement)
                throw(css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, uno::RuntimeException);

        virtual void SAL_CALL
            removeByName( const rtl::OUString& rName )
                throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException);

        // XPropertyWithState
        virtual css::beans::PropertyState SAL_CALL
            getStateAsProperty(  )
                throw (uno::RuntimeException);

        virtual void SAL_CALL
            setToDefaultAsProperty(  )
                throw (css::lang::WrappedTargetException, uno::RuntimeException);

        virtual uno::Reference< uno::XInterface > SAL_CALL
            getDefaultAsProperty(  )
                throw (css::lang::WrappedTargetException, uno::RuntimeException);

        // XSingleServiceFactory
        virtual uno::Reference< uno::XInterface > SAL_CALL
            createInstance(  )
                throw(uno::Exception, uno::RuntimeException);

        virtual uno::Reference< uno::XInterface > SAL_CALL
            createInstanceWithArguments( const uno::Sequence< uno::Any >& aArguments )
                throw(uno::Exception, uno::RuntimeException);

    protected:
                configapi::NodeTreeSetAccess&   getSetNode();
        virtual configapi::NodeTreeSetAccess*   maybeGetUpdateAccess() = 0;
    };

//==========================================================================
//= BasicValueSet
//==========================================================================

/** class for configuration nodes which are dynamic sets of simple types (values)
*/
    class BasicValueSet
    : public BasicSetAccess
    , public css::beans::XPropertyWithState
    , public css::container::XNameContainer
    {
    protected:
    // Destructors
        virtual ~BasicValueSet() {}

    public:
    // Base class Interface methods
        // XInterface joining
        uno::Any SAL_CALL queryInterface( uno::Type const& rType ) throw (uno::RuntimeException );

        // XTypeProvider joining
        uno::Sequence< uno::Type > SAL_CALL getTypes( ) throw (uno::RuntimeException );
        uno::Sequence< sal_Int8 > SAL_CALL getImplementationId( ) throw (uno::RuntimeException ) = 0;

        // XElementAccess forwarding
        virtual uno::Type SAL_CALL getElementType(  ) throw(uno::RuntimeException)
        { return BasicSetAccess::getElementType(); }

        virtual sal_Bool SAL_CALL hasElements(  )  throw(uno::RuntimeException)
        { return BasicSetAccess::hasElements(); }

        // XNameAccess forwarding
        virtual uno::Any SAL_CALL getByName( const rtl::OUString& aName )
            throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException)
         { return BasicSetAccess::getByName(aName); }

        virtual uno::Sequence< rtl::OUString > SAL_CALL getElementNames(  ) throw( uno::RuntimeException)
         { return BasicSetAccess::getElementNames(); }

        virtual sal_Bool SAL_CALL hasByName( const rtl::OUString& aName ) throw(uno::RuntimeException)
         { return BasicSetAccess::hasByName(aName); }

    // New Interface methods
        // XNameReplace
        virtual void SAL_CALL
            replaceByName( const rtl::OUString& rName, const uno::Any& rElement )
                throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException);

        // XNameContainer
        virtual void SAL_CALL
            insertByName( const rtl::OUString& rName, const uno::Any& rElement)
                throw(css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, uno::RuntimeException);

        virtual void SAL_CALL
            removeByName( const rtl::OUString& rName )
                throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, uno::RuntimeException);

        // XPropertyWithState
        virtual css::beans::PropertyState SAL_CALL
            getStateAsProperty(  )
                throw (uno::RuntimeException);

        virtual void SAL_CALL
            setToDefaultAsProperty(  )
                throw (css::lang::WrappedTargetException, uno::RuntimeException);

        virtual uno::Reference< uno::XInterface > SAL_CALL
            getDefaultAsProperty(  )
                throw (css::lang::WrappedTargetException, uno::RuntimeException);

    protected:
                configapi::NodeValueSetAccess&  getSetNode();
        virtual configapi::NodeValueSetAccess*  maybeGetUpdateAccess() = 0;
    };

//........................................................................
} // namespace configmgr
//........................................................................

#endif // CONFIGMGR_API_VALUESETACCESS_HXX_


