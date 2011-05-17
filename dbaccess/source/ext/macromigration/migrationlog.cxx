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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "dbmm_module.hxx"
#include "dbmm_global.hrc"
#include "migrationerror.hxx"
#include "migrationlog.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <comphelper/anytostring.hxx>
#include <comphelper/string.hxx>
#include <tools/string.hxx>
#include <rtl/ustrbuf.hxx>

#include <vector>
#include <map>
#include <list>

//........................................................................
namespace dbmm
{
//........................................................................

    /** === begin UNO using === **/
    /** === end UNO using === **/

    //====================================================================
    //= LibraryEntry
    //====================================================================
    struct LibraryEntry
    {
        ScriptType      eType;
        ::rtl::OUString sOldName;
        ::rtl::OUString sNewName;

        LibraryEntry()
            :eType( eBasic )
            ,sOldName()
            ,sNewName()
        {
        }

        LibraryEntry( const ScriptType& _eType, const ::rtl::OUString& _rOldName, const ::rtl::OUString& _rNewName )
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
        ::rtl::OUString                 sName;
        ::std::vector< LibraryEntry >   aMovedLibraries;

        DocumentEntry()
            :eType( eForm )
            ,sName()
            ,aMovedLibraries()
        {
        }

        DocumentEntry( const SubDocumentType _eType, const ::rtl::OUString& _rName )
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
        ::rtl::OUString sBackupLocation;
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
    void MigrationLog::backedUpDocument( const ::rtl::OUString& _rNewDocumentLocation )
    {
        m_pData->sBackupLocation = _rNewDocumentLocation;
    }

    //--------------------------------------------------------------------
    DocumentID MigrationLog::startedDocument( const SubDocumentType _eType, const ::rtl::OUString& _rName )
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
            const ::rtl::OUString& _rOriginalLibName, const ::rtl::OUString& _rNewLibName )
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
    const ::rtl::OUString& MigrationLog::getNewLibraryName( DocumentID _nDocID, ScriptType _eScriptType,
        const ::rtl::OUString& _rOriginalLibName ) const
    {
        static ::rtl::OUString s_sEmptyString;

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
        static void lcl_appendErrorDescription( ::rtl::OUStringBuffer& _inout_rBuffer, const MigrationError& _rError )
        {
            const sal_Char* pAsciiErrorDescription( NULL );
            ::std::vector< const sal_Char* > aAsciiParameterNames;
            switch ( _rError.eType )
            {
            case ERR_OPENING_SUB_DOCUMENT_FAILED:
                pAsciiErrorDescription = "opening '#doc#' failed";
                aAsciiParameterNames.push_back( "#doc#" );
                break;

            case ERR_CLOSING_SUB_DOCUMENT_FAILED:
                pAsciiErrorDescription = "closing '#doc#' failed";
                aAsciiParameterNames.push_back( "#doc#" );
                break;

            case ERR_STORAGE_COMMIT_FAILED:
                pAsciiErrorDescription = "committing the changes for document '#doc#' failed";
                aAsciiParameterNames.push_back( "#doc#" );
                break;

            case ERR_STORING_DATABASEDOC_FAILED:
                pAsciiErrorDescription = "storing the database document failed";
                break;

            case ERR_COLLECTING_DOCUMENTS_FAILED:
                pAsciiErrorDescription = "collecting the forms/reports of the database document failed";
                break;

            case ERR_UNEXPECTED_LIBSTORAGE_ELEMENT:
                pAsciiErrorDescription = "unexpected #lib# storage element in document '#doc#', named '#element#'";
                aAsciiParameterNames.push_back( "#doc#" );
                aAsciiParameterNames.push_back( "#libstore#" );
                aAsciiParameterNames.push_back( "#element#" );
                break;

            case ERR_CREATING_DBDOC_SCRIPT_STORAGE_FAILED:
                pAsciiErrorDescription = "creating the database document's storage for #scripttype# scripts failed";
                aAsciiParameterNames.push_back( "#scripttype#" );
                break;

            case ERR_COMMITTING_SCRIPT_STORAGES_FAILED:
                pAsciiErrorDescription = "saving the #scripttype# scripts for document '#doc#' failed";
                aAsciiParameterNames.push_back( "#scripttype#" );
                aAsciiParameterNames.push_back( "#doc#" );
                break;

            case ERR_GENERAL_SCRIPT_MIGRATION_FAILURE:
                pAsciiErrorDescription = "general error while migrating #scripttype# scripts of document '#doc#'";
                aAsciiParameterNames.push_back( "#scripttype#" );
                aAsciiParameterNames.push_back( "#doc#" );
                break;

            case ERR_GENERAL_MACRO_MIGRATION_FAILURE:
                pAsciiErrorDescription = "general error during macro migration of document '#doc#'";
                aAsciiParameterNames.push_back( "#doc#" );
                break;

            case ERR_UNKNOWN_SCRIPT_TYPE:
                pAsciiErrorDescription = "unknown script type: #type#";
                aAsciiParameterNames.push_back( "#type#" );
                break;

            case ERR_UNKNOWN_SCRIPT_LANGUAGE:
                pAsciiErrorDescription = "unknown script language: #lang#";
                aAsciiParameterNames.push_back( "#lang#" );
                break;

            case ERR_UNKNOWN_SCRIPT_NAME_FORMAT:
                pAsciiErrorDescription = "unknown script name format: #script#";
                aAsciiParameterNames.push_back( "#script#" );
                break;

            case ERR_SCRIPT_TRANSLATION_FAILURE:
                pAsciiErrorDescription = "analyzing/translating the script URL failed; script type: #type#; script: #code#";
                aAsciiParameterNames.push_back( "#type#" );
                aAsciiParameterNames.push_back( "#code#" );
                break;

            case ERR_INVALID_SCRIPT_DESCRIPTOR_FORMAT:
                pAsciiErrorDescription = "invalid script descriptor format";
                break;

            case ERR_ADJUSTING_DOCUMENT_EVENTS_FAILED:
                pAsciiErrorDescription = "adjusting events for document '#doc#' failed";
                aAsciiParameterNames.push_back( "#doc#" );
                break;

            case ERR_ADJUSTING_DIALOG_EVENTS_FAILED:
                pAsciiErrorDescription = "adjusting events for dialog #lib#.#dlg# in document '#doc#' failed";
                aAsciiParameterNames.push_back( "#doc#" );
                aAsciiParameterNames.push_back( "#lib#" );
                aAsciiParameterNames.push_back( "#dlg#" );
                break;

            case ERR_ADJUSTING_FORMCOMP_EVENTS_FAILED:
                pAsciiErrorDescription = "adjusting form component events for '#doc#' failed";
                aAsciiParameterNames.push_back( "#doc#" );
                break;

            case ERR_BIND_SCRIPT_STORAGE_FAILED:
                pAsciiErrorDescription = "binding to the script storage failed for document '#doc#'";
                aAsciiParameterNames.push_back( "#doc#" );
                break;

            case ERR_REMOVE_SCRIPTS_STORAGE_FAILED:
                pAsciiErrorDescription = "removing a scripts storage failed for document '#doc#'";
                aAsciiParameterNames.push_back( "#doc#" );
                break;

            case ERR_DOCUMENT_BACKUP_FAILED:
                pAsciiErrorDescription = "backing up the document to #location# failed";
                aAsciiParameterNames.push_back( "#location#" );
                break;

            case ERR_UNKNOWN_SCRIPT_FOLDER:
                pAsciiErrorDescription = "unknown script folder '#name#' in document '#doc#'";
                aAsciiParameterNames.push_back( "#doc#" );
                aAsciiParameterNames.push_back( "#name#" );
                break;

            case ERR_EXAMINING_SCRIPTS_FOLDER_FAILED:
                pAsciiErrorDescription = "examining the 'Scripts' folder failed for document '#doc#'";
                aAsciiParameterNames.push_back( "#doc#" );
                break;

            case ERR_PASSWORD_VERIFICATION_FAILED:
                pAsciiErrorDescription = "password verification failed for document '#doc#', #libtype# library '#name#'";
                aAsciiParameterNames.push_back( "#doc#" );
                aAsciiParameterNames.push_back( "#libtype#" );
                aAsciiParameterNames.push_back( "#name#" );
                break;

            case ERR_NEW_STYLE_REPORT:
                pAsciiErrorDescription = "#doc# could not be processed, since you don't have the Oracle Report Builder (TM) extension installed.";
                aAsciiParameterNames.push_back( "#doc#" );
                break;

                // do *not* add a default case here: Without a default, some compilers will warn you when
                // you miss a newly-introduced enum value here
            }
            OSL_ENSURE( pAsciiErrorDescription, "lcl_appendErrorDescription: no error message!" );
            if ( pAsciiErrorDescription )
            {
                ::rtl::OUString sSubstituted( ::rtl::OUString::createFromAscii( pAsciiErrorDescription ) );
                OSL_ENSURE( aAsciiParameterNames.size() == _rError.aErrorDetails.size(),
                    "lcl_appendErrorDescription: unexpected number of error message parameters!" );

                for ( size_t i=0; i < ::std::min( aAsciiParameterNames.size(), _rError.aErrorDetails.size() ); ++i )
                {
                    ::comphelper::string::searchAndReplaceAsciiI( sSubstituted, aAsciiParameterNames[i],
                        _rError.aErrorDetails[i] );
                }

                _inout_rBuffer.append( sSubstituted );
            }
        }

        //----------------------------------------------------------------
        void lcl_describeErrors( ::rtl::OUStringBuffer& _rBuffer, const ErrorLog& _rErrors, const sal_uInt16 _nHeadingResId )
        {
            _rBuffer.appendAscii( "=== " );
            _rBuffer.append     ( String( MacroMigrationResId( _nHeadingResId ) ) );
            _rBuffer.appendAscii( " ===\n" );

            String sException( MacroMigrationResId( STR_EXCEPTION ) );

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
    ::rtl::OUString MigrationLog::getCompleteLog() const
    {
        ::rtl::OUStringBuffer aBuffer;

        if ( m_pData->sBackupLocation.getLength() )
        {
            String sBackedUp( MacroMigrationResId( STR_SAVED_COPY_TO ) );
            sBackedUp.SearchAndReplaceAllAscii( "$location$", m_pData->sBackupLocation );

            aBuffer.appendAscii( "=== " );
            aBuffer.append     ( String( MacroMigrationResId( STR_DATABASE_DOCUMENT ) ) );
            aBuffer.appendAscii( " ===\n" );
            aBuffer.append     ( sBackedUp );
            aBuffer.appendAscii( "\n\n" );
        }

        if ( !m_pData->aFailures.empty() )
        {
            lcl_describeErrors( aBuffer, m_pData->aFailures
                , STR_ERRORS );
        }
        else
        {
            String sMovedLibTemplate( MacroMigrationResId( STR_MOVED_LIBRARY ) );

            for (   DocumentLogs::const_iterator doc = m_pData->aDocumentLogs.begin();
                    doc != m_pData->aDocumentLogs.end();
                    ++doc
                )
            {
                const DocumentEntry& rDoc( doc->second );

                if ( rDoc.aMovedLibraries.empty() )
                    continue;

                String sDocTitle( MacroMigrationResId( rDoc.eType == eForm ? STR_FORM : STR_REPORT ) );
                sDocTitle.SearchAndReplaceAllAscii( "$name$", rDoc.sName );

                aBuffer.appendAscii( "=== " );
                aBuffer.append     ( sDocTitle );
                aBuffer.appendAscii( " ===\n" );

                for (   ::std::vector< LibraryEntry >::const_iterator lib = rDoc.aMovedLibraries.begin();
                        lib != rDoc.aMovedLibraries.end();
                        ++lib
                    )
                {
                    String sMovedLib( sMovedLibTemplate );
                    sMovedLib.SearchAndReplaceAllAscii( "$type$", getScriptTypeDisplayName( lib->eType ) );
                    sMovedLib.SearchAndReplaceAllAscii( "$old$", lib->sOldName );
                    sMovedLib.SearchAndReplaceAllAscii( "$new$", lib->sNewName );

                    aBuffer.append( sMovedLib );
                    aBuffer.append( sal_Unicode( '\n' ) );
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
