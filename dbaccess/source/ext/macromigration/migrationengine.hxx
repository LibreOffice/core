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

#ifndef DBACCESS_MIGRATIONENGINE_HXX
#define DBACCESS_MIGRATIONENGINE_HXX

#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>

#include <memory>

namespace dbmm
{

    class IMigrationProgress;
    class MigrationLog;

    // MigrationEngine
    class MigrationEngine_Impl;
    class MigrationEngine
    {
    public:
        /** creates the migration engine
            @param  _rxDocument
                the document whose macros/scripts should be migrated
            @param  _rProgress
                a callback for notifying progress. Beware of lifetimes here: The progress callback,
                passed herein as reference, must live as long as the engine instance lives.
        */
        MigrationEngine(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XOfficeDatabaseDocument >& _rxDocument,
            IMigrationProgress& _rProgress,
            MigrationLog& _rLogger
        );

        ~MigrationEngine();

        sal_Int32   getFormCount() const;
        sal_Int32   getReportCount() const;

        /** starts migrating the scripts and macros in the forms/reports to the database
            document.

            This process cannot be cancelled, as it would leave the document in an inconsistent
            state.

            When the function returns, then the migration is finished.

            @return
                whether or not the migration was successful. If it wasn't, then an error has been reported
                to the user, using the document's interaction handler. If no such interaction handler
                was present, then the error has been silenced.
        */
        bool    migrateAll();

    private:
        ::std::auto_ptr< MigrationEngine_Impl > m_pImpl;
    };

} // namespace dbmm

#endif // DBACCESS_MIGRATIONENGINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
