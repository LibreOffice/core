/*************************************************************************
 *
 *  $RCSfile: setobjects.hxx,v $
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
    using configapi::ApiTreeImpl;
    using configapi::ApiProvider;

    using configapi::NodeAccess;
    using configapi::NodeSetInfoAccess;
    using configapi::NodeTreeSetAccess;
    using configapi::NodeValueSetAccess;

    using configapi::InnerElement;
    using configapi::SetElement;
    using configapi::RootElement;
    using configapi::UpdateRootElement;

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
        OInnerSetInfo(ApiTreeImpl& rTree, configuration::NodeRef const& aNode)
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
        virtual NodeAccess&         getNodeAccess();
        virtual NodeSetInfoAccess&  getNode();
        virtual InnerElement&       getElementClass();
    private:
        configapi::OInnerSetInfoAccess m_aAccessElement;
    };

/** updating access class for configuration nodes which are inner nodes and dynamic sets of complex types (trees)
*/
    class OInnerTreeSetUpdate
    : public BasicInnerElement
    , public BasicSet
    {
    public:
        // Construction/Destruction
        OInnerTreeSetUpdate(ApiTreeImpl& rTree, configuration::NodeRef const& aNode)
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
        virtual NodeAccess&         getNodeAccess();
        virtual NodeSetInfoAccess&  getNode();
        virtual NodeTreeSetAccess*  maybeGetUpdateAccess();
        virtual InnerElement&       getElementClass();
    private:
        configapi::OInnerTreeSetUpdateAccess m_aAccessElement;
    };


/** update access class for configuration nodes which are inner nodes and dynamic sets of  simple types (values)
*/
    class OInnerValueSetUpdate
    : public BasicInnerElement
    , public BasicValueSet
    {
    public:
        // Construction/Destruction
        OInnerValueSetUpdate(ApiTreeImpl& rTree, configuration::NodeRef const& aNode)
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
        virtual NodeAccess&         getNodeAccess();
        virtual NodeSetInfoAccess&  getNode();
        virtual NodeValueSetAccess* maybeGetUpdateAccess();
        virtual InnerElement&       getElementClass();
    private:
        configapi::OInnerValueSetUpdateAccess m_aAccessElement;
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
        OSetElementSetInfo(configuration::Tree const& aTree, ApiTreeImpl& rParentTree)
        : m_aAccessElement(static_cast<css::container::XChild*>(this),aTree,rParentTree)
        {
        }
        OSetElementSetInfo(configuration::Tree const& aTree, ApiProvider& rProvider, ApiTreeImpl* pParentTree = 0)
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
        virtual NodeAccess&         getNodeAccess();
        virtual NodeSetInfoAccess&  getNode();
        virtual SetElement&         getElementClass();
    private:
        configapi::OSetElementSetInfoAccess m_aAccessElement;
    };

/** updating access class for configuration nodes which are set elements and dynamic sets of complex types (trees)
*/
    class OSetElementTreeSetUpdate
    : public BasicSetElement
    , public BasicSet
    {
    public:
        // Construction/Destruction
        OSetElementTreeSetUpdate(configuration::Tree const& aTree, ApiTreeImpl& rParentTree)
        : m_aAccessElement(static_cast<css::container::XChild*>(this),aTree,rParentTree)
        {
        }
        OSetElementTreeSetUpdate(configuration::Tree const& aTree, ApiProvider& rProvider, ApiTreeImpl* pParentTree = 0)
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
        virtual NodeAccess&         getNodeAccess();
        virtual NodeSetInfoAccess&  getNode();
        virtual NodeTreeSetAccess*  maybeGetUpdateAccess();
        virtual SetElement&     getElementClass();
    private:
        configapi::OSetElementTreeSetUpdateAccess m_aAccessElement;
    };


/** update access class for configuration nodes which are set elements and dynamic sets of  simple types (values)
*/
    class OSetElementValueSetUpdate
    : public BasicSetElement
    , public BasicValueSet
    {
    public:
        // Construction/Destruction
        OSetElementValueSetUpdate(configuration::Tree const& aTree, ApiTreeImpl& rParentTree)
        : m_aAccessElement(static_cast<css::container::XChild*>(this),aTree,rParentTree)
        {
        }
        OSetElementValueSetUpdate(configuration::Tree const& aTree, ApiProvider& rProvider, ApiTreeImpl* pParentTree = 0)
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
        virtual NodeAccess&         getNodeAccess();
        virtual NodeSetInfoAccess&  getNode();
        virtual NodeValueSetAccess* maybeGetUpdateAccess();
        virtual SetElement&     getElementClass();
    private:
        configapi::OSetElementValueSetUpdateAccess m_aAccessElement;
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
        ORootElementSetInfo(ApiProvider& rProvider, configuration::Tree const& aTree)
            : m_aAccessElement(static_cast<css::lang::XComponent*>(this),rProvider,aTree)
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
        virtual NodeAccess&         getNodeAccess();
        virtual NodeSetInfoAccess&  getNode();
        virtual RootElement&        getElementClass();
    private:
        configapi::ORootElementSetInfoAccess m_aAccessElement;
    };

/** updating access class for configuration nodes which are root nodes and dynamic sets of complex types (trees)
*/
    class ORootElementTreeSetUpdate
    : public BasicUpdateElement
    , public BasicSet
    {
    public:
        // Construction/Destruction
        ORootElementTreeSetUpdate(ApiProvider& rProvider, configuration::Tree const& aTree)
        : m_aAccessElement(static_cast<css::lang::XComponent*>(this),rProvider,aTree)
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
        virtual NodeAccess&         getNodeAccess();
        virtual NodeSetInfoAccess&  getNode();
        virtual NodeTreeSetAccess*  maybeGetUpdateAccess();
        virtual UpdateRootElement&  getElementClass();
    private:
        configapi::ORootElementTreeSetUpdateAccess m_aAccessElement;
    };


/** update access class for configuration nodes which are root nodes and dynamic sets of  simple types (values)
*/
    class ORootElementValueSetUpdate
    : public BasicUpdateElement
    , public BasicValueSet
    {
    public:
        // Construction/Destruction
        ORootElementValueSetUpdate(ApiProvider& rProvider, configuration::Tree const& aTree)
        : m_aAccessElement(static_cast<css::lang::XComponent*>(this),rProvider,aTree)
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
        virtual NodeAccess&         getNodeAccess();
        virtual NodeSetInfoAccess&  getNode();
        virtual NodeValueSetAccess* maybeGetUpdateAccess();
        virtual UpdateRootElement&  getElementClass();
    private:
        configapi::ORootElementValueSetUpdateAccess m_aAccessElement;
    };


//........................................................................
} // namespace configmgr
//........................................................................

#endif // CONFIGMGR_API_SETOBJECTS_HXX_


