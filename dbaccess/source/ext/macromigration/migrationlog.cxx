/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: migrationlog.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-10 12:44:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "dbmm_module.hxx"
#include "dbmm_global.hrc"
#include "migrationlog.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <tools/string.hxx>
#include <rtl/ustrbuf.hxx>

#include <vector>
#include <map>

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
        bool                            bSuccess;

        DocumentEntry()
            :eType( eForm )
            ,sName()
            ,aMovedLibraries()
            ,bSuccess( false )
        {
        }

        DocumentEntry( const SubDocumentType _eType, const ::rtl::OUString& _rName )
            :eType( _eType )
            ,sName( _rName )
            ,bSuccess( false )
        {
        }
    };

    //====================================================================
    //= DocumentLogs
    //====================================================================
    typedef ::std::map< DocumentID, DocumentEntry > DocumentLogs;

    //====================================================================
    //= MigrationLog_Data
    //====================================================================
    struct MigrationLog_Data
    {
        ::rtl::OUString sBackupLocation;
        DocumentLogs    aDocumentLogs;
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
    void MigrationLog::finishedDocument( const DocumentID _nDocID, const bool _bSuccessful )
    {
        OSL_ENSURE( m_pData->aDocumentLogs.find( _nDocID ) != m_pData->aDocumentLogs.end(),
            "MigrationLog::finishedDocument: document is not known!" );

        DocumentEntry& rDocEntry = m_pData->aDocumentLogs[ _nDocID ];
        rDocEntry.bSuccess = _bSuccessful;
    }

    //--------------------------------------------------------------------
    const ::rtl::OUString& MigrationLog::getNewLibraryName( DocumentID _nDocID, ScriptType _eScriptType,
        const ::rtl::OUString& _rOriginalLibName ) const
    {
        static ::rtl::OUString s_sEmptyString;

        DocumentLogs::const_iterator docPos = m_pData->aDocumentLogs.find( _nDocID );
        if ( docPos == m_pData->aDocumentLogs.end() )
        {
            OSL_ENSURE( false, "MigrationLog::getNewLibraryName: document is not known!" );
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

        OSL_ENSURE( false, "MigrationLog::getNewLibraryName: doc is known, but library isn't!" );
        return s_sEmptyString;
    }

    //--------------------------------------------------------------------
    namespace
    {
        String lcl_getScriptTypeDisplayName( const ScriptType _eType )
        {
            USHORT nResId( 0 );

            switch ( _eType )
            {
            case eBasic:        nResId = STR_OOO_BASIC;     break;
            case eBeanShell:    nResId = STR_BEAN_SHELL;    break;
            case eJavaScript:   nResId = STR_JAVA_SCRIPT;   break;
            case ePython:       nResId = STR_PYTHON;        break;
            case eJava:         nResId = STR_JAVA;          break;
            case eDialog:       nResId = STR_DIALOG;        break;
            }
            OSL_ENSURE( nResId != 0, "lcl_getScriptTypeDisplayName: illegal script type!" );
            return nResId ? String( MacroMigrationResId( nResId ) ) : String();
        }
    }

    //--------------------------------------------------------------------
    ::rtl::OUString MigrationLog::getCompleteLog() const
    {
        ::rtl::OUStringBuffer aBuffer;

        String sBackedUp( MacroMigrationResId( STR_SAVED_COPY_TO ) );
        sBackedUp.SearchAndReplaceAllAscii( "$location$", m_pData->sBackupLocation );

        aBuffer.appendAscii( "=== " );
        aBuffer.append     ( String( MacroMigrationResId( STR_DATABASE_DOCUMENT ) ) );
        aBuffer.appendAscii( " ===\n" );
        aBuffer.append     ( sBackedUp );
        aBuffer.appendAscii( "\n\n" );

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
                sMovedLib.SearchAndReplaceAllAscii( "$type$", lcl_getScriptTypeDisplayName( lib->eType ) );
                sMovedLib.SearchAndReplaceAllAscii( "$old$", lib->sOldName );
                sMovedLib.SearchAndReplaceAllAscii( "$new$", lib->sNewName );

                aBuffer.append     ( sMovedLib );
                aBuffer.appendAscii( "\n" );
            }

            if ( !rDoc.bSuccess )
            {
                // TODO
            }

            aBuffer.appendAscii( "\n" );
        }

        return aBuffer.makeStringAndClear();
    }

//........................................................................
} // namespace dbmm
//........................................................................
