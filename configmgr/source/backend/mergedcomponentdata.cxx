/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mergedcomponentdata.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:32:58 $
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

#include "mergedcomponentdata.hxx"

#ifndef CONFIGMGR_TREE_NODEFACTORY_HXX
#include "treenodefactory.hxx"
#endif

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

OUString MergedComponentData::getTemplateAccessor (TemplateIdentifier const & _aTemplateName) const
{
    return _aTemplateName.Name;
}
// -----------------------------------------------------------------------------

bool MergedComponentData::hasTemplate(OUString const & _aTemplateName) const
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

std::auto_ptr<INode> MergedComponentData::extractTemplateNode(OUString const & _aTemplateName)
{
    if (m_pTemplatesTree.get() == NULL)
        return std::auto_ptr<INode>();

    return m_pTemplatesTree->removeChild(_aTemplateName);
}
// -----------------------------------------------------------------------------

ISubtree const * MergedComponentData::findTemplate(OUString const & _aTemplateName) const
{
    INode const * pTemplateNode = m_pTemplatesTree->getChild(_aTemplateName);

    ISubtree const * pTemplateTree = pTemplateNode ? pTemplateNode->asISubtree() : NULL;

    OSL_ENSURE(pTemplateTree || !pTemplateNode, "ERROR: Template is not a subtree");

    return pTemplateTree;
}
// -----------------------------------------------------------------------------

std::auto_ptr<INode>    MergedComponentData::instantiateTemplate(OUString const & _aName, OUString const & _aTemplateName) const
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
ISubtree  * MergedComponentData::addTemplate(std::auto_ptr<ISubtree>  _aNode, TemplateIdentifier const & aTemplate)
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
