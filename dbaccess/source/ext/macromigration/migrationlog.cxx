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


#include "dbmm_module.hxx"
#include "dbmm_global.hrc"
#include "migrationerror.hxx"
#include "migrationlog.hxx"


#include <comphelper/anytostring.hxx>
#include <tools/string.hxx>
#include <rtl/ustrbuf.hxx>

#include <vector>
#include <map>
#include <list>

//........................................................................
namespace dbmm
{
    //====================================================================
    //= LibraryEntry
    //====================================================================
    struct LibraryEntry
    {
        ScriptType      eType;
        OUString sOldName;
        OUString sNewName;

        LibraryEntry()
            :eType( eBasic )
            ,sOldName()
            ,sNewName()
        {
        }

        LibraryEntry( const ScriptType& _eType, const OUString& _rOldName, const OUString& _rNewName )
            :eType( _eType )
            ,sOldName( _rOldName )
            ,sNewName( _rNewName )
        {
        }
    };

    //====================================================================
    //= DocumentEntry
    //====================================================================
    struct DocumentEntry
    {
        SubDocumentType                 eType;
        OUString                 sName;
        ::std::vector< LibraryEntry >   aMovedLibraries;

        DocumentEntry()
            :eType( eForm )
            ,sName()
            ,aMovedLibraries()
        {
        }

        DocumentEntry( const SubDocumentType _eType, const OUString& _rName )
            :eType( _eType )
            ,sName( _rName )
        {
        }
    };

    //====================================================================
    //= DocumentLogs
    //====================================================================
    typedef ::std::map< DocumentID, DocumentEntry > DocumentLogs;

    //====================================================================
    //= ErrorLog
    //====================================================================
    typedef ::std::list< MigrationError >   ErrorLog;

    //====================================================================
    //= MigrationLog_Data
    //====================================================================
    struct MigrationLog_Data
    {
        OUString sBackupLocation;
        DocumentLogs    aDocumentLogs;
        ErrorLog        aFailures;
        ErrorLog        aWarnings;
    };

    //====================================================================
    //= MigrationLog
    //====================================================================
    //--------------------------------------------------------------------
    MigrationLog::MigrationLog()
        :m_pData( new MigrationLog_Data )
    {
    }

    //--------------------------------------------------------------------
    MigrationLog::~MigrationLog()
    {
    }

    //--------------------------------------------------------------------
    void MigrationLog::logFailure( const MigrationError& _rError )
    {
        m_pData->aFailures.push_back( _rError );
    }

    //--------------------------------------------------------------------
    void MigrationLog::logRecoverable( const MigrationError& _rError )
    {
        m_pData->aWarnings.push_back( _rError );
    }

    //--------------------------------------------------------------------
    bool MigrationLog::hadFailure() const
    {
        return !m_pData->aFailures.empty();
    }

    //--------------------------------------------------------------------
    void MigrationLog::backedUpDocument( const OUString& _rNewDocumentLocation )
    {
        m_pData->sBackupLocation = _rNewDocumentLocation;
    }

    //--------------------------------------------------------------------
    DocumentID MigrationLog::startedDocument( const SubDocumentType _eType, const OUString& _rName )
    {
#if OSL_DEBUG_LEVEL > 0
        bool bAlreadyKnown = false;
        for (   DocumentLogs::const_iterator doc = m_pData->aDocumentLogs.begin();
                doc != m_pData->aDocumentLogs.end() && !bAlreadyKnown;
                ++doc
            )
        {
            bAlreadyKnown = ( doc->second.eType == _eType ) && ( doc->second.sName == _rName );
        }
        OSL_ENSURE( !bAlreadyKnown, "MigrationLog::startedDocument: document is already known!" );
#endif

        DocumentID nID = (DocumentID)( m_pData->aDocumentLogs.size() + 1 );
        while ( m_pData->aDocumentLogs.find( nID ) != m_pData->aDocumentLogs.end() )
            ++nID;

        m_pData->aDocumentLogs[ nID ] = DocumentEntry( _eType, _rName );

        return nID;
    }

    //--------------------------------------------------------------------
    void MigrationLog::movedLibrary( const DocumentID _nDocID, const ScriptType _eScriptType,
            const OUString& _rOriginalLibName, const OUString& _rNewLibName )
    {
        OSL_ENSURE( m_pData->aDocumentLogs.find( _nDocID ) != m_pData->aDocumentLogs.end(),
            "MigrationLog::movedLibrary: document is not known!" );

        DocumentEntry& rDocEntry = m_pData->aDocumentLogs[ _nDocID ];
        rDocEntry.aMovedLibraries.push_back( LibraryEntry( _eScriptType, _rOriginalLibName, _rNewLibName ) );
    }

    //--------------------------------------------------------------------
    void MigrationLog::finishedDocument( const DocumentID _nDocID )
    {
        OSL_ENSURE( m_pData->aDocumentLogs.find( _nDocID ) != m_pData->aDocumentLogs.end(),
            "MigrationLog::finishedDocument: document is not known!" );

        DocumentEntry& rDocEntry = m_pData->aDocumentLogs[ _nDocID ];
        (void)rDocEntry;
        // nothing to do here
    }

    //--------------------------------------------------------------------
    const OUString& MigrationLog::getNewLibraryName( DocumentID _nDocID, ScriptType _eScriptType,
        const OUString& _rOriginalLibName ) const
    {
        static OUString s_sEmptyString;

        DocumentLogs::const_iterator docPos = m_pData->aDocumentLogs.find( _nDocID );
        if ( docPos == m_pData->aDocumentLogs.end() )
        {
            OSL_FAIL( "MigrationLog::getNewLibraryName: document is not known!" );
            return s_sEmptyString;
        }

        const DocumentEntry& rDocEntry( docPos->second );
        for (   ::std::vector< LibraryEntry >::const_iterator lib = rDocEntry.aMovedLibraries.begin();
                lib != rDocEntry.aMovedLibraries.end();
                ++lib
            )
        {
            if  (   ( _eScriptType == lib->eType )
                &&  ( _rOriginalLibName == lib->sOldName )
                )
                return lib->sNewName;
        }

        OSL_FAIL( "MigrationLog::getNewLibraryName: doc is known, but library isn't!" );
        return s_sEmptyString;
    }

