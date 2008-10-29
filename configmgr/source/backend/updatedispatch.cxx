/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: updatedispatch.cxx,v $
 * $Revision: 1.12 $
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

#include "treefragment.hxx"
#include "updatedispatch.hxx"
#include "configpath.hxx"
#include "node.hxx"
#include "matchlocale.hxx"

#include <com/sun/star/configuration/backend/XUpdateHandler.hpp>
#include <com/sun/star/configuration/backend/NodeAttribute.hpp>

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------

UpdateDispatcher::UpdateDispatcher(uno::Reference< backenduno::XUpdateHandler > const & _xUpdateHandler, rtl::OUString const & _aLocale)
: m_pContextPath(NULL)
, m_xUpdateHandler(_xUpdateHandler)
, m_aLocale(_aLocale)
, m_aElementName()
, m_bInValueSet(false)
, m_bInLocalizedValues(false)
{
}
// -----------------------------------------------------------------------------

UpdateDispatcher::~UpdateDispatcher()
{
}
// -----------------------------------------------------------------------------

void UpdateDispatcher::dispatchUpdate(configuration::AbsolutePath const & _aRootPath, SubtreeChange const& _anUpdate)
{
    if (!m_xUpdateHandler.is())
    {
        rtl::OUString sMsg( RTL_CONSTASCII_USTRINGPARAM("ERROR: Cannot dispatch update - no handler found") );
        throw uno::RuntimeException(sMsg,NULL);
    }

    OSL_PRECOND( !_aRootPath.isRoot(), "Cannot apply update, where root is outside a component" );

    OSL_PRECOND( m_pContextPath == NULL, "Update Dispatcher already has a context path" );
    if (!_aRootPath.getParentPath().isRoot())
    {
        OSL_ENSURE(false,"Obsolete functionality used: starting update with non-empty context");
        m_pContextPath = &_aRootPath;
    }

    this->startUpdate();
    this->applyToChange(_anUpdate);
    this->endUpdate();

    m_pContextPath = NULL;
}
// -----------------------------------------------------------------------------

void UpdateDispatcher::startUpdate()
{
    m_xUpdateHandler->startUpdate();
    m_bInValueSet = false;
    m_bInLocalizedValues = false;
    m_aElementName = rtl::OUString();

    if (m_pContextPath)
    {
        std::vector<configuration::Path::Component>::const_reverse_iterator   it = m_pContextPath->begin();
        std::vector<configuration::Path::Component>::const_reverse_iterator   stop = m_pContextPath->end();

        OSL_ASSERT(it != stop);
        --stop;

        for ( ; it != stop; ++it)
        {
            m_xUpdateHandler->modifyNode(it->getName(),0,0,false);
        }
    }
}
// -----------------------------------------------------------------------------

void UpdateDispatcher::endUpdate()
{
    if (m_pContextPath)
    {
        std::vector<configuration::Path::Component>::const_reverse_iterator   it = m_pContextPath->begin();
        std::vector<configuration::Path::Component>::const_reverse_iterator   stop = m_pContextPath->end();

        OSL_ASSERT(it != stop);
        --stop;

        for ( ; it != stop; ++it)
        {
            m_xUpdateHandler->endNode();
        }
    }
    m_xUpdateHandler->endUpdate();
}
// -----------------------------------------------------------------------------

