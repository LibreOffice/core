/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachedrowset.hxx,v $
 * $Revision: 1.4 $
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

#ifndef CACHEDROWSET_HXX
#define CACHEDROWSET_HXX

/** === begin UNO includes === **/
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <unotools/sharedunocomponent.hxx>

#include <memory>

//........................................................................
namespace frm
{
//........................................................................

    struct CachedRowSet_Data;
    //====================================================================
    //= CachedRowSet
    //====================================================================
    /** caches a result set obtained from a SQL statement
    */
    class CachedRowSet
    {
    public:
        CachedRowSet( const ::comphelper::ComponentContext& _rContext );
        ~CachedRowSet();

    public:
        /** executes the statement

            @return
                the result set produced by the statement. The caller takes ownership of the
                given object.

            @throws ::com::sun::star::sdbc::SQLException
                if such an exception is thrown when executing the statement
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >
                execute();

        /// determines whether the row set properties are dirty, i.e. have changed since the last call to execute
        bool    isDirty() const;

        /// disposes the instance and frees all associated resources
        void    dispose();

        /** sets the command of a query as command to be executed

            A connection must have been set before.

            @throws Exception
        */
        void    setCommandFromQuery ( const ::rtl::OUString& _rQueryName );

        void    setCommand          ( const ::rtl::OUString& _rCommand );
        void    setEscapeProcessing ( const sal_Bool _bEscapeProcessing );
        void    setConnection       ( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection );

    private:
        ::std::auto_ptr< CachedRowSet_Data >    m_pData;
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // CACHEDROWSET_HXX
