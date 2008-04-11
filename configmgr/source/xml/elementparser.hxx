/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: elementparser.hxx,v $
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

#ifndef CONFIGMGR_XML_ELEMENTPARSER_HXX
#define CONFIGMGR_XML_ELEMENTPARSER_HXX

#include "elementinfo.hxx"
#include "logger.hxx"
#include <com/sun/star/xml/sax/XAttributeList.hpp>

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
            Logger mLogger;
        public:
            typedef uno::Reference< sax::XAttributeList > SaxAttributeList;

            class BadValueType
            {
                OUString mMessage;
            public:
                BadValueType(OUString const & aMessage) : mMessage(aMessage) {}

                OUString message() const { return mMessage.getStr(); }
            };
        public:
            explicit
            ElementParser(Logger const & xLogger)
            : mLogger(xLogger)
            {}

            Logger const & logger() const { return mLogger; }

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
                // throw( BadValueType )

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

