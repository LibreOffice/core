/*************************************************************************
 *
 *  $RCSfile: elementformatter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jb $ $Date: 2002-05-28 15:42:24 $
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

#ifndef CONFIGMGR_XML_ELEMENTFORMATTER_HXX
#define CONFIGMGR_XML_ELEMENTFORMATTER_HXX

#ifndef CONFIGMGR_XML_ELEMENTINFO_HXX
#include "elementinfo.hxx"
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#include <rtl/ref.hxx>

namespace configmgr
{
// -----------------------------------------------------------------------------
    class AttributeListImpl;
// -----------------------------------------------------------------------------
    namespace xml
    {
// -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace sax       = ::com::sun::star::xml::sax;

        using rtl::OUString;

// -----------------------------------------------------------------------------
        class ElementFormatter
        {
        public:
            typedef uno::Reference< sax::XAttributeList > SaxAttributeList;
            typedef ElementInfo::FlagsType FlagsType;
        public:
            ElementFormatter();
            ~ElementFormatter();

            /// reset the formatter for a new document
            void reset();

            /// resets the formatter for a new element type
            void prepareElement(ElementInfo const& _aInfo);

            /// resets the formatter for a new element type
            void prepareSimpleElement(ElementType::Enum _eType);

            /// sets the instantiated type of a set item,
            void addInstanceType(OUString const & _aElementType, OUString const & _aElementTypeModule);

            /// retrieve element type and associated module name of a set,
            void addPropertyValueType(uno::Type const& _aType);

            /// add a language for the current element
            void addLanguage(OUString const & _sLanguage);

            /// adds a value attribute to the attribute list
            void addIsNull(bool _bIsNull = true);

            /// adds a value attribute to the attribute list
            void addSeparator(OUString const& _sSeparator);

            /// retrieve the tag to use for the current element
            OUString getElementTag() const;

            /// retrieve the attributes to use for the current element
            SaxAttributeList getElementAttributes() const;

            /// retrieve the attributes to use for an element with associated component
        private:
            void addNamespaces();
            /// sets an attributes for a node
            void addName(OUString const & _aName);
            /// sets attributes for nodes from the flags
            void addNodeFlags(FlagsType _eFlags);
            /// sets attributes for nodes from the flags
            void addOperation(Operation::Enum _eOp);
            /// sets attributes for nodes from the flags
            void maybeAddFlag(FlagsType _eFlags, FlagsType _eSelect,
                                OUString const & _anAttributeName, bool _bValue = true);

            /// sets attributes for nodes
            void addAttribute(OUString const & _anAttributeName, OUString const & _aValue);
            void addAttribute(OUString const & _anAttributeName, bool _bValue);

        private:
            ElementType::Enum                   m_aElementType;
            rtl::Reference<AttributeListImpl>   m_xAttributes;
        };
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif

