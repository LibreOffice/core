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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FCOLUMNS_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FCOLUMNS_HXX

#include <connectivity/sdbcx/VCollection.hxx>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <connectivity/sdbcx/IRefreshable.hxx>
#include "file/FTable.hxx"
#include "file/filedllapi.hxx"

namespace connectivity
{
    namespace file
    {
        class OOO_DLLPUBLIC_FILE OColumns : public sdbcx::OCollection
        {
        protected:
            OFileTable* m_pTable;

            virtual sdbcx::ObjectType createObject(const OUString& _rName) override;
            virtual void impl_refresh() throw(::com::sun::star::uno::RuntimeException) override;
        public:
            OColumns(   OFileTable* _pTable,
                        ::osl::Mutex& _rMutex,
                        const TStringVector &_rVector
                    ) : sdbcx::OCollection(*_pTable,_pTable->getConnection()->getMetaData()->supportsMixedCaseQuotedIdentifiers(),_rMutex,_rVector)
                ,m_pTable(_pTable)
            {}
        };
    }
}
#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FCOLUMNS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
