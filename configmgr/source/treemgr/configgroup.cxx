/*************************************************************************
 *
 *  $RCSfile: configgroup.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jb $ $Date: 2001-06-20 20:34:36 $
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

#include "configgroup.hxx"
#include "valueref.hxx"
#include "anynoderef.hxx"
#include "nodechange.hxx"
#include "nodechangeimpl.hxx"
#include "treeimpl.hxx"
#include "groupnodeimpl.hxx"
#include "tracer.hxx"

#include "valuenodeimpl.hxx"

#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif

namespace configmgr
{
    namespace configuration
    {

//-----------------------------------------------------------------------------
// class GroupUpdater
//-----------------------------------------------------------------------------

GroupUpdater::GroupUpdater(Tree const& aParentTree, NodeRef const& aGroupNode, UnoTypeConverter const& xConverter)
: m_aTree(aParentTree)
, m_aNode(aGroupNode)
, m_xTypeConverter(xConverter)
{
    implValidateGroup(m_aTree,m_aNode);

    if (!TreeImplHelper::isGroup(m_aNode))
        throw Exception("INTERNAL ERROR: Group Member Update: node is not a group");
}
//-----------------------------------------------------------------------------

void GroupUpdater::implValidateNode(Tree const& aTree, ValueRef const& aNode) const
{
    if (!aNode.isValid())
        throw Exception("INTERNAL ERROR: Group Member Update: Unexpected NULL node");

    if (!aTree.isValidNode(aNode))
        throw Exception("INTERNAL ERROR: Group Member Update: changed node does not match tree");

    if (!aTree.getAttributes(aNode).bWritable)
        throw ConstraintViolation( "Group Member Update: Node is read-only !" );

}
//-----------------------------------------------------------------------------

/** a helper that gets the UNO <type scope='com::sun::star::uno'>Type</type>
    for a UNO <type scope='com::sun::star::uno'>Any</type>.
*/
static inline UnoType getUnoAnyType()
{
    UnoAny const * const selectAny = 0;
    return ::getCppuType(selectAny);
}
//-----------------------------------------------------------------------------

bool convertCompatibleValue(UnoTypeConverter const& xTypeConverter, uno::Any& rConverted, UnoAny const& rNewValue, UnoType const& rTargetType)
{
    if (rTargetType == rNewValue.getValueType()
        || rTargetType == getUnoAnyType())
    {
        rConverted = rNewValue;
        return true;
    }

    if (xTypeConverter.is())
    try
    {
        rConverted = xTypeConverter->convertTo(rNewValue,rTargetType);
    }
    catch(uno::RuntimeException&) { throw; }
    catch(css::lang::IllegalArgumentException&)
    {
        // try to do more conversion here ?!
        return false;
    }
    catch(css::script::CannotConvertException&)
    {
        // try to do more conversion here ?!

        // throw a WrappedUnoException here ?!
        return false;
    }
    catch(uno::Exception&)
    {
        OSL_ENSURE(sal_False, "ValueUpdater::convertValue : generic exception ... thought we caught all allowed exceptions !");
        // try to do more conversion here ?!
        return false;
    }

    return true;
}


//-----------------------------------------------------------------------------

UnoAny GroupUpdater::implValidateValue(Tree const& aTree, ValueRef const& aNode, UnoAny const& aValue) const
{
    if (!aValue.hasValue())
    {
        if (!aTree.getAttributes(aNode).bNullable)
        {
            rtl::OString sError("Group Member Update: Node (");
            sError += OUSTRING2ASCII(aTree.getName(aNode).toString());
            sError += ") is not nullable !";
            throw ConstraintViolation( sError );
        }
    }

    UnoType aValueType  = aValue.getValueType();
    UnoType aTargetType = aTree.getUnoType(aNode);

    OSL_ASSERT(aTargetType.getTypeClass() != uno::TypeClass_VOID);
    OSL_ASSERT(aTargetType.getTypeClass() != uno::TypeClass_INTERFACE); // on a value node ??
    OSL_ASSERT( aValueType.getTypeClass() != uno::TypeClass_ANY);

    UnoAny aRet;

    if (aValueType == aTargetType)
    { aRet = aValue; }

    else if (aTargetType == getUnoAnyType())
    { aRet = aValue; } // OK - any type

    else if (aValueType.getTypeClass() == uno::TypeClass_VOID)
        OSL_ASSERT(!aRet.hasValue()); // legality has been checked elsewhere

    else if (!convertCompatibleValue(m_xTypeConverter, aRet, aValue,aTargetType))
        throw TypeMismatch(aValueType.getTypeName(), aTargetType.getTypeName(), " cannot set incompatible value");


    return aRet;
}
//-----------------------------------------------------------------------------

