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

#ifndef DBACCESS_MIGRATIONENGINE_HXX
#define DBACCESS_MIGRATIONENGINE_HXX

/** === begin UNO includes === **/
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>

#include <memory>

namespace comphelper {
    class ComponentContext;
}

//........................................................................
namespace dbmm
{
//........................................................................

    class IMigrationProgress;
    class MigrationLog;

    //====================================================================
    //= MigrationEngine
    //====================================================================
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
            const ::comphelper::ComponentContext& _rContext,
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

//........................................................................
} // namespace dbmm
//........................................................................

#endif // DBACCESS_MIGRATIONENGINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
