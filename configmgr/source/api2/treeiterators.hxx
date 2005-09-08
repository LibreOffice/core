/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treeiterators.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:22:17 $
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

#ifndef CONFIGMGR_TREEITERATORS_HXX_
#define CONFIGMGR_TREEITERATORS_HXX_

#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

// .......................................................................
namespace configmgr
{
// .......................................................................
    using rtl::OUString;

    namespace configapi
    {
        // ===================================================================
        // = CollectNodeNames
        // ===================================================================
        class CollectNodeNames :  public configuration::NodeVisitor
        {
        public:
            typedef std::vector<OUString> NameList;

        protected:
            NameList    m_aList;

        public:
            CollectNodeNames() { }

            virtual Result handle(configuration::Tree const& aTree, configuration::NodeRef const& aNode); // NodeVisitor
            virtual Result handle(configuration::Tree const& aTree, configuration::ValueRef const& aNode); // NodeVisitor

            NameList const& list() const { return m_aList; }
        };

        // ===================================================================
        // = CollectPropertyInfo
        // ===================================================================
        class CollectPropertyInfo :  public configuration::NodeVisitor
        {
        public:
            typedef com::sun::star::beans::Property Property;
            typedef std::vector<Property> PropertyList;

        protected:
            PropertyList    m_aList;

        public:
            CollectPropertyInfo() { }

            virtual Result handle(configuration::Tree const& aTree, configuration::NodeRef const& aNode); // NodeVisitor
            virtual Result handle(configuration::Tree const& aTree, configuration::ValueRef const& aNode); // NodeVisitor

            PropertyList const& list() const { return m_aList; }
        };
    }
// .......................................................................
}   // namespace configmgr
// .......................................................................

#endif // _CONFIGMGR_TREEITERATORS_HXX_

