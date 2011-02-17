/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SVX_TYPECONVERSION_CLIENT_HXX
#define SVX_TYPECONVERSION_CLIENT_HXX

#include "svx/dbtoolsclient.hxx"

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= OTypeConversionClient
    //====================================================================
    class OTypeConversionClient : public ODbtoolsClient
    {
    protected:
        mutable ::rtl::Reference< ::connectivity::simple::IDataAccessTypeConversion >
                m_xTypeConversion;
        virtual bool ensureLoaded() const;

    public:
        OTypeConversionClient();

        // --------------------------------------------------------
        inline ::com::sun::star::util::Date getStandardDate() const
        {
            ::com::sun::star::util::Date aReturn;
            if ( ensureLoaded() )
                aReturn = m_xTypeConversion->getStandardDate();
            return aReturn;
        }

        // --------------------------------------------------------
        inline double getValue(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn>& _rxVariant,
            const ::com::sun::star::util::Date& _rNullDate ) const
        {
            double nReturn(0);
            if ( ensureLoaded() )
                nReturn = m_xTypeConversion->getValue( _rxVariant, _rNullDate );
            return nReturn;
        }

        // --------------------------------------------------------
        inline ::rtl::OUString getFormattedValue(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxColumn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
            const ::com::sun::star::util::Date& _rNullDate,
            sal_Int32 _nKey,
            sal_Int16 _nKeyType) const
        {
            ::rtl::OUString sReturn;
            if ( ensureLoaded() )
                sReturn = m_xTypeConversion->getFormattedValue(_rxColumn, _rxFormatter, _rNullDate, _nKey, _nKeyType);
            return sReturn;
        }
    };

//........................................................................
}   // namespace svxform
//........................................................................

#endif // SVX_TYPECONVERSION_CLIENT_HXX


