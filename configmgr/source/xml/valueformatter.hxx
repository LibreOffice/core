/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: valueformatter.hxx,v $
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

#ifndef CONFIGMGR_XML_VALUEFORMATTER_HXX
#define CONFIGMGR_XML_VALUEFORMATTER_HXX

#include <com/sun/star/script/XTypeConverter.hpp>

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace xml
    {
// -----------------------------------------------------------------------------
        namespace uno   = ::com::sun::star::uno;
        namespace lang  = ::com::sun::star::lang;
        namespace script= ::com::sun::star::script;
// -----------------------------------------------------------------------------
        class ElementFormatter;
// -----------------------------------------------------------------------------

        // Value to XML (String) conversions
        class ValueFormatter
        {
        public:
            explicit
            ValueFormatter(uno::Any const & _aValue)
            : m_aValue(_aValue)
            {
                makeSeparator();
            }

            void reset(uno::Any const & _aValue)
            { m_aValue = _aValue; makeSeparator(); }

            bool addValueAttributes(ElementFormatter & _rFormatter) const;

            bool hasContent() const;

            rtl::OUString getContent(uno::Reference< script::XTypeConverter > const & _xTCV) const;

        private:
            bool isList() const { return m_sSeparator.getLength() != 0; }
            void makeSeparator();

            uno::Any m_aValue;
            rtl::OUString m_sSeparator;
            bool     m_bUseSeparator;
        };
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------
} // namespace

#endif