void GroupUpdater::implValidateTree(Tree const& aTree) const
{
    if (aTree.isEmpty())
        throw Exception("INTERNAL ERROR: Group Member Update: Unexpected NULL tree");

    // check for proper nesting
    TreeHolder const aParentTree = TreeImplHelper::impl(m_aTree);
    for(TreeHolder aTestTree =  TreeImplHelper::impl(aTree);
        aTestTree != aParentTree;           // search this as ancestor tree
        aTestTree = aTestTree->getContextTree() )
    {
        if (!aTestTree.isValid()) // no more trees to look for
            throw Exception("INTERNAL ERROR: Group Member Update: improper tree relationship");
    }
}
//-----------------------------------------------------------------------------

void GroupUpdater::implValidateGroup(Tree const& aTree, NodeRef const& aNode) const
{
    implValidateTree(aTree);

    if (!aNode.isValid())
        throw Exception("INTERNAL ERROR: Group Member Update: Unexpected NULL node");

    if (!aTree.isValidNode(aNode))
        throw Exception("INTERNAL ERROR: Group Member Update: node does not match tree");
}
//-----------------------------------------------------------------------------

NodeChange GroupUpdater::validateSetDefault(ValueRef const& aValueNode)
{
    implValidateNode(m_aTree, aValueNode);

    if (!m_aTree.getAttributes(aValueNode).bDefaultable)
        throw ConstraintViolation( "Group Member Update: Node may not be default !" );

    //if (!TreeImplHelper::member_node(aValueNode).canGetDefaultValue())
    //  m_aTree.ensureDefaults();

    if (!TreeImplHelper::member_node(aValueNode).canGetDefaultValue())
        throw Exception("INTERNAL ERROR: Group Member Update: default value is not available" );

    // now build the specific change
    std::auto_ptr<ValueChangeImpl> pChange( new ValueResetImpl() );

    pChange->setTarget(TreeImplHelper::impl(m_aTree), TreeImplHelper::parent_offset(aValueNode), m_aTree.getName(aValueNode));

    return NodeChange(pChange.release());
}
//-----------------------------------------------------------------------------

NodeChange GroupUpdater::validateSetDefault(AnyNodeRef const& aNode)
{
    if ( aNode.isNode() )
    {
        OSL_ENSURE(!m_aTree.getAttributes(aNode).bDefaultable,"Inner Node marked as defaultable");
        throw ConstraintViolation( "Group Member Update: Inner Node may not be set to default !" );
    }

    return this->validateSetDefault( aNode.toValue() );
}
//-----------------------------------------------------------------------------

NodeChange GroupUpdater::validateSetValue(ValueRef const& aValueNode, UnoAny const& newValue )
{
    implValidateNode(m_aTree, aValueNode);
    UnoAny aNewValue = implValidateValue(m_aTree, aValueNode, newValue);

    // now build the specific change
    std::auto_ptr<ValueChangeImpl> pChange( new ValueReplaceImpl(aNewValue) );

    pChange->setTarget(TreeImplHelper::impl(m_aTree), TreeImplHelper::parent_offset(aValueNode),m_aTree.getName(aValueNode));

    return NodeChange(pChange.release());
}
//-----------------------------------------------------------------------------

NodeChange GroupUpdater::validateSetDeepValue(  Tree const& aNestedTree, ValueRef const& aNestedNode,
                                                RelativePath const& aRelPath,UnoAny const& newValue)
{
    implValidateTree(aNestedTree);
    implValidateNode(aNestedTree, aNestedNode);
    UnoAny aNewValue = implValidateValue(aNestedTree, aNestedNode, newValue);

    // now build the specific change
    std::auto_ptr<DeepValueReplaceImpl> pChange( new DeepValueReplaceImpl(aRelPath, aNewValue) );

    pChange->setBaseContext(TreeImplHelper::impl(m_aTree), TreeImplHelper::offset(m_aNode));
    pChange->setTarget(TreeImplHelper::impl(aNestedTree), TreeImplHelper::parent_offset(aNestedNode),aNestedTree.getName(aNestedNode));

    return NodeChange(pChange.release());
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
    }
}

