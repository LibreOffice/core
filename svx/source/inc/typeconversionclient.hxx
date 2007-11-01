/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: typeconversionclient.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 15:01:03 $
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

#ifndef SVX_TYPECONVERSION_CLIENT_HXX
#define SVX_TYPECONVERSION_CLIENT_HXX

#ifndef SVX_DBTOOLSCLIENT_HXX
#include "dbtoolsclient.hxx"
#endif

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
            const ::com::sun::star::util::Date& _rNullDate,
            sal_Int16 _nKeyType) const
        {
            double nReturn(0);
            if ( ensureLoaded() )
                nReturn = m_xTypeConversion->getValue(_rxVariant, _rNullDate, _nKeyType);
            return nReturn;
        }

        // --------------------------------------------------------
        inline ::rtl::OUString getValue(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxColumn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
            const ::com::sun::star::util::Date& _rNullDate,
            sal_Int32 _nKey,
            sal_Int16 _nKeyType) const
        {
            ::rtl::OUString sReturn;
            if ( ensureLoaded() )
                sReturn = m_xTypeConversion->getValue(_rxColumn, _rxFormatter, _rNullDate, _nKey, _nKeyType);
            return sReturn;
        }
    };

//........................................................................
}   // namespace svxform
//........................................................................

#endif // SVX_TYPECONVERSION_CLIENT_HXX


