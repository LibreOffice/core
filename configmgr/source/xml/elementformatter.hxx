/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: elementformatter.hxx,v $
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

#ifndef CONFIGMGR_XML_ELEMENTFORMATTER_HXX
#define CONFIGMGR_XML_ELEMENTFORMATTER_HXX

#include "elementinfo.hxx"
#include <com/sun/star/xml/sax/XAttributeList.hpp>

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

