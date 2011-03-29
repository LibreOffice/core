/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _CONNECTIVITY_ADO_KEY_HXX_
#define _CONNECTIVITY_ADO_KEY_HXX_

#include "connectivity/sdbcx/VKey.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include "ado/Awrapadox.hxx"

namespace connectivity
{
    namespace ado
    {
        typedef sdbcx::OKey OKey_ADO;

        class OConnection;
        class OAdoKey : public OKey_ADO
        {
            WpADOKey        m_aKey;
            OConnection*    m_pConnection;
        protected:
            void fillPropertyValues();
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const ::com::sun::star::uno::Any& rValue)throw (::com::sun::star::uno::Exception);
        public:
            virtual void refreshColumns();
        public:
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
            OAdoKey(sal_Bool _bCase,OConnection* _pConnection,ADOKey* _pKey);
            OAdoKey(sal_Bool _bCase,OConnection* _pConnection);

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            WpADOKey        getImpl() const { return m_aKey;}
            // map the update/delete rules
            static RuleEnum Map2Rule(const sal_Int32& _eNum);
            static sal_Int32 MapRule(const RuleEnum& _eNum);

            // map the keytypes
            static sal_Int32 MapKeyRule(const KeyTypeEnum& _eNum);
            static KeyTypeEnum Map2KeyRule(const sal_Int32& _eNum);
        };
    }
}
#endif // _CONNECTIVITY_ADO_KEY_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
