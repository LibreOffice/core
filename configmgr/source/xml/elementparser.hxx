/*************************************************************************
 *
 *  $RCSfile: elementparser.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:43:45 $
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

#ifndef CONFIGMGR_XML_ELEMENTPARSER_HXX
#define CONFIGMGR_XML_ELEMENTPARSER_HXX

#ifndef CONFIGMGR_XML_ELEMENTINFO_HXX
#include "elementinfo.hxx"
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace xml
    {
// -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace sax       = ::com::sun::star::xml::sax;

        using rtl::OUString;

// -----------------------------------------------------------------------------
        class ElementParser
        {
        public:
            typedef uno::Reference< sax::XAttributeList > SaxAttributeList;
        public:
            ElementParser()
            {}

            /// reset the parser for a new document
            void reset()
            {}

            /// retrieve the (almost) complete information for an element
            ElementInfo parseElementInfo(OUString const& _sTag, SaxAttributeList const& _xAttribs) const;

            /// retrieve the node name for an element
            ElementType::Enum getNodeType(OUString const& _sTag, SaxAttributeList const& xAttribs) const;

            /// retrieve the node name for an element
            ElementName getName(OUString const& _sTag, SaxAttributeList const& xAttribs, ElementType::Enum eType = ElementType::unknown) const;

            /// query whether the node has an operation
            Operation::Enum getOperation(SaxAttributeList const& xAttribs, ElementType::Enum _eType) const;

            /// retrieve the language (if any) stored in the attribute list
            bool getLanguage(SaxAttributeList const& xAttribs, OUString & _rsLanguage) const;

            /// reads attributes for nodes from the attribute list
            ElementInfo::FlagsType getNodeFlags(SaxAttributeList const& xAttribs, ElementType::Enum _eType) const;

            /// retrieve element type and associated module name of a set,
            bool getSetElementType(SaxAttributeList const& _xAttribs, OUString& _rsElementType, OUString& _rsElementTypeModule) const;

            /// retrieve the instance type and associated module name of a instance,
            bool getInstanceType(SaxAttributeList const& _xAttribs, OUString& _rsElementType, OUString& _rsElementTypeModule) const;

            /// retrieve the component for an import or uses element,
            bool getImportComponent(SaxAttributeList const& _xAttribs, OUString& _rsComponent) const;

            /// retrieve element type and associated module name of a set,
            uno::Type getPropertyValueType(SaxAttributeList const& _xAttribs) const;

            /// reads a value attribute from the attribute list
            bool isNull(SaxAttributeList const& _xAttribs) const;

            /// reads a value attribute from the attribute list
            OUString getSeparator(SaxAttributeList const& _xAttribs) const;

        // low-level internal methods

            /// checks for presence of a boolean attribute and assigns its value if it exists (and is a bool)
            bool maybeGetAttribute(SaxAttributeList const& _xAttribs, OUString const& _aAttributeName, bool& _rbAttributeValue) const;

            /// checks for presence of an attribute and assigns its value if it exists
            bool maybeGetAttribute(SaxAttributeList const& _xAttribs, OUString const& _aAttributeName, OUString& _rsAttributeValue) const;

            /// assigns an attribute value or an empty string if it doesn't exist
            void alwaysGetAttribute(SaxAttributeList const& _xAttribs, OUString const& _aAttributeName, OUString& _rsAttributeValue) const;
        };
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif

