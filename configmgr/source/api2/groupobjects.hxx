/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: groupobjects.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:14:30 $
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
    using configapi::ApiTreeImpl;
    using configapi::ApiProvider;

    using configapi::NodeAccess;
    using configapi::NodeGroupInfoAccess;
    using configapi::NodeGroupAccess;

    using configapi::InnerElement;
    using configapi::SetElement;
    using configapi::RootElement;
    using configapi::UpdateRootElement;

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
        OInnerGroupInfo(ApiTreeImpl& rTree, configuration::NodeRef const& aNode)
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
        virtual NodeAccess&             getNodeAccess();
        virtual NodeGroupInfoAccess&    getNode();
        virtual NodeGroupAccess*        maybeGetUpdateAccess();
        virtual InnerElement&           getElementClass();
    private:
        configapi::OInnerGroupInfoAccess m_aAccessElement;
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
        OInnerGroupUpdate(ApiTreeImpl& rTree, configuration::NodeRef const& aNode)
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
        virtual NodeAccess&             getNodeAccess();
        virtual NodeGroupInfoAccess&    getNode();
        virtual NodeGroupAccess*        maybeGetUpdateAccess();
        virtual InnerElement&           getElementClass();
    private:
        configapi::OInnerGroupUpdateAccess m_aAccessElement;
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
        OSetElementGroupInfo(configuration::TreeRef const& aTree, ApiTreeImpl& rParentTree)
        : m_aAccessElement(static_cast<css::container::XChild*>(this),aTree,rParentTree)
        {
        }
        OSetElementGroupInfo(configuration::TreeRef const& aTree, ApiProvider& rProvider, ApiTreeImpl* pParentTree = 0)
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
        virtual NodeAccess&             getNodeAccess();
        virtual NodeGroupInfoAccess&    getNode();
        virtual NodeGroupAccess*        maybeGetUpdateAccess();
        virtual SetElement&             getElementClass();
    private:
        configapi::OSetElementGroupInfoAccess m_aAccessElement;
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
        OSetElementGroupUpdate(configuration::TreeRef const& aTree, ApiTreeImpl& rParentTree)
        : m_aAccessElement(static_cast<css::container::XChild*>(this),aTree,rParentTree)
        {
        }
        OSetElementGroupUpdate(configuration::TreeRef const& aTree, ApiProvider& rProvider, ApiTreeImpl* pParentTree = 0)
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
        virtual NodeAccess&             getNodeAccess();
        virtual NodeGroupInfoAccess&    getNode();
        virtual NodeGroupAccess*        maybeGetUpdateAccess();
        virtual SetElement&             getElementClass();
    private:
        configapi::OSetElementGroupUpdateAccess m_aAccessElement;
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
        ORootElementGroupInfo(ApiProvider& rProvider, configuration::Tree const& aTree, vos::ORef< OOptions >const& _xOptions)
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
        virtual NodeAccess&             getNodeAccess();
        virtual NodeGroupInfoAccess&    getNode();
        virtual NodeGroupAccess*        maybeGetUpdateAccess();
        virtual RootElement&            getElementClass();
    private:
        configapi::ORootElementGroupInfoAccess m_aAccessElement;
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
        ORootElementGroupUpdate(ApiProvider& rProvider, configuration::Tree const& aTree, vos::ORef< OOptions >const& _xOptions)
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
        virtual NodeAccess&             getNodeAccess();
        virtual NodeGroupInfoAccess&    getNode();
        virtual NodeGroupAccess*        maybeGetUpdateAccess();
        virtual UpdateRootElement&      getElementClass();
    private:
        configapi::ORootElementGroupUpdateAccess m_aAccessElement;
    };


//........................................................................
} // namespace configmgr
//........................................................................

#endif // CONFIGMGR_API_GROUPOBJECTS_HXX_


