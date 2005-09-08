/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nodefactory.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:31:03 $
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
#include <stdio.h>
#include "nodefactory.hxx"

#ifndef CONFIGMGR_NODEIMPLOBJECTS_HXX_
#include "nodeimplobj.hxx"
#endif
#ifndef CONFIGMGR_VALUENODEACCESS_HXX
#include "valuenodeaccess.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEACCESS_HXX
#include "groupnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_SETNODEACCESS_HXX
#include "setnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
namespace view
{

// Creating Specific types of nodes
//-----------------------------------------------------------------------------
namespace
{
//---------------------------------------------------------------------
    using configuration::NodeImplHolder;
    using configuration::Template;
    using data::ValueNodeAccess;
    using data::GroupNodeAccess;
    using data::SetNodeAccess;
//---------------------------------------------------------------------
/*  static bool isTreeSet(SetNodeAccess const& _aNodeAccess, Template* pTemplate)
    {
        OSL_ENSURE(pTemplate,"ERROR: Trying to instantiate a set without a template");
        if (!pTemplate) throw Exception("ERROR: Trying to instantiate a set without a template");

        OSL_ENSURE(pTemplate->getName().toString() == _aNodeAccess.getElementTemplateName(),
                    "WARNING: Template name mismatch creating a set node");

        return ! pTemplate->isInstanceValue();
    }
*/
//---------------------------------------------------------------------
    struct BasicNodeFactory : NodeFactory
    {
        NodeImplHolder makeValueNode(ValueNodeAccess const& _aNodeAccess);
        NodeImplHolder makeGroupNode(GroupNodeAccess const& _aNodeAccess);
        NodeImplHolder makeSetNode  (SetNodeAccess const& _aNodeAccess, Template* pTemplate);
    };
    //-------------------------------------------------------------------------

    NodeImplHolder BasicNodeFactory::makeValueNode(ValueNodeAccess const& _aNodeAccess)
    {
        return new configuration::ValueElementNodeImpl(_aNodeAccess.address());
    }
    //-------------------------------------------------------------------------

    NodeImplHolder BasicNodeFactory::makeGroupNode(GroupNodeAccess const& _aNodeAccess)
    {
        return new configuration::GroupNodeImpl(_aNodeAccess.address());
    }
    //-------------------------------------------------------------------------

    NodeImplHolder BasicNodeFactory::makeSetNode  (SetNodeAccess const& _aNodeAccess, Template* pTemplate)
    {
        return new configuration::SetNodeImpl(_aNodeAccess.address(),pTemplate);
    }
    //-------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/*
    struct DirectNodeFactory : NodeFactory
    {
        NodeImplHolder makeValueNode(ValueNodeAccess const& _aNodeAccess);
        NodeImplHolder makeGroupNode(GroupNodeAccess const& _aNodeAccess);
        NodeImplHolder makeSetNode  (SetNodeAccess const& _aNodeAccess, Template* pTemplate);
    };
    //-------------------------------------------------------------------------

    NodeImplHolder DirectNodeFactory::makeValueNode(ValueNodeAccess const& _aNodeAccess)
    {
        return new DirectValueElementNodeImpl(_aNodeAccess.address());
    }
    //-------------------------------------------------------------------------

    NodeImplHolder DirectNodeFactory::makeGroupNode(GroupNodeAccess const& _aNodeAccess)
    {
        return new DirectGroupNodeImpl(_aNodeAccess.address());
    }
    //-------------------------------------------------------------------------

    NodeImplHolder DirectNodeFactory::makeSetNode  (SetNodeAccess const& _aNodeAccess, Template* pTemplate)
    {
        if (isTreeSet(_aNodeAccess,pTemplate))
            return new DirectTreeSetNodeImpl(_aNodeAccess.address(),pTemplate);
        else
            return new DirectValueSetNodeImpl(_aNodeAccess.address(),pTemplate);
    }
    //-------------------------------------------------------------------------
*/
//-----------------------------------------------------------------------------

    struct DeferredNodeFactory : NodeFactory
    {
        NodeImplHolder makeValueNode(ValueNodeAccess const& _aNodeAccess);
        NodeImplHolder makeGroupNode(GroupNodeAccess const& _aNodeAccess);
        NodeImplHolder makeSetNode  (SetNodeAccess const& _aNodeAccess, Template* pTemplate);
    };
    //-------------------------------------------------------------------------

    NodeImplHolder DeferredNodeFactory::makeValueNode(ValueNodeAccess const& _aNodeAccess)
    {
    //    OSL_ENSURE(false, "Wrong factory for value elements - should be immutable (=read-only)");
        return new configuration::ValueElementNodeImpl(_aNodeAccess.address());
    }
    //-------------------------------------------------------------------------

    NodeImplHolder DeferredNodeFactory::makeGroupNode(GroupNodeAccess const& _aNodeAccess)
    {
        return new configuration::DeferredGroupNodeImpl(_aNodeAccess.address());
    }
    //-------------------------------------------------------------------------

    NodeImplHolder DeferredNodeFactory::makeSetNode  (SetNodeAccess const& _aNodeAccess, Template* pTemplate)
    {
        return new configuration::DeferredSetNodeImpl(_aNodeAccess.address(),pTemplate);
    }
    //-------------------------------------------------------------------------

//-----------------------------------------------------------------------------
} // anonymous
//-----------------------------------------------------------------------------

// Different standard (static) factories
//---------------------------------------------------------------------

    /// provides a factory for immediately commiting node implementations
    NodeFactory& getDirectAccessFactory()
    {
        static BasicNodeFactory aFactory;
        return aFactory;
    }
    /// provides a factory for read-only node implementations
    NodeFactory& getReadAccessFactory()
    {
        static BasicNodeFactory aFactory;
        return aFactory;
    }
    /// provides a factory for nodes that cache changes temporarily
    NodeFactory& getDeferredChangeFactory()
    {
        static DeferredNodeFactory aFactory;
        return aFactory;
    }

//---------------------------------------------------------------------
} // view

//-----------------------------------------------------------------------------
} // configmgr

