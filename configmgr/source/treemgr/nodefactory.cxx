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
    struct BasicNodeFactory : NodeFactory
    {
        rtl::Reference<configuration::NodeImpl> makeValueNode(sharable::ValueNode * node);
        rtl::Reference<configuration::NodeImpl> makeGroupNode(sharable::GroupNode * node);
        rtl::Reference<configuration::NodeImpl> makeSetNode(sharable::SetNode * node, configuration::Template* pTemplate);
    };
    //-------------------------------------------------------------------------

    rtl::Reference<configuration::NodeImpl> BasicNodeFactory::makeValueNode(sharable::ValueNode * node)
    {
        return new configuration::ValueElementNodeImpl(node);
    }
    //-------------------------------------------------------------------------

    rtl::Reference<configuration::NodeImpl> BasicNodeFactory::makeGroupNode(sharable::GroupNode * node)
    {
        return new configuration::GroupNodeImpl(node);
    }
    //-------------------------------------------------------------------------

    rtl::Reference<configuration::NodeImpl> BasicNodeFactory::makeSetNode(sharable::SetNode * node, configuration::Template* pTemplate)
    {
        return new configuration::SetNodeImpl(node, pTemplate);
    }
    //-------------------------------------------------------------------------

//-----------------------------------------------------------------------------

    struct DeferredNodeFactory : NodeFactory
    {
        rtl::Reference<configuration::NodeImpl> makeValueNode(sharable::ValueNode * node);
        rtl::Reference<configuration::NodeImpl> makeGroupNode(sharable::GroupNode * node);
        rtl::Reference<configuration::NodeImpl> makeSetNode(sharable::SetNode * node, configuration::Template* pTemplate);
    };
    //-------------------------------------------------------------------------

    rtl::Reference<configuration::NodeImpl> DeferredNodeFactory::makeValueNode(sharable::ValueNode * node)
    {
    //    OSL_ENSURE(false, "Wrong factory for value elements - should be immutable (=read-only)");
        return new configuration::ValueElementNodeImpl(node);
    }
    //-------------------------------------------------------------------------

    rtl::Reference<configuration::NodeImpl> DeferredNodeFactory::makeGroupNode(sharable::GroupNode * node)
    {
        return new configuration::DeferredGroupNodeImpl(node);
    }
    //-------------------------------------------------------------------------

    rtl::Reference<configuration::NodeImpl> DeferredNodeFactory::makeSetNode(sharable::SetNode * node, configuration::Template* pTemplate)
    {
        return new configuration::DeferredSetNodeImpl(node, pTemplate);
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

