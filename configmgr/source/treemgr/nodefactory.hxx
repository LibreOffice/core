/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nodefactory.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:44:08 $
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

#ifndef CONFIGMGR_CONFIGNODEFACTORY_HXX_
#define CONFIGMGR_CONFIGNODEFACTORY_HXX_

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace data
    {
        class ValueNodeAccess;
        class GroupNodeAccess;
        class SetNodeAccess;
    }
//-----------------------------------------------------------------------------

    namespace configuration
    {
        class NodeImpl;
        class Template;
    }
//-----------------------------------------------------------------------------
    namespace view
    {
        typedef rtl::Reference<configuration::NodeImpl> NodeImplRef;

//-----------------------------------------------------------------------------

// Creating Specific types of nodes
//-----------------------------------------------------------------------------

        struct NodeFactory
        {
            virtual NodeImplRef makeValueNode(data::ValueNodeAccess const& _aNodeAccess) = 0;
            virtual NodeImplRef makeGroupNode(data::GroupNodeAccess const& _aNodeAccess) = 0;
            virtual NodeImplRef makeSetNode(data::SetNodeAccess const& _aNodeAccess, configuration::Template* pTemplate) = 0;
        };
    }
//-----------------------------------------------------------------------------

}

#endif // CONFIGMGR_CONFIGNODEFACTORY_HXX_
