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
#ifndef _CONNECTIVITY_ADO_INDEXES_HXX_
#define _CONNECTIVITY_ADO_INDEXES_HXX_

#include "connectivity/sdbcx/VCollection.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include "ado/Awrapadox.hxx"

namespace connectivity
{
    namespace ado
    {
        class OConnection;
        class OIndexes : public sdbcx::OCollection
        {
            WpADOIndexes    m_aCollection;
            OConnection*    m_pConnection;
        protected:
            virtual sdbcx::ObjectType createObject(const OUString& _rName);
            virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > createDescriptor();
            virtual sdbcx::ObjectType appendObject( const OUString& _rForName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& descriptor );
            virtual void dropObject(sal_Int32 _nPos,const OUString _sElementName);
        public:
            OIndexes(::cppu::OWeakObject& _rParent,
                     ::osl::Mutex& _rMutex,
                     const TStringVector &_rVector,
                     const WpADOIndexes&    _rCollection,
                     sal_Bool _bCase,
                     OConnection* _pConnection)
                    : sdbcx::OCollection(_rParent,_bCase,_rMutex,_rVector)
                    , m_aCollection(_rCollection)
                    , m_pConnection(_pConnection)
            {
            }
        };
    }
}

#endif // _CONNECTIVITY_ADO_INDEXES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
