/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: groupobjects.hxx,v $
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

#ifndef CONFIGMGR_API_GROUPOBJECTS_HXX_
#define CONFIGMGR_API_GROUPOBJECTS_HXX_

#include "groupaccess.hxx"
#include "groupupdate.hxx"
#include "propertysetaccess.hxx"
#include "elementaccess.hxx"

#include "apiaccessobj.hxx"

//........................................................................
namespace configmgr
{
//........................................................................

//==========================================================================
//= Inner Group Instances
//==========================================================================

/** read-only access class for configuration nodes which are inner nodes and groups of other nodes
*/
    class OInnerGroupInfo
    : public BasicInnerElement
    , public BasicGroupAccess
    , public BasicPropertySet
    {
    public:
        // Construction/Destruction
        OInnerGroupInfo(configapi::ApiTreeImpl& rTree, configuration::NodeRef const& aNode)
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
        virtual configapi::NodeAccess&              getNodeAccess();
        virtual configapi::NodeGroupInfoAccess& getNode();
        virtual configapi::NodeGroupAccess*     maybeGetUpdateAccess();
        virtual configapi::InnerElement&            getElementClass();
    private:
        configapi::OInnerElement<configapi::NodeGroupInfoAccess> m_aAccessElement;
    };

/** updating access class for configuration nodes which are inner nodes and groups of other nodes
*/
    class OInnerGroupUpdate
    : public BasicInnerElement
    , public BasicGroup
    , public BasicPropertySet
    {
    public:
        // Construction/Destruction
        OInnerGroupUpdate(configapi::ApiTreeImpl& rTree, configuration::NodeRef const& aNode)
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
        virtual configapi::NodeAccess&              getNodeAccess();
        virtual configapi::NodeGroupInfoAccess& getNode();
        virtual configapi::NodeGroupAccess*     maybeGetUpdateAccess();
        virtual configapi::InnerElement&            getElementClass();
    private:
        configapi::OInnerElement<configapi::NodeGroupAccess> m_aAccessElement;
    };


//==========================================================================
//= Set Element Group Instances
//==========================================================================

/** read-only access class for configuration nodes which are set elements and groups of other nodes
*/
    class OSetElementGroupInfo
    : public BasicSetElement
    , public BasicGroupAccess
    , public BasicPropertySet
    {
    public:
        // Construction/Destruction
        OSetElementGroupInfo(rtl::Reference< configuration::Tree > const& aTree, configapi::ApiTreeImpl& rParentTree)
        : m_aAccessElement(static_cast<css::container::XChild*>(this),aTree,rParentTree)
        {
        }
        OSetElementGroupInfo(rtl::Reference< configuration::Tree > const& aTree, configapi::ApiProvider& rProvider, configapi::ApiTreeImpl* pParentTree = 0)
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
        virtual configapi::NodeAccess&              getNodeAccess();
        virtual configapi::NodeGroupInfoAccess& getNode();
        virtual configapi::NodeGroupAccess*     maybeGetUpdateAccess();
        virtual configapi::SetElement&              getElementClass();
    private:
        configapi::OSetElement<configapi::NodeGroupInfoAccess> m_aAccessElement;
    };

/** updating access class for configuration nodes which are set elements and groups of other nodes
*/
    class OSetElementGroupUpdate
    : public BasicSetElement
    , public BasicGroup
    , public BasicPropertySet
    {
    public:
        // Construction/Destruction
        OSetElementGroupUpdate(rtl::Reference< configuration::Tree > const& aTree, configapi::ApiTreeImpl& rParentTree)
        : m_aAccessElement(static_cast<css::container::XChild*>(this),aTree,rParentTree)
        {
        }
        OSetElementGroupUpdate(rtl::Reference< configuration::Tree > const& aTree, configapi::ApiProvider& rProvider, configapi::ApiTreeImpl* pParentTree = 0)
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
        virtual configapi::NodeAccess&              getNodeAccess();
        virtual configapi::NodeGroupInfoAccess& getNode();
        virtual configapi::NodeGroupAccess*     maybeGetUpdateAccess();
        virtual configapi::SetElement&              getElementClass();
    private:
        configapi::OSetElement<configapi::NodeGroupAccess> m_aAccessElement;
    };


//==========================================================================
//= Root Element Set Instances
//==========================================================================

/** read-only access class for configuration nodes which are root nodes and groups of other nodes
*/
    class ORootElementGroupInfo
    : public BasicRootElement
    , public BasicGroupAccess
    , public BasicPropertySet
    {
    public:
        // Construction/Destruction
        ORootElementGroupInfo(configapi::ApiProvider& rProvider, rtl::Reference< configuration::Tree > const& aTree, vos::ORef< OOptions >const& _xOptions)
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
        virtual configapi::NodeAccess&              getNodeAccess();
        virtual configapi::NodeGroupInfoAccess& getNode();
        virtual configapi::NodeGroupAccess*     maybeGetUpdateAccess();
        virtual configapi::RootElement&         getElementClass();
    private:
        configapi::OReadRootElement<configapi::NodeGroupInfoAccess> m_aAccessElement;
    };

/** updating access class for configuration nodes which are root nodes and groups of other nodes
*/
    class ORootElementGroupUpdate
    : public BasicUpdateElement
    , public BasicGroup
    , public BasicPropertySet
    {
    public:
        // Construction/Destruction
        ORootElementGroupUpdate(configapi::ApiProvider& rProvider, rtl::Reference< configuration::Tree > const& aTree, vos::ORef< OOptions >const& _xOptions)
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
        virtual configapi::NodeAccess&              getNodeAccess();
        virtual configapi::NodeGroupInfoAccess& getNode();
        virtual configapi::NodeGroupAccess*     maybeGetUpdateAccess();
        virtual configapi::UpdateRootElement&       getElementClass();
    private:
        configapi::OUpdateRootElement<configapi::NodeGroupAccess> m_aAccessElement;
    };


//........................................................................
} // namespace configmgr
//........................................................................

#endif // CONFIGMGR_API_GROUPOBJECTS_HXX_


