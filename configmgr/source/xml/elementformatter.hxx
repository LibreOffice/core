/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: elementformatter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:38:38 $
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

