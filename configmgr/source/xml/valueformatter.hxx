/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: valueformatter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:43:15 $
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

#ifndef CONFIGMGR_XML_VALUEFORMATTER_HXX
#define CONFIGMGR_XML_VALUEFORMATTER_HXX

#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace xml
    {
// -----------------------------------------------------------------------------
        namespace uno   = ::com::sun::star::uno;
        namespace lang  = ::com::sun::star::lang;
        namespace script= ::com::sun::star::script;

        using rtl::OUString;
// -----------------------------------------------------------------------------
        class ElementFormatter;
// -----------------------------------------------------------------------------

        // Value to XML (String) conversions
        class ValueFormatter
        {
        public:
            typedef uno::Reference< script::XTypeConverter >  TypeConverter;

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

            OUString getContent(TypeConverter const & _xTCV) const;

        private:
            bool isList() const { return m_sSeparator.getLength() != 0; }
            void makeSeparator();

            uno::Any m_aValue;
            OUString m_sSeparator;
            bool     m_bUseSeparator;
        };
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------
} // namespace

#endif
