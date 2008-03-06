/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: migrationlog.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:05:37 $
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
        void        finishedDocument( const DocumentID _nDocID, const bool _bSuccessful );

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
