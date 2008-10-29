/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodefactory.hxx,v $
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

#ifndef CONFIGMGR_CONFIGNODEFACTORY_HXX_
#define CONFIGMGR_CONFIGNODEFACTORY_HXX_

#include <rtl/ref.hxx>

namespace configmgr
{
    namespace configuration
    {
        class NodeImpl;
        class Template;
    }
    namespace sharable {
        struct GroupNode;
        struct SetNode;
        struct ValueNode;
    }
//-----------------------------------------------------------------------------
    namespace view
    {
//-----------------------------------------------------------------------------

// Creating Specific types of nodes
//-----------------------------------------------------------------------------

        struct NodeFactory
        {
            virtual rtl::Reference<configuration::NodeImpl> makeValueNode(sharable::ValueNode * node) = 0;
            virtual rtl::Reference<configuration::NodeImpl> makeGroupNode(sharable::GroupNode * node) = 0;
            virtual rtl::Reference<configuration::NodeImpl> makeSetNode(sharable::SetNode * node, configuration::Template* pTemplate) = 0;
        };
    }
//-----------------------------------------------------------------------------

}

#endif // CONFIGMGR_CONFIGNODEFACTORY_HXX_