    //--------------------------------------------------------------------
    namespace
    {
        //----------------------------------------------------------------
        static void lcl_appendErrorDescription( OUStringBuffer& _inout_rBuffer, const MigrationError& _rError )
        {
            const sal_Char* pAsciiErrorDescription( NULL );
            ::std::vector< OUString > aParameterNames;
            switch ( _rError.eType )
            {
            case ERR_OPENING_SUB_DOCUMENT_FAILED:
                pAsciiErrorDescription = "opening '#doc#' failed";
                aParameterNames.push_back(OUString("#doc#"));
                break;

            case ERR_CLOSING_SUB_DOCUMENT_FAILED:
                pAsciiErrorDescription = "closing '#doc#' failed";
                aParameterNames.push_back(OUString("#doc#"));
                break;

            case ERR_STORAGE_COMMIT_FAILED:
                pAsciiErrorDescription = "committing the changes for document '#doc#' failed";
                aParameterNames.push_back(OUString("#doc#"));
                break;

            case ERR_STORING_DATABASEDOC_FAILED:
                pAsciiErrorDescription = "storing the database document failed";
                break;

            case ERR_COLLECTING_DOCUMENTS_FAILED:
                pAsciiErrorDescription = "collecting the forms/reports of the database document failed";
                break;

            case ERR_UNEXPECTED_LIBSTORAGE_ELEMENT:
                pAsciiErrorDescription = "unexpected #lib# storage element in document '#doc#', named '#element#'";
                aParameterNames.push_back(OUString("#doc#"));
                aParameterNames.push_back(OUString("#libstore#"));
                aParameterNames.push_back(OUString("#element#"));
                break;

            case ERR_CREATING_DBDOC_SCRIPT_STORAGE_FAILED:
                pAsciiErrorDescription = "creating the database document's storage for #scripttype# scripts failed";
                aParameterNames.push_back(OUString("#scripttype#"));
                break;

            case ERR_COMMITTING_SCRIPT_STORAGES_FAILED:
                pAsciiErrorDescription = "saving the #scripttype# scripts for document '#doc#' failed";
                aParameterNames.push_back(OUString("#scripttype#"));
                aParameterNames.push_back(OUString("#doc#"));
                break;

            case ERR_GENERAL_SCRIPT_MIGRATION_FAILURE:
                pAsciiErrorDescription = "general error while migrating #scripttype# scripts of document '#doc#'";
                aParameterNames.push_back(OUString("#scripttype#"));
                aParameterNames.push_back(OUString("#doc#"));
                break;

            case ERR_GENERAL_MACRO_MIGRATION_FAILURE:
                pAsciiErrorDescription = "general error during macro migration of document '#doc#'";
                aParameterNames.push_back(OUString("#doc#"));
                break;

            case ERR_UNKNOWN_SCRIPT_TYPE:
                pAsciiErrorDescription = "unknown script type: #type#";
                aParameterNames.push_back(OUString("#type#"));
                break;

            case ERR_UNKNOWN_SCRIPT_LANGUAGE:
                pAsciiErrorDescription = "unknown script language: #lang#";
                aParameterNames.push_back(OUString("#lang#"));
                break;

            case ERR_UNKNOWN_SCRIPT_NAME_FORMAT:
                pAsciiErrorDescription = "unknown script name format: #script#";
                aParameterNames.push_back(OUString("#script#"));
                break;

            case ERR_SCRIPT_TRANSLATION_FAILURE:
                pAsciiErrorDescription = "analyzing/translating the script URL failed; script type: #type#; script: #code#";
                aParameterNames.push_back(OUString("#type#"));
                aParameterNames.push_back(OUString("#code#"));
                break;

            case ERR_INVALID_SCRIPT_DESCRIPTOR_FORMAT:
                pAsciiErrorDescription = "invalid script descriptor format";
                break;

            case ERR_ADJUSTING_DOCUMENT_EVENTS_FAILED:
                pAsciiErrorDescription = "adjusting events for document '#doc#' failed";
                aParameterNames.push_back(OUString("#doc#"));
                break;

            case ERR_ADJUSTING_DIALOG_EVENTS_FAILED:
                pAsciiErrorDescription = "adjusting events for dialog #lib#.#dlg# in document '#doc#' failed";
                aParameterNames.push_back(OUString("#doc#"));
                aParameterNames.push_back(OUString("#lib#"));
                aParameterNames.push_back(OUString("#dlg#"));
                break;

            case ERR_ADJUSTING_FORMCOMP_EVENTS_FAILED:
                pAsciiErrorDescription = "adjusting form component events for '#doc#' failed";
                aParameterNames.push_back(OUString("#doc#"));
                break;

            case ERR_BIND_SCRIPT_STORAGE_FAILED:
                pAsciiErrorDescription = "binding to the script storage failed for document '#doc#'";
                aParameterNames.push_back(OUString("#doc#"));
                break;

            case ERR_REMOVE_SCRIPTS_STORAGE_FAILED:
                pAsciiErrorDescription = "removing a scripts storage failed for document '#doc#'";
                aParameterNames.push_back(OUString("#doc#"));
                break;

            case ERR_DOCUMENT_BACKUP_FAILED:
                pAsciiErrorDescription = "backing up the document to #location# failed";
                aParameterNames.push_back(OUString("#location#"));
                break;

            case ERR_UNKNOWN_SCRIPT_FOLDER:
                pAsciiErrorDescription = "unknown script folder '#name#' in document '#doc#'";
                aParameterNames.push_back(OUString("#doc#"));
                aParameterNames.push_back(OUString("#name#"));
                break;

            case ERR_EXAMINING_SCRIPTS_FOLDER_FAILED:
                pAsciiErrorDescription = "examining the 'Scripts' folder failed for document '#doc#'";
                aParameterNames.push_back(OUString("#doc#"));
                break;

            case ERR_PASSWORD_VERIFICATION_FAILED:
                pAsciiErrorDescription = "password verification failed for document '#doc#', #libtype# library '#name#'";
                aParameterNames.push_back(OUString("#doc#"));
                aParameterNames.push_back(OUString("#libtype#"));
                aParameterNames.push_back(OUString("#name#"));
                break;

            case ERR_NEW_STYLE_REPORT:
                pAsciiErrorDescription = "#doc# could not be processed, since you don't have the Oracle Report Builder (TM) extension installed.";
                aParameterNames.push_back(OUString("#doc#"));
                break;

                // do *not* add a default case here: Without a default, some compilers will warn you when
                // you miss a newly-introduced enum value here
            }
            OSL_ENSURE( pAsciiErrorDescription, "lcl_appendErrorDescription: no error message!" );
            if ( pAsciiErrorDescription )
            {
                OUString sSubstituted( OUString::createFromAscii( pAsciiErrorDescription ) );
                OSL_ENSURE( aParameterNames.size() == _rError.aErrorDetails.size(),
                    "lcl_appendErrorDescription: unexpected number of error message parameters!" );

                for ( size_t i=0; i < ::std::min( aParameterNames.size(), _rError.aErrorDetails.size() ); ++i )
                {
                    sSubstituted = sSubstituted.replaceFirst(
                        aParameterNames[i], _rError.aErrorDetails[i]);
                }

                _inout_rBuffer.append( sSubstituted );
            }
        }

