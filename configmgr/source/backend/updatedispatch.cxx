/*************************************************************************
 *
 *  $RCSfile: updatedispatch.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:30:47 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "updatedispatch.hxx"

#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif
#ifndef INCLUDED_SHARABLE_NODE_HXX
#include "node.hxx"
#endif
#ifndef CONFIGMGR_TREEACCESSOR_HXX
#include "treeaccessor.hxx"
#endif
#ifndef CONFIGMGR_VALUENODEACCESS_HXX
#include "valuenodeaccess.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEACCESS_HXX
#include "groupnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_SETNODEACCESS_HXX
#include "setnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_MATCHLOCALE_HXX
#include "matchlocale.hxx"
#endif

#include <drafts/com/sun/star/configuration/backend/XUpdateHandler.hpp>
#include <drafts/com/sun/star/configuration/backend/NodeAttribute.hpp>

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------

UpdateDispatcher::UpdateDispatcher(UpdateHandler const & _xUpdateHandler, OUString const & _aLocale)
: m_xUpdateHandler(_xUpdateHandler)
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
        OUString sMsg( RTL_CONSTASCII_USTRINGPARAM("ERROR: Cannot dispatch update - no handler found") );
        throw uno::RuntimeException(sMsg,NULL);
    }

    OSL_PRECOND( !_aRootPath.isRoot(), "Cannot apply update, where root is outside a component" );

#ifdef CFG_UPDATE_CONTEXT_IS_ROOT
    this->startUpdate(_aRootPath);
    this->applyToChildren(_anUpdate);
    this->endUpdate();
#else
    this->startUpdate(_aRootPath.getParentPath());
    this->applyToChange(_anUpdate);
    this->endUpdate();
#endif

}
// -----------------------------------------------------------------------------

void UpdateDispatcher::startUpdate(configuration::AbsolutePath const & _aContextPath)
{
//    OUString sContext = _aContextPath.isRoot() ? OUString() : _aContextPath.toString();
    OUString sContext = _aContextPath.toString();
    m_xUpdateHandler->startUpdate(sContext);
    m_bInValueSet = false;
    m_bInLocalizedValues = false;
    m_aElementName = OUString();
}
// -----------------------------------------------------------------------------

void UpdateDispatcher::endUpdate()
{
    m_xUpdateHandler->endUpdate();
}
// -----------------------------------------------------------------------------

void UpdateDispatcher::handle(ValueChange const& aValueNode)
{
    // special case: doing members of a localized property (as set)
    if (m_bInLocalizedValues)
    {
        OUString aLocale = aValueNode.getNodeName();

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
                                                nAttr, nAttrMask );

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
    data::TreeSegment aAddedTree = aAddNode.getNewTree();

    OSL_ENSURE(aAddedTree.is(), "AddNode has no new data -> cannot add anything");

    OSL_ENSURE( !aAddedTree.is() || (m_bInValueSet == aAddedTree.getSegmentRootNode()->isValue()),
                "Found added subtree in value set (extensible group)\n" );

    this->visitTree( aAddedTree.getTreeAccess() );
}
// -----------------------------------------------------------------------------

void UpdateDispatcher::handle(RemoveNode const& aRemoveNode)
{
    OSL_ENSURE( !m_bInLocalizedValues, "UpdateDispatcher: Removing values for a specific locale is currently not supported");

    data::TreeSegment aRemovedTree = aRemoveNode.getRemovedTree();

    OSL_ENSURE( !aRemovedTree.is() || (m_bInValueSet == aRemovedTree.getSegmentRootNode()->isValue()),
                "Found removed subtree in value set (extensible group)\n" );

    if (m_bInValueSet)
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
                                            nAttr, nAttrMask );

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

data::SetVisitor::Result UpdateDispatcher::handle(data::ValueNodeAccess const& _aNode)
{
    OUString aName;

    // special case: doing members of a localized property (as set)
    if (m_bInLocalizedValues)
    {
        // the node name is the locale
        OUString aLocale;
        OSL_VERIFY(testReplacedAndGetName(_aNode,aLocale)); // "Adding a localized subvalue but not as root of element tree"

        if (aLocale.getLength() && aLocale != localehelper::getDefaultLocale())
        {
            m_xUpdateHandler->setPropertyValueForLocale( _aNode.getValue(), aLocale );
        }
        else
        {
            m_xUpdateHandler->setPropertyValue( _aNode.getValue() );
        }
    }
    else if (testReplacedAndGetName(_aNode,aName)&& (_aNode.getAttributes().isRemovable()) ) // we must be inside a set of values
    {

        OSL_ENSURE(!_aNode.isLocalized(), "UpdateDispatcher: Cannot add a localized value in a layer .");

        sal_Int16 nAttr = getUpdateAttributes(_aNode.getAttributes(),true);

        if (!_aNode.isNull())
        {
            m_xUpdateHandler->addOrReplacePropertyWithValue( aName,
                                                             nAttr,
                                                             _aNode.getValue());
        }
        else
        {
            m_xUpdateHandler->addOrReplaceProperty( aName,
                                                    nAttr,
                                                    _aNode.getValueType());
        }
    }
    else // normal case: updating a single property //Inserting set
    {
         sal_Int16 nAttr     = getUpdateAttributes(_aNode.getAttributes(),false);
        sal_Int16 nAttrMask = getUpdateAttributeMask(_aNode.getAttributes());

        m_xUpdateHandler->modifyProperty( aName, nAttr, nAttrMask );

        if (_aNode.isLocalized() && m_aLocale.getLength())
        {
            m_xUpdateHandler->setPropertyValueForLocale( _aNode.getValue(), m_aLocale );
        }
        else
        {
            m_xUpdateHandler->setPropertyValue( _aNode.getValue() );
        }

        m_xUpdateHandler->endProperty();
    }
    return CONTINUE;
}
// -----------------------------------------------------------------------------

data::SetVisitor::Result UpdateDispatcher::handle(data::GroupNodeAccess const& _aNode)
{
    OSL_ENSURE( !m_bInLocalizedValues, "UpdateDispatcher: A localized value cannot be a complete group");

    OUString aName;

    if ( testReplacedAndGetName(_aNode,aName) )
    {
        sal_Int16 nAttr = getUpdateAttributes(_aNode.getAttributes(),true);

        m_xUpdateHandler->addOrReplaceNode( aName, nAttr );

        this->visitChildren(_aNode);

        m_xUpdateHandler->endNode();
    }
    else
    {
        sal_Int16 nAttr     = getUpdateAttributes(_aNode.getAttributes(),false);
        sal_Int16 nAttrMask = getUpdateAttributeMask(_aNode.getAttributes());

        m_xUpdateHandler->modifyNode( aName, nAttr, nAttrMask, false );

        this->visitChildren(_aNode);

        m_xUpdateHandler->endNode();
    }
    return CONTINUE;
}
// -----------------------------------------------------------------------------

data::SetVisitor::Result UpdateDispatcher::handle(data::SetNodeAccess const& _aNode)
{
    OSL_ENSURE( !m_bInLocalizedValues, "UpdateDispatcher: A localized value cannot be a complete set");

    OUString aName;

    if ( testReplacedAndGetName(_aNode,aName) )
    {
        OSL_ENSURE( !_aNode.isLocalizedValueSetNode(), "UpdateDispatcher: Cannot add a localized value in a layer." );

        sal_Int16 nAttr     = getUpdateAttributes(_aNode.getAttributes(),true);

        m_xUpdateHandler->addOrReplaceNode( aName, nAttr );

        this->visitElements(_aNode);

        m_xUpdateHandler->endNode();
    }
    else
    {
        sal_Int16 nAttr     = getUpdateAttributes(_aNode.getAttributes(),false);
        sal_Int16 nAttrMask = getUpdateAttributeMask(_aNode.getAttributes());

        if (_aNode.isLocalizedValueSetNode())
        {
            m_xUpdateHandler->modifyProperty( aName, nAttr, nAttrMask );

            m_bInLocalizedValues = true;
            this->visitElements(_aNode);
            m_bInLocalizedValues = false;

            m_xUpdateHandler->endProperty();
        }

        else
        {
            m_xUpdateHandler->modifyNode( aName, nAttr, nAttrMask, false );

            this->visitElements(_aNode);

            m_xUpdateHandler->endNode();
        }
    }
    return CONTINUE;
}
// -----------------------------------------------------------------------------

bool UpdateDispatcher::testReplacedAndGetName(data::NodeAccessRef const & _aNode, OUString & _aName)
{
    if (m_aElementName.getLength())
    {
        OSL_ENSURE( _aNode.isLocalRoot(), "ERROR - UpdateDispatcher: Found orphaned 'element' name for inner node");
        _aName = m_aElementName;
        m_aElementName = OUString();
        return true;
    }
    else
    {
        OSL_ENSURE(!_aNode.isLocalRoot(), "ERROR - UpdateDispatcher: Found no 'element' name for fragment root node");
        _aName = _aNode.getName().toString();
        return false;
    }
    // return _aNode.isLocalRoot();
}
// -----------------------------------------------------------------------------

data::SetVisitor::Result UpdateDispatcher::handle(data::TreeAccessor const& _aElement)
{
    m_aElementName = _aElement.getName().toString();
    Result aRes = SetVisitor::handle(_aElement); // dispatch to root node
    m_aElementName = OUString(); // clear - just to be safe
    return aRes;
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

sal_Int16 UpdateDispatcher::getUpdateAttributeMask(node::Attributes const & _aAttributes)
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
