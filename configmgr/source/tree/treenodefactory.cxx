/*************************************************************************
 *
 *  $RCSfile: treenodefactory.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:39 $
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

#include "treenodefactory.hxx"

#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif
#ifndef CONFIGMGR_SUBTREE_HXX
#include "subtree.hxx"
#endif

#ifndef CONFIGMGR_TREE_CHANGEFACTORY_HXX
#include "treechangefactory.hxx"
#endif

#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif

namespace configmgr
{

//= static default ============================================================
OTreeNodeFactory& getDefaultTreeNodeFactory()
{
    static OTreeNodeFactory aDefaultFactory;
    return aDefaultFactory;
}

//= ValueNodes ============================================================

std::auto_ptr<ValueNode> OTreeNodeFactory::createValueNode(
                            rtl::OUString const& aName,
                            uno::Any const& aValue,
                            node::Attributes _aAttrs)
{
    OSL_ENSURE(aValue.hasValue(), "OTreeNodeFactory: Creating a value node having no type");
    return std::auto_ptr<ValueNode>( new ValueNode(aName, aValue, _aAttrs) );
}

//-----------------------------------------------

std::auto_ptr<ValueNode> OTreeNodeFactory::createValueNode(
                            rtl::OUString const& aName,
                            uno::Any const& aValue,
                            uno::Any const& aDefault,
                            node::Attributes _aAttrs)
{
    OSL_ENSURE(aValue.hasValue() || aDefault.hasValue(), "OTreeNodeFactory: Creating a value node having no type");
    return std::auto_ptr<ValueNode>( new ValueNode(aName, aValue, aDefault, _aAttrs) );
}


//-----------------------------------------------

std::auto_ptr<ValueNode>  OTreeNodeFactory::createNullValueNode(
                            rtl::OUString const& aName,
                            uno::Type const& aType,
                            node::Attributes _aAttrs)
{
    OSL_ENSURE(aType.getTypeClass() != uno::TypeClass_VOID, "OTreeNodeFactory: Creating a value node having VOID type");
    return std::auto_ptr<ValueNode>( new ValueNode(aName, aType, _aAttrs) );
}


//= ISubtree ============================================================

std::auto_ptr<ISubtree> OTreeNodeFactory::createDummyTree(Name const& _aName, Name const& _aElementTypeName)
{
    std::auto_ptr<ISubtree> pResult;

    if (_aElementTypeName.isEmpty())
    {
        pResult.reset( new Subtree(_aName.toString(),node::Attributes()) );
    }
    else
    {
        pResult.reset( new Subtree(_aName.toString(),
                                   _aElementTypeName.toString(),
                                   getDummySetElementModule().toString(),
                                   node::Attributes()) );
    }
    return pResult;
}

//-----------------------------------------------

std::auto_ptr<ISubtree> OTreeNodeFactory::createGroupNode(
                            rtl::OUString const& aName,
                            node::Attributes _aAttrs)
{
    return std::auto_ptr<ISubtree>( new Subtree(aName, _aAttrs) );
}


//-----------------------------------------------

std::auto_ptr<ISubtree> OTreeNodeFactory::createSetNode(
                            rtl::OUString const& aName,
                            rtl::OUString const& _rTemplateName,
                            rtl::OUString const& _rTemplateModule,
                            node::Attributes _aAttrs)
{
    return std::auto_ptr<ISubtree>( new Subtree(aName, _rTemplateName, _rTemplateModule, _aAttrs) );
}

//-----------------------------------------------

} // namespace configmgr


