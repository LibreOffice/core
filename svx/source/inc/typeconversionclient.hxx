/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_SOURCE_INC_TYPECONVERSIONCLIENT_HXX
#define INCLUDED_SVX_SOURCE_INC_TYPECONVERSIONCLIENT_HXX

#include "svx/dbtoolsclient.hxx"


namespace svxform
{



    //= OTypeConversionClient

    class OTypeConversionClient : public ODbtoolsClient
    {
    protected:
        mutable ::rtl::Reference< ::connectivity::simple::IDataAccessTypeConversion >
                m_xTypeConversion;
        virtual bool ensureLoaded() const SAL_OVERRIDE;

    public:
        OTypeConversionClient();


        inline ::com::sun::star::util::Date getStandardDate() const
        {
            ::com::sun::star::util::Date aReturn;
            if ( ensureLoaded() )
                aReturn = m_xTypeConversion->getStandardDate();
            return aReturn;
        }


        inline double getValue(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn>& _rxVariant,
            const ::com::sun::star::util::Date& _rNullDate ) const
        {
            double nReturn(0);
            if ( ensureLoaded() )
                nReturn = m_xTypeConversion->getValue( _rxVariant, _rNullDate );
            return nReturn;
        }


        inline OUString getFormattedValue(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XColumn >& _rxColumn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
            const ::com::sun::star::util::Date& _rNullDate,
            sal_Int32 _nKey,
            sal_Int16 _nKeyType) const
        {
            OUString sReturn;
            if ( ensureLoaded() )
                sReturn = m_xTypeConversion->getFormattedValue(_rxColumn, _rxFormatter, _rNullDate, _nKey, _nKeyType);
            return sReturn;
        }
    };


}   // namespace svxform


#endif // INCLUDED_SVX_SOURCE_INC_TYPECONVERSIONCLIENT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
