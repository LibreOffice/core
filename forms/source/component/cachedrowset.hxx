/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachedrowset.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-27 16:13:34 $
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
