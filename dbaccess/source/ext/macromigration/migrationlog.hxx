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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
