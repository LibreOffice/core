/*************************************************************************
 *
 *  $RCSfile: valuetypeconverter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:11 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_VALUECONVERTER_HXX
#define CONFIGMGR_VALUECONVERTER_HXX

#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#include <com/sun/star/uno/Type.hxx>
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

namespace configmgr
{
    namespace uno       = ::com::sun::star::uno;
    namespace script    = ::com::sun::star::script;
    using rtl::OUString;

// -----------------------------------------------------------------------------
    class ValueConverter
    {
        /// TypeConverter is used for converting type from string values
        uno::Reference< script::XTypeConverter > m_xTypeConverter;
        /// Value info
        uno::Type   m_aType;
        OUString    m_sSeparator;
        bool        m_bNull;
    public:
        typedef std::vector< OUString > StringList;
    public:
        /// construct a value converter with no initial type info
        ValueConverter(const uno::Reference< script::XTypeConverter > & _xTC)
        : m_xTypeConverter(_xTC)
        , m_aType()
        {
            implReset();
        }

        /// construct a value converter with a type
        ValueConverter(uno::Type const& _aType, const uno::Reference< script::XTypeConverter > & _xTC)
        : m_xTypeConverter(_xTC)
        , m_aType(_aType)
        {
            implReset();
        }

        /// provide access to the TypeConverter that is used for converting string format
        uno::Reference< script::XTypeConverter > const& getTypeConverter() const CFG_NOTHROW()
        {
            return m_xTypeConverter;
        }

        /// (re)start the converter with the current type
        void restart()
        {
            implReset();
        }

        /// (re)start the converter with a new type info (or none)
        void reset(uno::Type const & _aType = uno::Type())
        {
            m_aType = _aType;
            implReset();
        }

        /// set the NULL state of this converter
        void setIsNull(bool bNull = true)
        {
            m_bNull = bNull;
        }

        /// set the separator of this converter
        void setSeparator(OUString const & _aSeparator)
        {
            m_sSeparator = _aSeparator;
        }

        /// get the (UNO) type
        bool isTypeSet() const { return m_aType.getTypeClass() != uno::TypeClass_VOID; }

        /// get the (UNO) type
        uno::Type getType() const { return m_aType; }

        /// is this marked null
        bool isNull() const { return m_bNull; }

        /// does this have a list type
        bool isList() const;

        /// does this have a separator set
        bool hasSeparator() const { return m_sSeparator.getLength() != 0; }

        /// converting a value
        uno::Any convertToAny(OUString const& aContent) const
            CFG_UNO_THROW1( script::CannotConvertException );

        /// converting a list
        uno::Any convertListToAny(uno::Sequence< OUString > const& aContentList) const
            CFG_UNO_THROW1( script::CannotConvertException );

        /// converting a binary value
        uno::Sequence<sal_Int8> parseBinary(OUString const& aBinaryString) const
            CFG_UNO_THROW1( script::CannotConvertException );

        /// splits a string list
        uno::Sequence< OUString > splitStringList(OUString const& aContent) const;
    private:
        /// converting a list
        bool convertListToAny(StringList const& aContentList, uno::Any& rValue) const
            CFG_UNO_THROW1( script::CannotConvertException );

        /// converting a scalar value
        bool convertScalarToAny(OUString const& aContent, uno::Any& rValue) const
            CFG_UNO_THROW1( script::CannotConvertException );

        /// splitting a string list
        void splitListData(OUString const& aContent, StringList& rContentList) const
            CFG_NOTHROW( );
    private:
        void implReset() CFG_NOTHROW()
        {
            m_sSeparator = OUString();
            m_bNull = false;
        }
    };
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
} // namespace

#endif