        //----------------------------------------------------------------
        void lcl_describeErrors( OUStringBuffer& _rBuffer, const ErrorLog& _rErrors, const sal_uInt16 _nHeadingResId )
        {
            _rBuffer.appendAscii( "=== " );
            _rBuffer.append     ( OUString( MacroMigrationResId( _nHeadingResId ) ) );
            _rBuffer.appendAscii( " ===\n" );

            OUString sException( MacroMigrationResId( STR_EXCEPTION ) );

            for (   ErrorLog::const_iterator error = _rErrors.begin();
                    error != _rErrors.end();
                    ++error
                )
            {
                _rBuffer.append( sal_Unicode( '-' ) );
                _rBuffer.append( sal_Unicode( ' ' ) );
                lcl_appendErrorDescription( _rBuffer, *error );
                _rBuffer.append( sal_Unicode( '\n' ) );

                if ( !error->aCaughtException.hasValue() )
                    continue;

                _rBuffer.append( sException );
                _rBuffer.append( ::comphelper::anyToString( error->aCaughtException ) );
                _rBuffer.append( sal_Unicode( '\n' ) );
                _rBuffer.append( sal_Unicode( '\n' ) );
            }
        }
    }

    //--------------------------------------------------------------------
    bool MigrationLog::movedAnyLibrary( const DocumentID _nDocID )
    {
        DocumentLogs::const_iterator docPos = m_pData->aDocumentLogs.find( _nDocID );
        if ( docPos == m_pData->aDocumentLogs.end() )
        {
            OSL_FAIL( "MigrationLog::movedAnyLibrary: document is not known!" );
            return false;
        }
        return !docPos->second.aMovedLibraries.empty();
    }

    //--------------------------------------------------------------------
    OUString MigrationLog::getCompleteLog() const
    {
        OUStringBuffer aBuffer;

        if ( !m_pData->sBackupLocation.isEmpty() )
        {
            OUString sBackedUp( MacroMigrationResId( STR_SAVED_COPY_TO ) );
            sBackedUp = sBackedUp.replaceAll( "$location$", m_pData->sBackupLocation );

            aBuffer.append( "=== " + OUString( MacroMigrationResId( STR_DATABASE_DOCUMENT ) )
                    + " ===\n" + sBackedUp + "\n\n");
        }

        if ( !m_pData->aFailures.empty() )
        {
            lcl_describeErrors( aBuffer, m_pData->aFailures
                , STR_ERRORS );
        }
        else
        {
            OUString sMovedLibTemplate( MacroMigrationResId( STR_MOVED_LIBRARY ) );

            for (   DocumentLogs::const_iterator doc = m_pData->aDocumentLogs.begin();
                    doc != m_pData->aDocumentLogs.end();
                    ++doc
                )
            {
                const DocumentEntry& rDoc( doc->second );

                if ( rDoc.aMovedLibraries.empty() )
                    continue;

                OUString sDocTitle( MacroMigrationResId( rDoc.eType == eForm ? STR_FORM : STR_REPORT ) );
                sDocTitle = sDocTitle.replaceAll( "$name$", rDoc.sName );

                aBuffer.append( "=== " + sDocTitle + " ===\n" );

                for (   ::std::vector< LibraryEntry >::const_iterator lib = rDoc.aMovedLibraries.begin();
                        lib != rDoc.aMovedLibraries.end();
                        ++lib
                    )
                {
                    OUString sMovedLib( sMovedLibTemplate );
                    sMovedLib = sMovedLib.replaceAll( "$type$", getScriptTypeDisplayName( lib->eType ) );
                    sMovedLib = sMovedLib.replaceAll( "$old$", lib->sOldName );
                    sMovedLib = sMovedLib.replaceAll( "$new$", lib->sNewName );

                    aBuffer.append( sMovedLib + "\n" );
                }

                aBuffer.append( sal_Unicode( '\n' ) );
            }
        }

        if ( !m_pData->aWarnings.empty() )
        {
            lcl_describeErrors( aBuffer, m_pData->aWarnings, STR_WARNINGS );
        }

        return aBuffer.makeStringAndClear();
    }

//........................................................................
} // namespace dbmm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