void UpdateDispatcher::handle(ValueChange const& aValueNode)
{
    // special case: doing members of a localized property (as set)
    if (m_bInLocalizedValues)
    {
        rtl::OUString aLocale = aValueNode.getNodeName();

        if (aLocale.getLength())
        {
            if ( aValueNode.isToDefault() )
                m_xUpdateHandler->resetPropertyValueForLocale( aLocale );
             else
                m_xUpdateHandler->setPropertyValueForLocale( aValueNode.getNewValue(), aLocale );
        }
        else
        {
            if ( aValueNode.isToDefault() )
                m_xUpdateHandler->resetPropertyValue( );
             else
                m_xUpdateHandler->setPropertyValue( aValueNode.getNewValue() );
        }
        return;
    }

    // normal case: updating a single property
    switch (aValueNode.getMode())
    {
    case ValueChange::wasDefault:
        if (aValueNode.isReplacedValue())
        {

            OSL_ENSURE(m_bInValueSet, "UpdateDispatcher: Cannot add/replace a value in a nonextensible node");
            OSL_ENSURE(!aValueNode.isLocalizedValue(), "UpdateDispatcher: Cannot add a localized value in a layer");

            sal_Int16 nAttr = getUpdateAttributes(aValueNode.getAttributes(),true);

            if (aValueNode.getNewValue().hasValue())
            {
                m_xUpdateHandler->addOrReplacePropertyWithValue( aValueNode.getNodeName(),
                                                                 nAttr,
                                                                 aValueNode.getNewValue());
            }
            else
            {
                m_xUpdateHandler->addOrReplaceProperty( aValueNode.getNodeName(),
                                                        nAttr,
                                                        aValueNode.getValueType());
            }

            break;
        }
        // else fall thru to changeValue case

    case ValueChange::changeValue:
        {
            sal_Int16 nAttr     = getUpdateAttributes(aValueNode.getAttributes(),false);
            sal_Int16 nAttrMask = getUpdateAttributeMask(aValueNode.getAttributes());

            m_xUpdateHandler->modifyProperty( aValueNode.getNodeName(),
                                                nAttr, nAttrMask,
                                              aValueNode.getValueType() );

            if (aValueNode.isLocalizedValue() && m_aLocale.getLength())
            {
                m_xUpdateHandler->setPropertyValueForLocale( aValueNode.getNewValue(), m_aLocale );
            }
            else
            {
                m_xUpdateHandler->setPropertyValue( aValueNode.getNewValue() );
            }

            m_xUpdateHandler->endProperty();
        }
        break;

    case ValueChange::setToDefault:
        m_xUpdateHandler->resetProperty( aValueNode.getNodeName() );
        break;

    case ValueChange::changeDefault:
        OSL_ENSURE(false, "Illegal mode in ValueChange");
        break;

    default:
        OSL_ENSURE(false, "Illegal mode in ValueChange");
        break;
    };
}
// -----------------------------------------------------------------------------

void UpdateDispatcher::handle(AddNode const& aAddNode)
{
    rtl::Reference< data::TreeSegment > aAddedTree = aAddNode.getNewTree();

    OSL_ENSURE(aAddedTree.is(), "AddNode has no new data -> cannot add anything");

    OSL_ENSURE( ((m_bInValueSet||m_bInLocalizedValues) == aAddedTree->fragment->nodes[0].isValue()),
                "Found added subtree in value set (extensible group)\n" );

    this->visitTree(aAddedTree->fragment);
}
// -----------------------------------------------------------------------------

void UpdateDispatcher::handle(RemoveNode const& aRemoveNode)
{
    OSL_ENSURE( !m_bInLocalizedValues, "UpdateDispatcher: Removing values for a specific locale is currently not supported");

    rtl::Reference< data::TreeSegment > aRemovedTree = aRemoveNode.getRemovedTree();

    OSL_ENSURE( !aRemovedTree.is() ||
                ((m_bInValueSet||m_bInLocalizedValues) == aRemovedTree->fragment->nodes[0].isValue()),
                "Found removed subtree in value set (extensible group)\n" );

    if (m_bInLocalizedValues)
        OSL_TRACE("configmgr: UpdateDispatcher - Removing value for locale ignored");

    else if (m_bInValueSet)
        m_xUpdateHandler->removeProperty( aRemoveNode.getNodeName() );

    else
        m_xUpdateHandler->removeNode( aRemoveNode.getNodeName() );
}
// -----------------------------------------------------------------------------

