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
