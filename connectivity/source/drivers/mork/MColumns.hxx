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

#ifndef _CONNECTIVITY_MORK_COLUMNS_HXX_
#define _CONNECTIVITY_MORK_COLUMNS_HXX_

#include "connectivity/sdbcx/VCollection.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include "connectivity/sdbcx/IRefreshable.hxx"
#include "MTable.hxx"

namespace connectivity
{
    namespace mork
    {
        class OColumns : public sdbcx::OCollection
        {
        protected:
            OTable* m_pTable;

            virtual sdbcx::ObjectType createObject(const ::rtl::OUString& _rName);
            virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException);
        public:
            OColumns(   OTable* _pTable,
                        ::osl::Mutex& _rMutex,
                        const TStringVector &_rVector
                        ) : sdbcx::OCollection(*_pTable,sal_True,_rMutex,_rVector)
                ,m_pTable(_pTable)
            {}
        };
    }
}
#endif // _CONNECTIVITY_MORK_COLUMNS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
