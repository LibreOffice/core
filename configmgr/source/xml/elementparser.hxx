/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: elementparser.hxx,v $
 * $Revision: 1.7 $
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

// -----------------------------------------------------------------------------
        class ElementParser
        {
            Logger mLogger;
        public:
            class BadValueType
            {
                rtl::OUString mMessage;
            public:
                BadValueType(rtl::OUString const & aMessage) : mMessage(aMessage) {}

                rtl::OUString message() const { return mMessage.getStr(); }
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
            ElementInfo parseElementInfo(rtl::OUString const& _sTag, uno::Reference< sax::XAttributeList > const& _xAttribs) const;

            /// retrieve the node name for an element
            ElementType::Enum getNodeType(rtl::OUString const& _sTag, uno::Reference< sax::XAttributeList > const& xAttribs) const;

            /// retrieve the node name for an element
            rtl::OUString getName(rtl::OUString const& _sTag, uno::Reference< sax::XAttributeList > const& xAttribs, ElementType::Enum eType = ElementType::unknown) const;

            /// query whether the node has an operation
            Operation::Enum getOperation(uno::Reference< sax::XAttributeList > const& xAttribs, ElementType::Enum _eType) const;

            /// retrieve the language (if any) stored in the attribute list
            bool getLanguage(uno::Reference< sax::XAttributeList > const& xAttribs, rtl::OUString & _rsLanguage) const;

            /// reads attributes for nodes from the attribute list
            sal_Int16 getNodeFlags(uno::Reference< sax::XAttributeList > const& xAttribs, ElementType::Enum _eType) const;

            /// retrieve element type and associated module name of a set,
            bool getSetElementType(uno::Reference< sax::XAttributeList > const& _xAttribs, rtl::OUString& _rsElementType, rtl::OUString& _rsElementTypeModule) const;

            /// retrieve the instance type and associated module name of a instance,
            bool getInstanceType(uno::Reference< sax::XAttributeList > const& _xAttribs, rtl::OUString& _rsElementType, rtl::OUString& _rsElementTypeModule) const;

            /// retrieve the component for an import or uses element,
            bool getImportComponent(uno::Reference< sax::XAttributeList > const& _xAttribs, rtl::OUString& _rsComponent) const;

            /// retrieve element type and associated module name of a set,
            uno::Type getPropertyValueType(uno::Reference< sax::XAttributeList > const& _xAttribs) const;
                // throw( BadValueType )

            /// reads a value attribute from the attribute list
            bool isNull(uno::Reference< sax::XAttributeList > const& _xAttribs) const;

            /// reads a value attribute from the attribute list
            rtl::OUString getSeparator(uno::Reference< sax::XAttributeList > const& _xAttribs) const;

        // low-level internal methods

            /// checks for presence of a boolean attribute and assigns its value if it exists (and is a bool)
            bool maybeGetAttribute(uno::Reference< sax::XAttributeList > const& _xAttribs, rtl::OUString const& _aAttributeName, bool& _rbAttributeValue) const;

            /// checks for presence of an attribute and assigns its value if it exists
            bool maybeGetAttribute(uno::Reference< sax::XAttributeList > const& _xAttribs, rtl::OUString const& _aAttributeName, rtl::OUString& _rsAttributeValue) const;
        };
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif

