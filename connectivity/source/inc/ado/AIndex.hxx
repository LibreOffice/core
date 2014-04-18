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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_AINDEX_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_AINDEX_HXX

#include "connectivity/sdbcx/VIndex.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include "ado/Awrapadox.hxx"

namespace connectivity
{
    namespace ado
    {
        typedef sdbcx::OIndex OIndex_ADO;

        class OConnection;
        class OAdoIndex :    public OIndex_ADO
        {
            WpADOIndex      m_aIndex;
            OConnection*    m_pConnection;
        protected:
            void fillPropertyValues();
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const ::com::sun::star::uno::Any& rValue)throw (::com::sun::star::uno::Exception);
        public:
            virtual void refreshColumns();
        public:
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
            OAdoIndex(sal_Bool _bCase,  OConnection* _pConnection,ADOIndex* _pIndex);
            OAdoIndex(sal_Bool _bCase,  OConnection* _pConnection);
            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            WpADOIndex      getImpl() const { return m_aIndex;}
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_AINDEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
