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



#ifndef DBACCESS_MIGRATIONLOG_HXX
#define DBACCESS_MIGRATIONLOG_HXX

#include "dbmm_types.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <rtl/ustring.hxx>

#include <memory>

//........................................................................
namespace dbmm
{
//........................................................................

    typedef sal_Int16 DocumentID;
    struct MigrationError;

    //====================================================================
    //= MigrationLog
    //====================================================================
    struct MigrationLog_Data;
    class MigrationLog
    {
    public:
        MigrationLog();
        ~MigrationLog();

        //----------------------------------------------------------------
        //- event logging

        /** logs an unrecoverable error during the migration process
        */
        void        logFailure( const MigrationError& _rError );

        /** logs a recoverable (or at least ignorable) error during the migration process
        */
        void        logRecoverable( const MigrationError& _rError );

        /// checks whether logFailure has been called
        bool        hadFailure() const;

        /// logs the fact that the database document has been backed up
        void        backedUpDocument( const ::rtl::OUString& _rNewDocumentLocation );

        /** logs that the migration for a certain sub document has been started, returns
            a unique ID for this sub document which is to be used in later calls
        */
        DocumentID  startedDocument( const SubDocumentType _eType, const ::rtl::OUString& _rName );

        /** logs the event that a macro or script library has been moved from within a
            sub document to the database document
        */
        void        movedLibrary(
                        const DocumentID _nDocID,
                        const ScriptType _eScriptType,
                        const ::rtl::OUString& _rOriginalLibName,
                        const ::rtl::OUString& _rNewLibName
                    );

        /** logs that the migration for a certain document has been finished
        */
        void        finishedDocument( const DocumentID _nDocID );

        //----------------------------------------------------------------
        //- information retrieval

        /** retrieves the new name of a library

            The new library name must previously have been logger by calling
            <member>movedLibrary</member>. If not, an assertion will be raised in
            the non-product builds, and an empty string will be returned.
        */
        const ::rtl::OUString&
                    getNewLibraryName(
                        DocumentID _nDocID,
                        ScriptType _eScriptType,
                        const ::rtl::OUString& _rOriginalLibName
                    ) const;

        /** determines whether for the given document, any library needed to be (and was) moved
        */
        bool    movedAnyLibrary( const DocumentID );

        /** provides a human-readable version of the log, explaining a user what happened during
            the migration.
        */
        ::rtl::OUString
                    getCompleteLog() const;

    private:
        ::std::auto_ptr< MigrationLog_Data >    m_pData;
    };

//........................................................................
} // namespace dbmm
//........................................................................

#endif // DBACCESS_MIGRATIONLOG_HXX
