/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: migrationengine.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:05:12 $
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
