/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mergedcomponentdata.cxx,v $
 * $Revision: 1.8 $
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

#include "mergedcomponentdata.hxx"
#include "treenodefactory.hxx"

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------

MergedComponentData::MergedComponentData(  )
: m_pSchemaTree()
, m_pTemplatesTree()
{

}
// -----------------------------------------------------------------------------

MergedComponentData::~MergedComponentData(  )
{

}
// -----------------------------------------------------------------------------

void MergedComponentData::clear(  )
{
    m_pTemplatesTree.reset();
    m_pSchemaTree.reset();
}
// -----------------------------------------------------------------------------

bool MergedComponentData::hasSchema()const
{
    return m_pSchemaTree.get() != NULL;
}
// -----------------------------------------------------------------------------


bool MergedComponentData::hasTemplates() const
{
    return m_pTemplatesTree.get() != NULL;
}
// -----------------------------------------------------------------------------

rtl::OUString MergedComponentData::getTemplateAccessor (backenduno::TemplateIdentifier const & _aTemplateName) const
{
    return _aTemplateName.Name;
}
// -----------------------------------------------------------------------------

bool MergedComponentData::hasTemplate(rtl::OUString const & _aTemplateName) const
{
    return m_pTemplatesTree.get() != NULL &&
            m_pTemplatesTree->getChild( _aTemplateName ) != NULL;
}
// -----------------------------------------------------------------------------

std::auto_ptr<ISubtree> MergedComponentData::extractSchemaTree()
{
    return m_pSchemaTree;
}
// -----------------------------------------------------------------------------

std::auto_ptr<ISubtree> MergedComponentData::extractTemplatesTree()
{
    return m_pTemplatesTree;
}
// -----------------------------------------------------------------------------

std::auto_ptr<INode> MergedComponentData::extractTemplateNode(rtl::OUString const & _aTemplateName)
{
    if (m_pTemplatesTree.get() == NULL)
        return std::auto_ptr<INode>();

    return m_pTemplatesTree->removeChild(_aTemplateName);
}
// -----------------------------------------------------------------------------

ISubtree const * MergedComponentData::findTemplate(rtl::OUString const & _aTemplateName) const
{
    INode const * pTemplateNode = m_pTemplatesTree->getChild(_aTemplateName);

    ISubtree const * pTemplateTree = pTemplateNode ? pTemplateNode->asISubtree() : NULL;

    OSL_ENSURE(pTemplateTree || !pTemplateNode, "ERROR: Template is not a subtree");

    return pTemplateTree;
}
// -----------------------------------------------------------------------------

std::auto_ptr<INode>    MergedComponentData::instantiateTemplate(rtl::OUString const & _aName, rtl::OUString const & _aTemplateName) const
{
    if (INode const * pTemplateNode = m_pTemplatesTree->getChild(_aTemplateName))
    {
        std::auto_ptr<INode> aResult = pTemplateNode->clone();
        aResult->setName(_aName);
        return aResult;
    }
    else
    {
        return std::auto_ptr<INode>();
    }
}
// -----------------------------------------------------------------------------

ISubtree  * MergedComponentData::setSchemaRoot(std::auto_ptr<ISubtree>  _aSchemaRoot)
{
    OSL_PRECOND(_aSchemaRoot.get(),"ERROR: Setting a NULL schema root.");
    OSL_PRECOND(!hasSchema(),"ERROR: Schema root already set");

    m_pSchemaTree = _aSchemaRoot;

    return m_pSchemaTree.get();
}
// -----------------------------------------------------------------------------
void  MergedComponentData::setTemplatesTree(std::auto_ptr<ISubtree>  _aTemplateTree)
{
    OSL_PRECOND(!hasTemplates(),"ERROR: Template Tree already set");
    m_pTemplatesTree = _aTemplateTree;
}
// -----------------------------------------------------------------------------
ISubtree  * MergedComponentData::addTemplate(std::auto_ptr<ISubtree>  _aNode, backenduno::TemplateIdentifier const & aTemplate)
{
    OSL_PRECOND(_aNode.get(), "ERROR: Adding a NULL template");

    if (!m_pTemplatesTree.get())
    {
        m_pTemplatesTree = getDefaultTreeNodeFactory().createGroupNode( aTemplate.Component, node::Attributes() );
    }
    else
    {
        OSL_ENSURE(m_pTemplatesTree->getName().equals(aTemplate.Component),
                    "Template Component names do not match");
    }

    return m_pTemplatesTree->addChild( base_ptr(_aNode) )->asISubtree();
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace backend

// -------------------------------------------------------------------------
} // namespace configmgr
