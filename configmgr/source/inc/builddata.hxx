/*************************************************************************
 *
 *  $RCSfile: builddata.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2002-02-11 14:29:07 $
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

#ifndef CONFIGMGR_BUILDDATA_HXX
#define CONFIGMGR_BUILDDATA_HXX

#ifndef INCLUDED_MEMORY
#define INCLUDED_MEMORY
#include <memory>
#endif
//-----------------------------------------------------------------------------
namespace rtl { class OUString; }
//-----------------------------------------------------------------------------
namespace configmgr
{
//-----------------------------------------------------------------------------
    class INode;
    class ISubtree;
    class ValueNode;
//-----------------------------------------------------------------------------
    namespace memory { class Accessor; class UpdateAccessor; }
//-----------------------------------------------------------------------------
    namespace data
    {
//-----------------------------------------------------------------------------
        class TreeAddress;
        class TreeAccessor;
//-----------------------------------------------------------------------------
        TreeAddress buildTree(memory::UpdateAccessor& _aTargetMemory, TreeAccessor const& _aTree);
//-----------------------------------------------------------------------------
        TreeAddress buildTree(memory::UpdateAccessor& _aTargetMemory, rtl::OUString const & _aTreeName, INode const& _aNode, bool _bWithDefaults);
//-----------------------------------------------------------------------------
        TreeAddress buildElementTree(memory::UpdateAccessor& _aTargetMemory, INode const& _aNode, rtl::OUString const & _aTypeName, bool _bWithDefaults);
//-----------------------------------------------------------------------------
        void mergeDefaults(memory::UpdateAccessor& _aTargetMemory, TreeAddress _aBaseAddress, INode const& _aDefaultNode);
//-----------------------------------------------------------------------------
        void destroyTree(memory::UpdateAccessor& _aTreeMemory, TreeAddress _aBaseAddress);
//-----------------------------------------------------------------------------
        std::auto_ptr<INode> convertTree(TreeAccessor const & _aTree, bool _bUseTreeName);
//-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}   // namespace configmgr
//-----------------------------------------------------------------------------

#endif // CONFIGMGR_BUILDDATA_HXX


