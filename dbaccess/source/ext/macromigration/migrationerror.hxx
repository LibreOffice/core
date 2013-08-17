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

#ifndef DBACCESS_MIGRATIONERROR_HXX
#define DBACCESS_MIGRATIONERROR_HXX

#include <com/sun/star/uno/Any.hxx>

#include <vector>

namespace dbmm
{

    enum MigrationErrorType
    {
        ERR_OPENING_SUB_DOCUMENT_FAILED = 1,
        ERR_CLOSING_SUB_DOCUMENT_FAILED,
        ERR_STORAGE_COMMIT_FAILED,
        ERR_STORING_DATABASEDOC_FAILED,
        ERR_COLLECTING_DOCUMENTS_FAILED,
        ERR_UNEXPECTED_LIBSTORAGE_ELEMENT,
        ERR_CREATING_DBDOC_SCRIPT_STORAGE_FAILED,
        ERR_COMMITTING_SCRIPT_STORAGES_FAILED,
        ERR_GENERAL_SCRIPT_MIGRATION_FAILURE,
        ERR_GENERAL_MACRO_MIGRATION_FAILURE,
        ERR_UNKNOWN_SCRIPT_TYPE,
        ERR_UNKNOWN_SCRIPT_LANGUAGE,
        ERR_UNKNOWN_SCRIPT_NAME_FORMAT,
        ERR_SCRIPT_TRANSLATION_FAILURE,
        ERR_INVALID_SCRIPT_DESCRIPTOR_FORMAT,
        ERR_ADJUSTING_DOCUMENT_EVENTS_FAILED,
        ERR_ADJUSTING_DIALOG_EVENTS_FAILED,
        ERR_ADJUSTING_FORMCOMP_EVENTS_FAILED,
        ERR_BIND_SCRIPT_STORAGE_FAILED,
        ERR_REMOVE_SCRIPTS_STORAGE_FAILED,
        ERR_DOCUMENT_BACKUP_FAILED,
        ERR_UNKNOWN_SCRIPT_FOLDER,
        ERR_EXAMINING_SCRIPTS_FOLDER_FAILED,
        ERR_PASSWORD_VERIFICATION_FAILED,
        ERR_NEW_STYLE_REPORT
    };

    // MigrationError
    /** encapsulates information about an error which happened during the migration
    */
    struct MigrationError
    {
        const MigrationErrorType            eType;
        ::std::vector< OUString >    aErrorDetails;
        const ::com::sun::star::uno::Any    aCaughtException;

        MigrationError(
                const MigrationErrorType _eType )
            :eType( _eType )
        {
        }

        MigrationError(
                const MigrationErrorType _eType,
                const ::com::sun::star::uno::Any& _rCaughtException )
            :eType( _eType )
            ,aCaughtException( _rCaughtException )
        {
        }

        MigrationError(
                const MigrationErrorType _eType,
                const OUString& _rDetail )
            :eType( _eType )
        {
            impl_constructDetails( _rDetail );
        }

        MigrationError(
                const MigrationErrorType _eType,
                const OUString& _rDetail,
                const ::com::sun::star::uno::Any& _rCaughtException )
            :eType( _eType )
            ,aCaughtException( _rCaughtException )
        {
            impl_constructDetails( _rDetail );
        }

        MigrationError(
                const MigrationErrorType _eType,
                const OUString& _rDetail1,
                const OUString& _rDetail2 )
            :eType( _eType )
        {
            impl_constructDetails( _rDetail1, _rDetail2 );
        }

        MigrationError(
                const MigrationErrorType _eType,
                const OUString& _rDetail1,
                const OUString& _rDetail2,
                const ::com::sun::star::uno::Any& _rCaughtException )
            :eType( _eType )
            ,aCaughtException( _rCaughtException )
        {
            impl_constructDetails( _rDetail1, _rDetail2 );
        }

        MigrationError(
                const MigrationErrorType _eType,
                const OUString& _rDetail1,
                const OUString& _rDetail2,
                const OUString& _rDetail3,
                const ::com::sun::star::uno::Any& _rCaughtException )
            :eType( _eType )
            ,aCaughtException( _rCaughtException )
        {
            impl_constructDetails( _rDetail1, _rDetail2, _rDetail3 );
        }

        MigrationError(
                const MigrationErrorType _eType,
                const OUString& _rDetail1,
                const OUString& _rDetail2,
                const OUString& _rDetail3 )
            :eType( _eType )
        {
            impl_constructDetails( _rDetail1, _rDetail2, _rDetail3 );
        }

    private:
        void    impl_constructDetails(
                    const OUString& _rDetail1,
                    const OUString& _rDetail2 = OUString(),
                    const OUString& _rDetail3 = OUString()
                )
        {
            if ( !_rDetail1.isEmpty() ) aErrorDetails.push_back( _rDetail1 );
            if ( !_rDetail2.isEmpty() ) aErrorDetails.push_back( _rDetail2 );
            if ( !_rDetail3.isEmpty() ) aErrorDetails.push_back( _rDetail3 );
        }
    };

} // namespace dbmm

#endif // DBACCESS_MIGRATIONERROR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
