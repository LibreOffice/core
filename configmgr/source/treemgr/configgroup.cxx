/*************************************************************************
 *
 *  $RCSfile: configgroup.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:39:39 $
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

#ifndef CONFIGMGR_CONFIGSET_HXX_
#include "configset.hxx"
#endif
#ifndef CONFIGMGR_CONFIGVALUEREF_HXX_
#include "valueref.hxx"
#endif
#ifndef CONFIGMGR_CONFIGANYNODE_HXX_
#include "anynoderef.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGE_HXX_
#include "nodechange.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGEIMPL_HXX_
#include "nodechangeimpl.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNODEIMPL_HXX_
#include "treeimpl.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEBEHAVIOR_HXX_
#include "groupnodeimpl.hxx"
#endif
#ifndef CONFIGMGR_VALUENODEBEHAVIOR_HXX_
#include "valuenodeimpl.hxx"
#endif
#ifndef CONFIGMGR_TYPECONVERTER_HXX
#include "typeconverter.hxx"
#endif

#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif

namespace configmgr
{
    namespace configuration
    {

//-----------------------------------------------------------------------------
// class GroupUpdateHelper
//-----------------------------------------------------------------------------

GroupUpdateHelper::GroupUpdateHelper(Tree const& aParentTree, NodeRef const& aGroupNode)
: m_aTree(aParentTree)
, m_aNode(aGroupNode)
{
    implValidateTree(m_aTree);
    implValidateNode(m_aTree,m_aNode);

    if (! m_aTree.getView().isGroupNode(m_aNode) )
        throw Exception("INTERNAL ERROR: Group Member Update: node is not a group");
}
//-----------------------------------------------------------------------------

void GroupUpdateHelper::implValidateTree(Tree const& aTree) const
{
    if (aTree.isEmpty())
        throw Exception("INTERNAL ERROR: Group Member Update: Unexpected NULL tree");

    typedef rtl::Reference<TreeImpl> TreeHolder;

    // check for proper nesting
    TreeHolder const aParentTree = TreeImplHelper::impl(m_aTree);
    for(TreeHolder aTestTree =  TreeImplHelper::impl(aTree);
        aTestTree != aParentTree;           // search this as ancestor tree
        aTestTree = aTestTree->getContextTree() )
    {
        if (!aTestTree.is()) // no more trees to look for
            throw Exception("INTERNAL ERROR: Group Member Update: improper tree relationship");
    }
}
//-----------------------------------------------------------------------------

void GroupUpdateHelper::implValidateNode(Tree const& aTree, NodeRef const& aNode) const
{
    if (!aNode.isValid())
        throw Exception("INTERNAL ERROR: Group Member Update: Unexpected NULL node");

    if (!aTree.isValidNode(aNode))
        throw Exception("INTERNAL ERROR: Group Member Update: node does not match tree");
}
//-----------------------------------------------------------------------------

void GroupUpdateHelper::implValidateNode(Tree const& aTree, ValueRef const& aNode) const
{
    if (!aNode.isValid())
        throw Exception("INTERNAL ERROR: Group Member Update: Unexpected NULL node");

    if (!aTree.isValidNode(aNode))
        throw Exception("INTERNAL ERROR: Group Member Update: changed node does not match tree");

    if (aTree.getAttributes(aNode).isReadonly())
        throw ConstraintViolation( "Group Member Update: Node is read-only !" );

}
//-----------------------------------------------------------------------------

void GroupUpdateHelper::validateNode(ValueRef const& aNode) const
{
    implValidateNode(m_aTree,aNode);
}
//-----------------------------------------------------------------------------

void GroupUpdateHelper::validateNode(NodeRef const& aNode) const
{
    implValidateNode(m_aTree,aNode);
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

bool isPossibleValueType(UnoType const& aValueType)
{
    switch(aValueType.getTypeClass())
    {
    case uno::TypeClass_BOOLEAN:
    case uno::TypeClass_SHORT:
    case uno::TypeClass_LONG:
    case uno::TypeClass_HYPER:
    case uno::TypeClass_DOUBLE:
    case uno::TypeClass_STRING:
        return true;

    case uno::TypeClass_SEQUENCE:
        switch(getSequenceElementType(aValueType).getTypeClass())
        {
        case uno::TypeClass_BYTE:  // scalar binary

        case uno::TypeClass_BOOLEAN:
        case uno::TypeClass_SHORT:
        case uno::TypeClass_LONG:
        case uno::TypeClass_HYPER:
        case uno::TypeClass_DOUBLE:
        case uno::TypeClass_STRING:
            return true;

        case uno::TypeClass_SEQUENCE:
            {
                uno::Sequence< uno::Sequence< sal_Int8 > > const * const forBinaryList = 0;
                return !!(aValueType == ::getCppuType(forBinaryList));
            }

        default:
            return false;
        }

    default:
        return false;
    }
}

//-----------------------------------------------------------------------------
bool convertCompatibleValue(UnoTypeConverter const& xTypeConverter, uno::Any& rConverted, UnoAny const& rNewValue, UnoType const& rTargetType)
{
    OSL_ASSERT( isPossibleValueType(rTargetType) );

    if (rTargetType == rNewValue.getValueType())
    {
        rConverted = rNewValue;
        return true;
    }

    if (xTypeConverter.is())
    try
    {
        rConverted = xTypeConverter->convertTo(rNewValue,rTargetType);

        OSL_ASSERT( rConverted.getValueType() == rTargetType );
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

//-----------------------------------------------------------------------------
// class GroupUpdater
//-----------------------------------------------------------------------------

GroupUpdater::GroupUpdater(Tree const& aParentTree, NodeRef const& aGroupNode, UnoTypeConverter const& xConverter)
: m_aHelper(aParentTree,aGroupNode)
, m_xTypeConverter(xConverter)
{
}
//-----------------------------------------------------------------------------

UnoAny GroupUpdater::implValidateValue(Tree const& aTree, ValueRef const& aNode, UnoAny const& aValue) const
{
    UnoType aValueType  = aValue.getValueType();
    UnoType aTargetType = aTree.getUnoType(aNode);

    OSL_ENSURE( aTargetType.getTypeClass() == uno::TypeClass_ANY || isPossibleValueType(aTargetType),
                "Invalid value type found on existing property" );

    OSL_ASSERT( aValueType.getTypeClass() != uno::TypeClass_ANY);

    UnoAny aRet;

    if (!aValue.hasValue())
    {
        if (!aTree.getAttributes(aNode).isNullable())
        {
            rtl::OString sError("Group Member Update: Node (");
            sError += OUSTRING2ASCII(aTree.getName(aNode).toString());
            sError += ") is not nullable !";
            throw ConstraintViolation( sError );
        }
        OSL_ASSERT( !aRet.hasValue() );
    }

    else if (aValueType == aTargetType)
    {
        aRet = aValue;
    }

    else if (aTargetType == getUnoAnyType())
    {
        if ( ! isPossibleValueType(aValueType) )
            throw TypeMismatch(aValueType.getTypeName(), aTargetType.getTypeName(), " - new property value has no legal configuration data type");

        // OK - any type
        aRet = aValue;
    }

    else
    {
        if (!convertCompatibleValue(m_xTypeConverter, aRet, aValue,aTargetType))
            throw TypeMismatch(aValueType.getTypeName(), aTargetType.getTypeName(), " cannot set incompatible value");
    }


    OSL_ASSERT( !aRet.hasValue() || isPossibleValueType(aRet.getValueType()) );

    return aRet;
}
//-----------------------------------------------------------------------------

NodeChange GroupUpdater::validateSetValue(ValueRef const& aValueNode, UnoAny const& newValue )
{
    m_aHelper.validateNode(aValueNode);

    UnoAny aNewValue = implValidateValue(m_aHelper.tree(), aValueNode, newValue);

    // now build the specific change
    std::auto_ptr<ValueChangeImpl> pChange( new ValueReplaceImpl(aNewValue) );

    NodeRef aParent = m_aHelper.tree().getParent(aValueNode);
    pChange->setTarget(
                m_aHelper.tree().getView().toGroupNode(aParent),
                m_aHelper.tree().getName(aValueNode)
            );

    return NodeChange(pChange.release());
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// helper class NodeDefaulter
//-----------------------------------------------------------------------------

namespace
{
    struct NodeDefaulter : NodeVisitor
    {
        GroupDefaulter& updater;
        NodeChanges     result;

        explicit
        NodeDefaulter(GroupDefaulter& _rUpdater) : updater(_rUpdater), result() {}

        /// do the operation on <var>aNode</var>. needs to be implemented by concrete visitor classes
        Result handle(Tree const& aTree, NodeRef const& aNode);

        /// do the operation on <var>aValue</var>. needs to be implemented by concrete visitor classes
        Result handle(Tree const& aTree, ValueRef const& aValue);

        inline void addResult(NodeChange const& aChange)
        {
            if (aChange.maybeChange())
                this->result.add(aChange);
        }
    };

    NodeVisitor::Result NodeDefaulter::handle(Tree const& , NodeRef const& aNode)
    {
        addResult( updater.validateSetToDefaultState(aNode) );
        return CONTINUE;
    }

    NodeVisitor::Result NodeDefaulter::handle(Tree const& , ValueRef const& aValue)
    {
        addResult( updater.validateSetToDefaultValue(aValue) );
        return CONTINUE;
    }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class GroupDefaulter
//-----------------------------------------------------------------------------

GroupDefaulter::GroupDefaulter(Tree const& _aParentTree, NodeRef const& _aGroupNode, DefaultProvider const& _aProvider)
: m_aHelper(_aParentTree,_aGroupNode)
, m_aDefaultProvider(_aProvider)
, m_bHasDoneSet(false)
{
}
//-----------------------------------------------------------------------------
bool GroupDefaulter::isDataAvailable(TreeRef const& _aParentTree, NodeRef const& _aGroupNode)
{
    data::Accessor aTempAccess( getRootSegment(_aParentTree) );

    Tree aTempTree(aTempAccess, _aParentTree);

    return aTempTree.areValueDefaultsAvailable(_aGroupNode);
}
//-----------------------------------------------------------------------------
bool GroupDefaulter::ensureDataAvailable(TreeRef const& _aParentTree, NodeRef const& _aGroupNode, DefaultProvider const& _aDataSource)
{
    return  isDataAvailable(_aParentTree, _aGroupNode) ||
            _aDataSource.fetchDefaultData( _aParentTree );
}
//-----------------------------------------------------------------------------

bool GroupDefaulter::isDataAvailable()
{
    return m_aHelper.tree().areValueDefaultsAvailable(m_aHelper.node());
}
//-----------------------------------------------------------------------------

NodeChange GroupDefaulter::validateSetToDefaultValue(ValueRef const& aValueNode)
{
    m_aHelper.validateNode(aValueNode);

    //if (!TreeImplHelper::member_node(aValueNode).canGetDefaultValue())
    //  m_aTree.ensureDefaults();

    if (!m_aHelper.tree().hasNodeDefault(aValueNode))
        throw Exception("INTERNAL ERROR: Group Member Update: Node has no default value" );

    // now build the specific change
    std::auto_ptr<ValueChangeImpl> pChange( new ValueResetImpl() );

    NodeRef aParent = m_aHelper.tree().getParent(aValueNode);
    pChange->setTarget(
                m_aHelper.tree().getView().toGroupNode(aParent),
                m_aHelper.tree().getName(aValueNode)
            );

    return NodeChange(pChange.release());
}
//-----------------------------------------------------------------------------

NodeChange GroupDefaulter::validateSetToDefaultState(NodeRef const& aNode)
{
    m_aHelper.validateNode(aNode);

    NodeChange aResult;

    // only works for set nodes - groups are left alone
    if ( m_aHelper.tree().getView().isSetNode(aNode) )
    {
       aResult = SetDefaulter( m_aHelper.tree(), aNode, m_aDefaultProvider ).validateSetToDefaultState();
    }

    m_bHasDoneSet = aResult.maybeChange();

    return aResult;
}
//-----------------------------------------------------------------------------

NodeChanges GroupDefaulter::validateSetAllToDefault()
{
    NodeDefaulter aDefaulter(*this);

    m_aHelper.tree().dispatchToChildren(m_aHelper.node(),aDefaulter);

    return aDefaulter.result;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
    }
}

