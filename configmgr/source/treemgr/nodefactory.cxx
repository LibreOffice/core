/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodefactory.cxx,v $
 * $Revision: 1.9 $
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
#include <stdio.h>
#include "nodefactory.hxx"
#include "nodeimplobj.hxx"
#include "valuenodeaccess.hxx"
#include "groupnodeaccess.hxx"
#include "setnodeaccess.hxx"
#include "configpath.hxx"
#include <osl/diagnose.h>

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
    using configuration::NodeImpl;
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
        rtl::Reference<NodeImpl> makeValueNode(ValueNodeAccess const& _aNodeAccess);
        rtl::Reference<NodeImpl> makeGroupNode(GroupNodeAccess const& _aNodeAccess);
        rtl::Reference<NodeImpl> makeSetNode  (SetNodeAccess const& _aNodeAccess, Template* pTemplate);
    };
    //-------------------------------------------------------------------------

    rtl::Reference<NodeImpl> BasicNodeFactory::makeValueNode(ValueNodeAccess const& _aNodeAccess)
    {
        return new configuration::ValueElementNodeImpl(_aNodeAccess);
    }
    //-------------------------------------------------------------------------

    rtl::Reference<NodeImpl> BasicNodeFactory::makeGroupNode(GroupNodeAccess const& _aNodeAccess)
    {
        return new configuration::GroupNodeImpl(_aNodeAccess);
    }
    //-------------------------------------------------------------------------

    rtl::Reference<NodeImpl> BasicNodeFactory::makeSetNode  (SetNodeAccess const& _aNodeAccess, Template* pTemplate)
    {
        return new configuration::SetNodeImpl(_aNodeAccess,pTemplate);
    }
    //-------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/*
    struct DirectNodeFactory : NodeFactory
    {
        rtl::Reference<NodeImpl> makeValueNode(ValueNodeAccess const& _aNodeAccess);
        rtl::Reference<NodeImpl> makeGroupNode(GroupNodeAccess const& _aNodeAccess);
        rtl::Reference<NodeImpl> makeSetNode  (SetNodeAccess const& _aNodeAccess, Template* pTemplate);
    };
    //-------------------------------------------------------------------------

    rtl::Reference<NodeImpl> DirectNodeFactory::makeValueNode(ValueNodeAccess const& _aNodeAccess)
    {
        return new DirectValueElementNodeImpl(_aNodeAccess);
    }
    //-------------------------------------------------------------------------

    rtl::Reference<NodeImpl> DirectNodeFactory::makeGroupNode(GroupNodeAccess const& _aNodeAccess)
    {
        return new DirectGroupNodeImpl(_aNodeAccess);
    }
    //-------------------------------------------------------------------------

    rtl::Reference<NodeImpl> DirectNodeFactory::makeSetNode  (SetNodeAccess const& _aNodeAccess, Template* pTemplate)
    {
        if (isTreeSet(_aNodeAccess,pTemplate))
            return new DirectTreeSetNodeImpl(_aNodeAccess,pTemplate);
        else
            return new DirectValueSetNodeImpl(_aNodeAccess,pTemplate);
    }
    //-------------------------------------------------------------------------
*/
//-----------------------------------------------------------------------------

    struct DeferredNodeFactory : NodeFactory
    {
        rtl::Reference<NodeImpl> makeValueNode(ValueNodeAccess const& _aNodeAccess);
        rtl::Reference<NodeImpl> makeGroupNode(GroupNodeAccess const& _aNodeAccess);
        rtl::Reference<NodeImpl> makeSetNode  (SetNodeAccess const& _aNodeAccess, Template* pTemplate);
    };
    //-------------------------------------------------------------------------

    rtl::Reference<NodeImpl> DeferredNodeFactory::makeValueNode(ValueNodeAccess const& _aNodeAccess)
    {
    //    OSL_ENSURE(false, "Wrong factory for value elements - should be immutable (=read-only)");
        return new configuration::ValueElementNodeImpl(_aNodeAccess);
    }
    //-------------------------------------------------------------------------

    rtl::Reference<NodeImpl> DeferredNodeFactory::makeGroupNode(GroupNodeAccess const& _aNodeAccess)
    {
        return new configuration::DeferredGroupNodeImpl(_aNodeAccess);
    }
    //-------------------------------------------------------------------------

    rtl::Reference<NodeImpl> DeferredNodeFactory::makeSetNode  (SetNodeAccess const& _aNodeAccess, Template* pTemplate)
    {
        return new configuration::DeferredSetNodeImpl(_aNodeAccess,pTemplate);
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

