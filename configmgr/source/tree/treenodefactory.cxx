/*************************************************************************
 *
 *  $RCSfile: treenodefactory.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2001-03-16 17:31:24 $
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
/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */

#include "treenodefactory.hxx"

#include "valuenode.hxx"
#include "cmtree.hxx"


namespace configmgr
{

//= ValueNodes ============================================================

std::auto_ptr<ValueNode> OTreeNodeFactory::createValueNode(
                            rtl::OUString const& aName,
                            uno::Any const& aValue,
                            configuration::Attributes _aAttrs)
{
    OSL_ENSURE(aValue.hasValue(), "OTreeNodeFactory: Creating a value node having no type");
    return std::auto_ptr<ValueNode>( new ValueNode(aName, aValue, _aAttrs) );
}

//-----------------------------------------------

std::auto_ptr<ValueNode> OTreeNodeFactory::createValueNode(
                            rtl::OUString const& aName,
                            uno::Any const& aValue,
                            uno::Any const& aDefault,
                            configuration::Attributes _aAttrs)
{
    OSL_ENSURE(aValue.hasValue() || aDefault.hasValue(), "OTreeNodeFactory: Creating a value node having no type");
    return std::auto_ptr<ValueNode>( new ValueNode(aName, aValue, aDefault, _aAttrs) );
}


//-----------------------------------------------

std::auto_ptr<ValueNode>  OTreeNodeFactory::createNullValueNode(
                            rtl::OUString const& aName,
                            uno::Type const& aType,
                            configuration::Attributes _aAttrs)
{
    OSL_ENSURE(aType.getTypeClass() != uno::TypeClass_VOID, "OTreeNodeFactory: Creating a value node having VOID type");
    return std::auto_ptr<ValueNode>( new ValueNode(aName, aType, _aAttrs) );
}


//= ISubtree ============================================================

std::auto_ptr<ISubtree> OTreeNodeFactory::createGroupNode(
                            rtl::OUString const& aName,
                            configuration::Attributes _aAttrs)
{
    return std::auto_ptr<ISubtree>( new Subtree(aName, _aAttrs) );
}


//-----------------------------------------------

std::auto_ptr<ISubtree> OTreeNodeFactory::createSetNode(
                            rtl::OUString const& aName,
                            rtl::OUString const& _rTemplateName,
                            rtl::OUString const& _rTemplateModule,
                            configuration::Attributes _aAttrs)
{
    return std::auto_ptr<ISubtree>( new Subtree(aName, _rTemplateName, _rTemplateModule, _aAttrs) );
}

//-----------------------------------------------

} // namespace configmgr