void UpdateDispatcher::handle(SubtreeChange const& aSubtree)
{
    OSL_ENSURE( !m_bInLocalizedValues, "UpdateDispatcher: A localized value cannot be a complete subtree");
    OSL_ENSURE( !m_bInValueSet, "UpdateDispatcher: A dynamic property cannot be a complete subtree");

    sal_Int16 nAttr     = getUpdateAttributes(aSubtree.getAttributes(),false);
    sal_Int16 nAttrMask = getUpdateAttributeMask(aSubtree.getAttributes());

    if (isLocalizedValueSet(aSubtree))
    {
        m_xUpdateHandler->modifyProperty( aSubtree.getNodeName(),
                                            nAttr, nAttrMask,
                                            uno::Type() );

        m_bInLocalizedValues = true;
        this->applyToChildren(aSubtree);
        m_bInLocalizedValues = false;

        m_xUpdateHandler->endProperty();
    }

    else
    {
        m_xUpdateHandler->modifyNode( aSubtree.getNodeName(),
                                       nAttr, nAttrMask,
                                      aSubtree.isToDefault() );

        m_bInValueSet = isValueSet(aSubtree);
        this->applyToChildren(aSubtree);
        m_bInValueSet = false;

        m_xUpdateHandler->endNode();
    }
}
// -----------------------------------------------------------------------------

bool UpdateDispatcher::handle(sharable::ValueNode * node)
{
    rtl::OUString aName;

    // special case: doing members of a localized property (as set)
    if (m_bInLocalizedValues)
    {
        // the node name is the locale
        rtl::OUString aLocale;
        OSL_VERIFY(testReplacedAndGetName(sharable::node(node), aLocale)); // "Adding a localized subvalue but not as root of element tree"

        if (aLocale.getLength() && ! localehelper::isDefaultLanguage(aLocale))
        {
            m_xUpdateHandler->setPropertyValueForLocale(node->getValue(), aLocale);
        }
        else
        {
            m_xUpdateHandler->setPropertyValue(node->getValue());
        }
    }
    else if (testReplacedAndGetName(sharable::node(node), aName) && sharable::node(node)->getAttributes().isRemovable()) // we must be inside a set of values
    {

        OSL_ENSURE(!node->info.isLocalized(), "UpdateDispatcher: Cannot add a localized value in a layer .");

        sal_Int16 nAttr = getUpdateAttributes(sharable::node(node)->getAttributes(),true);

        if (!node->isNull())
        {
            m_xUpdateHandler->addOrReplacePropertyWithValue( aName,
                                                             nAttr,
                                                             node->getValue());
        }
        else
        {
            m_xUpdateHandler->addOrReplaceProperty( aName,
                                                    nAttr,
                                                    node->getValueType());
        }
    }
    else // normal case: updating a single property //Inserting set
    {
         sal_Int16 nAttr     = getUpdateAttributes(sharable::node(node)->getAttributes(),false);
        sal_Int16 nAttrMask = getUpdateAttributeMask(sharable::node(node)->getAttributes());

        m_xUpdateHandler->modifyProperty( aName, nAttr, nAttrMask, node->getValueType() );

        if (node->info.isLocalized() && m_aLocale.getLength())
        {
            m_xUpdateHandler->setPropertyValueForLocale(node->getValue(), m_aLocale);
        }
        else
        {
            m_xUpdateHandler->setPropertyValue(node->getValue());
        }

        m_xUpdateHandler->endProperty();
    }
    return false;
}
// -----------------------------------------------------------------------------

bool UpdateDispatcher::handle(sharable::GroupNode * node)
{
    OSL_ENSURE( !m_bInLocalizedValues, "UpdateDispatcher: A localized value cannot be a complete group");

    rtl::OUString aName;

    if ( testReplacedAndGetName(sharable::node(node), aName) )
    {
        sal_Int16 nAttr = getUpdateAttributes(sharable::node(node)->getAttributes(),true);

        m_xUpdateHandler->addOrReplaceNode( aName, nAttr );

        this->visitChildren(node);

        m_xUpdateHandler->endNode();
    }
    else
    {
        sal_Int16 nAttr     = getUpdateAttributes(sharable::node(node)->getAttributes(),false);
        sal_Int16 nAttrMask = getUpdateAttributeMask(sharable::node(node)->getAttributes());

        m_xUpdateHandler->modifyNode( aName, nAttr, nAttrMask, false );

        this->visitChildren(node);

        m_xUpdateHandler->endNode();
    }
    return false;
}
// -----------------------------------------------------------------------------

