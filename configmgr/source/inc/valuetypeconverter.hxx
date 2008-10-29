/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: valuetypeconverter.hxx,v $
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

#ifndef CONFIGMGR_VALUECONVERTER_HXX
#define CONFIGMGR_VALUECONVERTER_HXX

#include "utility.hxx"
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

namespace configmgr
{
    namespace uno       = ::com::sun::star::uno;
    namespace script    = ::com::sun::star::script;

// -----------------------------------------------------------------------------
    class ValueConverter
    {
        /// TypeConverter is used for converting type from string values
        uno::Reference< script::XTypeConverter > m_xTypeConverter;
        /// Value info
        uno::Type   m_aType;
        rtl::OUString    m_sSeparator;
        bool        m_bNull;
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
        uno::Reference< script::XTypeConverter > const& getTypeConverter() const SAL_THROW(())
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
        void setSeparator(rtl::OUString const & _aSeparator)
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
        uno::Any convertToAny(rtl::OUString const& aContent) const
            SAL_THROW((script::CannotConvertException , com::sun::star::uno::RuntimeException));

        /// converting a list
        uno::Any convertListToAny(uno::Sequence< rtl::OUString > const& aContentList) const
            SAL_THROW((script::CannotConvertException , com::sun::star::uno::RuntimeException));

        /// converting a binary value
        uno::Sequence<sal_Int8> parseBinary(rtl::OUString const& aBinaryString) const
            SAL_THROW((script::CannotConvertException , com::sun::star::uno::RuntimeException));

        /// splits a string list
        uno::Sequence< rtl::OUString > splitStringList(rtl::OUString const& aContent) const;
    private:
        /// converting a list
        bool convertListToAny(std::vector< rtl::OUString > const& aContentList, uno::Any& rValue) const
            SAL_THROW((script::CannotConvertException , com::sun::star::uno::RuntimeException));

        /// converting a scalar value
        bool convertScalarToAny(rtl::OUString const& aContent, uno::Any& rValue) const
            SAL_THROW((script::CannotConvertException , com::sun::star::uno::RuntimeException));

        /// splitting a string list
        void splitListData(rtl::OUString const& aContent, std::vector< rtl::OUString >& rContentList) const
            SAL_THROW(());
    private:
        void implReset() SAL_THROW(())
        {
            m_sSeparator = rtl::OUString();
            m_bNull = false;
        }
    };
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
} // namespace

#endif
