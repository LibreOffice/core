/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: setobjects.hxx,v $
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

#ifndef CONFIGMGR_API_SETOBJECTS_HXX_
#define CONFIGMGR_API_SETOBJECTS_HXX_

#include "setaccess.hxx"
#include "setupdate.hxx"
#include "elementaccess.hxx"

#include "apiaccessobj.hxx"

//........................................................................
namespace configmgr
{
//........................................................................

//==========================================================================
//= Inner Set Instances
//==========================================================================

/** read-only access class for configuration nodes which are inner nodes and dynamic sets
*/
    class OInnerSetInfo
    : public BasicInnerElement
    , public BasicSetAccess
    {
    public:
        // Construction/Destruction
        OInnerSetInfo(configapi::ApiTreeImpl& rTree, configuration::NodeRef const& aNode)
        : m_aAccessElement(static_cast<css::container::XChild*>(this),rTree,aNode)
        {
        }

        // XInterface refcounting
        void SAL_CALL acquire( ) throw ();
        void SAL_CALL release( ) throw ();

        // XInterface joining
        uno::Any SAL_CALL queryInterface( uno::Type const& rType ) throw (uno::RuntimeException );

        // XTypeProvider joining
        uno::Sequence< uno::Type > SAL_CALL getTypes( ) throw (uno::RuntimeException );
        uno::Sequence< sal_Int8 > SAL_CALL getImplementationId( ) throw (uno::RuntimeException );

    // Base class implementation
        virtual configapi::NodeAccess&          getNodeAccess();
        virtual configapi::NodeSetInfoAccess&   getNode();
        virtual configapi::InnerElement&        getElementClass();
    private:
        configapi::OInnerElement<configapi::NodeSetInfoAccess> m_aAccessElement;
    };

/** updating access class for configuration nodes which are inner nodes and dynamic sets of complex types (trees)
*/
    class OInnerTreeSetUpdate
    : public BasicInnerElement
    , public BasicSet
    {
    public:
        // Construction/Destruction
        OInnerTreeSetUpdate(configapi::ApiTreeImpl& rTree, configuration::NodeRef const& aNode)
        : m_aAccessElement(static_cast<css::container::XChild*>(this),rTree,aNode)
        {
        }

        // XInterface refcounting
        void SAL_CALL acquire( ) throw ();
        void SAL_CALL release( ) throw ();

        // XInterface joining
        uno::Any SAL_CALL queryInterface( uno::Type const& rType ) throw (uno::RuntimeException );

        // XTypeProvider joining
        uno::Sequence< uno::Type > SAL_CALL getTypes( ) throw (uno::RuntimeException );
        uno::Sequence< sal_Int8 > SAL_CALL getImplementationId( ) throw (uno::RuntimeException );

    // Base class implementation
        virtual configapi::NodeAccess&          getNodeAccess();
        virtual configapi::NodeSetInfoAccess&   getNode();
        virtual configapi::NodeTreeSetAccess*   maybeGetUpdateAccess();
        virtual configapi::InnerElement&        getElementClass();
    private:
        configapi::OInnerElement<configapi::NodeTreeSetAccess> m_aAccessElement;
    };


/** update access class for configuration nodes which are inner nodes and dynamic sets of  simple types (values)
*/
    class OInnerValueSetUpdate
    : public BasicInnerElement
    , public BasicValueSet
    {
    public:
        // Construction/Destruction
        OInnerValueSetUpdate(configapi::ApiTreeImpl& rTree, configuration::NodeRef const& aNode)
        : m_aAccessElement(static_cast<css::container::XChild*>(this),rTree,aNode)
        {
        }

        // XInterface refcounting
        void SAL_CALL acquire( ) throw ();
        void SAL_CALL release( ) throw ();

        // XInterface joining
        uno::Any SAL_CALL queryInterface( uno::Type const& rType ) throw (uno::RuntimeException );

        // XTypeProvider joining
        uno::Sequence< uno::Type > SAL_CALL getTypes( ) throw (uno::RuntimeException );
        uno::Sequence< sal_Int8 > SAL_CALL getImplementationId( ) throw (uno::RuntimeException );

    // Base class implementation
        virtual configapi::NodeAccess&          getNodeAccess();
        virtual configapi::NodeSetInfoAccess&   getNode();
        virtual configapi::NodeValueSetAccess*  maybeGetUpdateAccess();
        virtual configapi::InnerElement&        getElementClass();
    private:
        configapi::OInnerElement<configapi::NodeValueSetAccess> m_aAccessElement;
    };

//==========================================================================
//= Set Element Set Instances
//==========================================================================

/** read-only access class for configuration nodes which are set elements and dynamic sets
*/
    class OSetElementSetInfo
    : public BasicSetElement
    , public BasicSetAccess
    {
    public:
        // Construction/Destruction
        OSetElementSetInfo(rtl::Reference< configuration::Tree > const& aTree, configapi::ApiTreeImpl& rParentTree)
        : m_aAccessElement(static_cast<css::container::XChild*>(this),aTree,rParentTree)
        {
        }
        OSetElementSetInfo(rtl::Reference< configuration::Tree > const& aTree, configapi::ApiProvider& rProvider, configapi::ApiTreeImpl* pParentTree = 0)
        : m_aAccessElement(static_cast<css::container::XChild*>(this),aTree,rProvider,pParentTree)
        {
        }

        // XInterface refcounting
        void SAL_CALL acquire( ) throw ();
        void SAL_CALL release( ) throw ();

        // XInterface joining
        uno::Any SAL_CALL queryInterface( uno::Type const& rType ) throw (uno::RuntimeException );

        // XTypeProvider joining
        uno::Sequence< uno::Type > SAL_CALL getTypes( ) throw (uno::RuntimeException );
        uno::Sequence< sal_Int8 > SAL_CALL getImplementationId( ) throw (uno::RuntimeException );

    // Base class implementation
        virtual configapi::NodeAccess&          getNodeAccess();
        virtual configapi::NodeSetInfoAccess&   getNode();
        virtual configapi::SetElement&          getElementClass();
    private:
        configapi::OSetElement<configapi::NodeSetInfoAccess> m_aAccessElement;
    };

/** updating access class for configuration nodes which are set elements and dynamic sets of complex types (trees)
*/
    class OSetElementTreeSetUpdate
    : public BasicSetElement
    , public BasicSet
    {
    public:
        // Construction/Destruction
        OSetElementTreeSetUpdate(rtl::Reference< configuration::Tree > const& aTree, configapi::ApiTreeImpl& rParentTree)
        : m_aAccessElement(static_cast<css::container::XChild*>(this),aTree,rParentTree)
        {
        }
        OSetElementTreeSetUpdate(rtl::Reference< configuration::Tree > const& aTree, configapi::ApiProvider& rProvider, configapi::ApiTreeImpl* pParentTree = 0)
        : m_aAccessElement(static_cast<css::container::XChild*>(this),aTree,rProvider,pParentTree)
        {
        }

        // XInterface refcounting
        void SAL_CALL acquire( ) throw ();
        void SAL_CALL release( ) throw ();

        // XInterface joining
        uno::Any SAL_CALL queryInterface( uno::Type const& rType ) throw (uno::RuntimeException );

        // XTypeProvider joining
        uno::Sequence< uno::Type > SAL_CALL getTypes( ) throw (uno::RuntimeException );
        uno::Sequence< sal_Int8 > SAL_CALL getImplementationId( ) throw (uno::RuntimeException );

    // Base class implementation
        virtual configapi::NodeAccess&          getNodeAccess();
        virtual configapi::NodeSetInfoAccess&   getNode();
        virtual configapi::NodeTreeSetAccess*   maybeGetUpdateAccess();
        virtual configapi::SetElement&      getElementClass();
    private:
        configapi::OSetElement<configapi::NodeTreeSetAccess> m_aAccessElement;
    };


/** update access class for configuration nodes which are set elements and dynamic sets of  simple types (values)
*/
    class OSetElementValueSetUpdate
    : public BasicSetElement
    , public BasicValueSet
    {
    public:
        // Construction/Destruction
        OSetElementValueSetUpdate(rtl::Reference< configuration::Tree > const& aTree, configapi::ApiTreeImpl& rParentTree)
        : m_aAccessElement(static_cast<css::container::XChild*>(this),aTree,rParentTree)
        {
        }
        OSetElementValueSetUpdate(rtl::Reference< configuration::Tree > const& aTree, configapi::ApiProvider& rProvider, configapi::ApiTreeImpl* pParentTree = 0)
        : m_aAccessElement(static_cast<css::container::XChild*>(this),aTree,rProvider,pParentTree)
        {
        }

        // XInterface refcounting
        void SAL_CALL acquire( ) throw ();
        void SAL_CALL release( ) throw ();

        // XInterface joining
        uno::Any SAL_CALL queryInterface( uno::Type const& rType ) throw (uno::RuntimeException );

        // XTypeProvider joining
        uno::Sequence< uno::Type > SAL_CALL getTypes( ) throw (uno::RuntimeException );
        uno::Sequence< sal_Int8 > SAL_CALL getImplementationId( ) throw (uno::RuntimeException );

    // Base class implementation
        virtual configapi::NodeAccess&          getNodeAccess();
        virtual configapi::NodeSetInfoAccess&   getNode();
        virtual configapi::NodeValueSetAccess*  maybeGetUpdateAccess();
        virtual configapi::SetElement&      getElementClass();
    private:
        configapi::OSetElement<configapi::NodeValueSetAccess> m_aAccessElement;
    };

//==========================================================================
//= Root Element Set Instances
//==========================================================================

/** read-only access class for configuration nodes which are root nodes and dynamic sets
*/
    class ORootElementSetInfo
    : public BasicRootElement
    , public BasicSetAccess
    {
    public:
        // Construction/Destruction
        ORootElementSetInfo(configapi::ApiProvider& rProvider, rtl::Reference< configuration::Tree > const& aTree, vos::ORef< OOptions >const& _xOptions)
            : m_aAccessElement(static_cast<css::lang::XComponent*>(this),rProvider,aTree,_xOptions)
        {
        }

        // XInterface refcounting
        void SAL_CALL acquire( ) throw ();
        void SAL_CALL release( ) throw ();

        // XInterface joining
        uno::Any SAL_CALL queryInterface( uno::Type const& rType ) throw (uno::RuntimeException );

        // XTypeProvider joining
        uno::Sequence< uno::Type > SAL_CALL getTypes( ) throw (uno::RuntimeException );
        uno::Sequence< sal_Int8 > SAL_CALL getImplementationId( ) throw (uno::RuntimeException );

    // Base class implementation
        virtual configapi::NodeAccess&          getNodeAccess();
        virtual configapi::NodeSetInfoAccess&   getNode();
        virtual configapi::RootElement&     getElementClass();
    private:
        configapi::OReadRootElement<configapi::NodeSetInfoAccess> m_aAccessElement;
    };

/** updating access class for configuration nodes which are root nodes and dynamic sets of complex types (trees)
*/
    class ORootElementTreeSetUpdate
    : public BasicUpdateElement
    , public BasicSet
    {
    public:
        // Construction/Destruction
        ORootElementTreeSetUpdate(configapi::ApiProvider& rProvider, rtl::Reference< configuration::Tree > const& aTree, vos::ORef< OOptions >const& _xOptions)
        : m_aAccessElement(static_cast<css::lang::XComponent*>(this),rProvider,aTree, _xOptions)
        {
        }

        // XInterface refcounting
        void SAL_CALL acquire( ) throw ();
        void SAL_CALL release( ) throw ();

        // XInterface joining
        uno::Any SAL_CALL queryInterface( uno::Type const& rType ) throw (uno::RuntimeException );

        // XTypeProvider joining
        uno::Sequence< uno::Type > SAL_CALL getTypes( ) throw (uno::RuntimeException );
        uno::Sequence< sal_Int8 > SAL_CALL getImplementationId( ) throw (uno::RuntimeException );

    // Base class implementation
        virtual configapi::NodeAccess&          getNodeAccess();
        virtual configapi::NodeSetInfoAccess&   getNode();
        virtual configapi::NodeTreeSetAccess*   maybeGetUpdateAccess();
        virtual configapi::UpdateRootElement&   getElementClass();
    private:
        configapi::OUpdateRootElement<configapi::NodeTreeSetAccess> m_aAccessElement;
    };


/** update access class for configuration nodes which are root nodes and dynamic sets of  simple types (values)
*/
    class ORootElementValueSetUpdate
    : public BasicUpdateElement
    , public BasicValueSet
    {
    public:
        // Construction/Destruction
        ORootElementValueSetUpdate(configapi::ApiProvider& rProvider, rtl::Reference< configuration::Tree > const& aTree, vos::ORef< OOptions >const& _xOptions)
        : m_aAccessElement(static_cast<css::lang::XComponent*>(this),rProvider,aTree,_xOptions)
        {
        }

        // XInterface refcounting
        void SAL_CALL acquire( ) throw ();
        void SAL_CALL release( ) throw ();

        // XInterface joining
        uno::Any SAL_CALL queryInterface( uno::Type const& rType ) throw (uno::RuntimeException );

        // XTypeProvider joining
        uno::Sequence< uno::Type > SAL_CALL getTypes( ) throw (uno::RuntimeException );
        uno::Sequence< sal_Int8 > SAL_CALL getImplementationId( ) throw (uno::RuntimeException );

    // Base class implementation
        virtual configapi::NodeAccess&          getNodeAccess();
        virtual configapi::NodeSetInfoAccess&   getNode();
        virtual configapi::NodeValueSetAccess*  maybeGetUpdateAccess();
        virtual configapi::UpdateRootElement&   getElementClass();
    private:
        configapi::OUpdateRootElement<configapi::NodeValueSetAccess> m_aAccessElement;
    };


//........................................................................
} // namespace configmgr
//........................................................................

#endif // CONFIGMGR_API_SETOBJECTS_HXX_


