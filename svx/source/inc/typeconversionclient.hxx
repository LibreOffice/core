/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


