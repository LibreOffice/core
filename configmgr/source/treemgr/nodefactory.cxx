/*************************************************************************
 *
 *  $RCSfile: nodefactory.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dg $ $Date: 2000-11-13 11:54:51 $
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
#include <stdio.h>
#include "nodefactory.hxx"

#include "nodeimplobj.hxx"

#include "configpath.hxx"
#include "cmtreemodel.hxx"

#include <osl/diagnose.h>

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------

// Creating Specific types of nodes
//-----------------------------------------------------------------------------
namespace
{
//---------------------------------------------------------------------
    static bool isTreeSet(ISubtree& rOriginal, Template* pTemplate)
    {
        OSL_ENSURE(pTemplate,"ERROR: Trying to instantiate a set without a template");
        if (!pTemplate) throw Exception("ERROR: Trying to instantiate a set without a template");

        OSL_ENSURE(pTemplate->getName() == Path::parse(rOriginal.getChildTemplateName()).back(),
                    "WARNING: Template name mismatch creating a set node");

        return ! pTemplate->isInstanceValue();
    }

//---------------------------------------------------------------------
    struct ReadOnlyNodeFactory : NodeFactory
    {
        NodeImplHolder makeValueNode(ValueNode& rOriginal);
        NodeImplHolder makeGroupNode(ISubtree& rOriginal);
        NodeImplHolder makeSetNode  (ISubtree& rOriginal, Template* pTemplate);
    };
    //-------------------------------------------------------------------------

    NodeImplHolder ReadOnlyNodeFactory::makeValueNode(ValueNode& rOriginal)
    {
        return new ReadOnlyValueNodeImpl(rOriginal);
    }
    //-------------------------------------------------------------------------

    NodeImplHolder ReadOnlyNodeFactory::makeGroupNode(ISubtree& rOriginal)
    {
        return new ReadOnlyGroupNodeImpl(rOriginal);
    }
    //-------------------------------------------------------------------------

    NodeImplHolder ReadOnlyNodeFactory::makeSetNode  (ISubtree& rOriginal, Template* pTemplate)
    {
        if (isTreeSet(rOriginal,pTemplate))
            return new ReadOnlyTreeSetNodeImpl(rOriginal,pTemplate);
        else
            return new ReadOnlyValueSetNodeImpl(rOriginal,pTemplate);
    }
    //-------------------------------------------------------------------------

//-----------------------------------------------------------------------------

    struct DirectNodeFactory : NodeFactory
    {
        NodeImplHolder makeValueNode(ValueNode& rOriginal);
        NodeImplHolder makeGroupNode(ISubtree& rOriginal);
        NodeImplHolder makeSetNode  (ISubtree& rOriginal, Template* pTemplate);
    };
    //-------------------------------------------------------------------------

    NodeImplHolder DirectNodeFactory::makeValueNode(ValueNode& rOriginal)
    {
        return new DirectValueNodeImpl(rOriginal);
    }
    //-------------------------------------------------------------------------

    NodeImplHolder DirectNodeFactory::makeGroupNode(ISubtree& rOriginal)
    {
        return new DirectGroupNodeImpl(rOriginal);
    }
    //-------------------------------------------------------------------------

    NodeImplHolder DirectNodeFactory::makeSetNode  (ISubtree& rOriginal, Template* pTemplate)
    {
        if (isTreeSet(rOriginal,pTemplate))
            return new DirectTreeSetNodeImpl(rOriginal,pTemplate);
        else
            return new DirectValueSetNodeImpl(rOriginal,pTemplate);
    }
    //-------------------------------------------------------------------------

//-----------------------------------------------------------------------------

    struct DeferredNodeFactory : NodeFactory
    {
        NodeImplHolder makeValueNode(ValueNode& rOriginal);
        NodeImplHolder makeGroupNode(ISubtree& rOriginal);
        NodeImplHolder makeSetNode  (ISubtree& rOriginal, Template* pTemplate);
    };
    //-------------------------------------------------------------------------

    NodeImplHolder DeferredNodeFactory::makeValueNode(ValueNode& rOriginal)
    {
        return new DeferredValueNodeImpl(rOriginal);
    }
    //-------------------------------------------------------------------------

    NodeImplHolder DeferredNodeFactory::makeGroupNode(ISubtree& rOriginal)
    {
        return new DeferredGroupNodeImpl(rOriginal);
    }
    //-------------------------------------------------------------------------

    NodeImplHolder DeferredNodeFactory::makeSetNode  (ISubtree& rOriginal, Template* pTemplate)
    {
        if (isTreeSet(rOriginal,pTemplate))
            return new DeferredTreeSetNodeImpl(rOriginal,pTemplate);
        else
            return new DeferredValueSetNodeImpl(rOriginal,pTemplate);
    }
    //-------------------------------------------------------------------------

//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------

namespace NodeType
{
// Different standard (static) factories
//---------------------------------------------------------------------

    /// provides a factory for read-only node implementations
    NodeFactory& getReadAccessFactory()
    {
        static ReadOnlyNodeFactory aFactory;
        return aFactory;
    }
    /// provides a factory for immediately commiting node implementations
    NodeFactory& getDirectAccessFactory()
    {
        static DirectNodeFactory aFactory;
        return aFactory;
    }
    /// provides a factory for nodes that cache changes temporarily
    NodeFactory& getDeferredChangeFactory()
    {
        static DeferredNodeFactory aFactory;
        return aFactory;
    }

//---------------------------------------------------------------------
}

//-----------------------------------------------------------------------------

    }
}

