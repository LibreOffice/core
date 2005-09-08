/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mergedcomponentdata.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:33:11 $
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

/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */

#ifndef CONFIGMGR_BACKEND_MERGEDCOMPONENTDATA_HXX
#define CONFIGMGR_BACKEND_MERGEDCOMPONENTDATA_HXX

#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif

#include <com/sun/star/configuration/backend/TemplateIdentifier.hpp>

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    class OTreeNodeFactory;
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        namespace backenduno = ::com::sun::star::configuration::backend;

        using  backenduno::TemplateIdentifier;

        using ::rtl::OUString;

// -----------------------------------------------------------------------------

        class MergedComponentData
        {
        public:
            MergedComponentData();
            ~MergedComponentData();

            bool hasSchema()    const;
            bool hasTemplates() const;
            bool hasTemplate(OUString const & _aTemplateName) const;

            OUString getTemplateAccessor (TemplateIdentifier const & _aTemplateName) const;

            ISubtree const * getSchemaTree() const     { return m_pSchemaTree.get(); }
            ISubtree const * getTemplatesTree() const  { return m_pTemplatesTree.get(); }
            ISubtree const * findTemplate(OUString const & _aTemplateName) const;

            ISubtree * getSchemaTree()      { return m_pSchemaTree.get(); }
            ISubtree * getTemplatesTree()   { return m_pTemplatesTree.get(); }

            std::auto_ptr<INode> instantiateTemplate(OUString const & _aName, OUString const & _aTemplateName) const;

            void clear();

            ISubtree  * setSchemaRoot(std::auto_ptr<ISubtree>  _aSchemaRoot);
            ISubtree  * addTemplate(std::auto_ptr<ISubtree>  _aNode, TemplateIdentifier const & aTemplate);
            void  setTemplatesTree(std::auto_ptr<ISubtree>  _aTemplateTree);

            std::auto_ptr<ISubtree> extractSchemaTree();
            std::auto_ptr<ISubtree> extractTemplatesTree();
            std::auto_ptr<INode>    extractTemplateNode(OUString const & _aTemplateName);
        private:
            std::auto_ptr<ISubtree> m_pSchemaTree;
            std::auto_ptr<ISubtree> m_pTemplatesTree;

        };
// -----------------------------------------------------------------------------

    } // namespace backend
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




