/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: treeiterators.hxx,v $
 * $Revision: 1.4 $
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

#ifndef CONFIGMGR_TREEITERATORS_HXX_
#define CONFIGMGR_TREEITERATORS_HXX_

#include "tree.hxx"
#include <com/sun/star/beans/Property.hpp>
#include <rtl/ustring.hxx>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

// .......................................................................
namespace configmgr
{
// .......................................................................
    namespace configapi
    {
        // ===================================================================
        // = CollectNodeNames
        // ===================================================================
        class CollectNodeNames :  public configuration::NodeVisitor
        {
        protected:
            std::vector<rtl::OUString>  m_aList;

        public:
            CollectNodeNames() { }

            virtual Result handle(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode); // NodeVisitor
            virtual Result handle(rtl::Reference< configuration::Tree > const& aTree, configuration::ValueRef const& aNode); // NodeVisitor

            std::vector<rtl::OUString> const& list() const { return m_aList; }
        };

        // ===================================================================
        // = CollectPropertyInfo
        // ===================================================================
        class CollectPropertyInfo :  public configuration::NodeVisitor
        {
        protected:
            std::vector<com::sun::star::beans::Property>    m_aList;

        public:
            CollectPropertyInfo() { }

            virtual Result handle(rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode); // NodeVisitor
            virtual Result handle(rtl::Reference< configuration::Tree > const& aTree, configuration::ValueRef const& aNode); // NodeVisitor

            std::vector<com::sun::star::beans::Property> const& list() const { return m_aList; }
        };
    }
// .......................................................................
}   // namespace configmgr
// .......................................................................

#endif // _CONFIGMGR_TREEITERATORS_HXX_