bool UpdateDispatcher::handle(sharable::SetNode * node)
{
    OSL_ENSURE( !m_bInLocalizedValues, "UpdateDispatcher: A localized value cannot be a complete set");

    rtl::OUString aName;

    if ( testReplacedAndGetName(sharable::node(node), aName) )
    {
        OSL_ENSURE( !node->info.isLocalized(), "UpdateDispatcher: Cannot add a localized value in a layer." );

        sal_Int16 nAttr     = getUpdateAttributes(sharable::node(node)->getAttributes(),true);

        m_xUpdateHandler->addOrReplaceNode( aName, nAttr );

        this->visitElements(node);

        m_xUpdateHandler->endNode();
    }
    else
    {
        sal_Int16 nAttr     = getUpdateAttributes(sharable::node(node)->getAttributes(),false);
        sal_Int16 nAttrMask = getUpdateAttributeMask(sharable::node(node)->getAttributes());

        if (node->info.isLocalized())
        {
            m_xUpdateHandler->modifyProperty( aName, nAttr, nAttrMask, uno::Type() );

            m_bInLocalizedValues = true;
            this->visitElements(node);
            m_bInLocalizedValues = false;

            m_xUpdateHandler->endProperty();
        }

        else
        {
            m_xUpdateHandler->modifyNode( aName, nAttr, nAttrMask, false );

            this->visitElements(node);

            m_xUpdateHandler->endNode();
        }
    }
    return false;
}
// -----------------------------------------------------------------------------

bool UpdateDispatcher::testReplacedAndGetName(sharable::Node * node, rtl::OUString & _aName)
{
    if (m_aElementName.getLength())
    {
        OSL_ENSURE( node->isFragmentRoot(), "ERROR - UpdateDispatcher: Found orphaned 'element' name for inner node");
        _aName = m_aElementName;
        m_aElementName = rtl::OUString();
        return true;
    }
    else
    {
        OSL_ENSURE(!node->isFragmentRoot(), "ERROR - UpdateDispatcher: Found no 'element' name for fragment root node");
        _aName = node->getName();
        return false;
    }
}
// -----------------------------------------------------------------------------

bool UpdateDispatcher::handle(sharable::TreeFragment * tree)
{
    m_aElementName = tree->getName();
    bool done = SetVisitor::handle(tree); // dispatch to root node
    m_aElementName = rtl::OUString(); // clear - just to be safe
    return done;
}
// -----------------------------------------------------------------------------

sal_Int16 UpdateDispatcher::getUpdateAttributes(node::Attributes const & _aAttributes, bool bAdded)
{
    namespace NodeAttribute = backenduno::NodeAttribute;

    // no support for post-creation attribute changes yet
    if (!bAdded)
    {
        OSL_ENSURE( getUpdateAttributeMask(_aAttributes) == 0,
                    "Incomplete support for attribute changes" );
        return 0;
    }

    sal_Int16 nResult = 0;

    if (_aAttributes.isReadonly())
        nResult = NodeAttribute::READONLY;

    if (_aAttributes.isFinalized())
        nResult |= NodeAttribute::FINALIZED;

    if (!_aAttributes.isNullable())
        nResult |= NodeAttribute::MANDATORY;

    return nResult;
}
// -----------------------------------------------------------------------------

sal_Int16 UpdateDispatcher::getUpdateAttributeMask(node::Attributes const & /*_aAttributes*/)
{
    // no support for post-creation attribute changes yet
    return 0;
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace backend

// -------------------------------------------------------------------------
} // namespace configmgr
