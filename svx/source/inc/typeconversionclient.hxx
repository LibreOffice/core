/*************************************************************************
 *
 *  $RCSfile: typeconversionclient.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2001-07-25 13:36:11 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
        ::rtl::Reference< ::connectivity::simple::IDataAccessTypeConversion >
                m_xTypeConversion;

    public:
        OTypeConversionClient();

        // --------------------------------------------------------
        inline ::com::sun::star::util::Date getStandardDate() const
        {
            ::com::sun::star::util::Date aReturn;
            if (m_xTypeConversion.is())
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
            if (m_xTypeConversion.is())
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
            if (m_xTypeConversion.is())
                sReturn = m_xTypeConversion->getValue(_rxColumn, _rxFormatter, _rNullDate, _nKey, _nKeyType);
            return sReturn;
        }
    };

//........................................................................
}   // namespace svxform
//........................................................................

#endif // SVX_TYPECONVERSION_CLIENT_HXX

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 25.07.01 13:52:37  fs
 ************************************************************************/

