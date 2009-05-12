/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mergedcomponentdata.hxx,v $
 * $Revision: 1.6 $
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

/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */

#ifndef CONFIGMGR_BACKEND_MERGEDCOMPONENTDATA_HXX
#define CONFIGMGR_BACKEND_MERGEDCOMPONENTDATA_HXX

#include "valuenode.hxx"

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

// -----------------------------------------------------------------------------

        class MergedComponentData
        {
        public:
            MergedComponentData();
            ~MergedComponentData();

            bool hasSchema()    const;
            bool hasTemplates() const;
            bool hasTemplate(rtl::OUString const & _aTemplateName) const;

            rtl::OUString getTemplateAccessor (backenduno::TemplateIdentifier const & _aTemplateName) const;

            ISubtree const * getSchemaTree() const     { return m_pSchemaTree.get(); }
            ISubtree const * getTemplatesTree() const  { return m_pTemplatesTree.get(); }
            ISubtree const * findTemplate(rtl::OUString const & _aTemplateName) const;

            ISubtree * getSchemaTree()      { return m_pSchemaTree.get(); }
            ISubtree * getTemplatesTree()   { return m_pTemplatesTree.get(); }

            std::auto_ptr<INode> instantiateTemplate(rtl::OUString const & _aName, rtl::OUString const & _aTemplateName) const;

            void clear();

            ISubtree  * setSchemaRoot(std::auto_ptr<ISubtree>  _aSchemaRoot);
            ISubtree  * addTemplate(std::auto_ptr<ISubtree>  _aNode, backenduno::TemplateIdentifier const & aTemplate);
            void  setTemplatesTree(std::auto_ptr<ISubtree>  _aTemplateTree);

            std::auto_ptr<ISubtree> extractSchemaTree();
            std::auto_ptr<ISubtree> extractTemplatesTree();
            std::auto_ptr<INode>    extractTemplateNode(rtl::OUString const & _aTemplateName);
        private:
            std::auto_ptr<ISubtree> m_pSchemaTree;
            std::auto_ptr<ISubtree> m_pTemplatesTree;

        };
// -----------------------------------------------------------------------------

    } // namespace backend
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




