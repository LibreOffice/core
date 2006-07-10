/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: warnings.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 15:15:50 $
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

#ifndef DBA_CORE_WARNINGS_HXX
#define DBA_CORE_WARNINGS_HXX

/** ==== begin UNO includes === **/
#ifndef _COM_SUN_STAR_SDBC_XWARNINGSSUPPLIER_HPP_
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
/** ==== end UNO includes === **/

//.........................................................................
namespace dbaccess
{
//.........................................................................

    //=====================================================================
    //= IWarningsContainer
    //=====================================================================
    class SAL_NO_VTABLE IWarningsContainer
    {
    public:
        virtual void appendWarning(const ::com::sun::star::sdbc::SQLException& _rWarning) = 0;
        virtual void appendWarning(const ::com::sun::star::sdbc::SQLWarning& _rWarning) = 0;
        virtual void appendWarning(const ::com::sun::star::sdb::SQLContext& _rContext) = 0;
    };

    //====================================================================
    //= WarningsContainer
    //====================================================================
    /** helper class for implementing XWarningsSupplier, which mixes own warnings with
        warnings obtained from an external instance
    */
    class WarningsContainer : public IWarningsContainer
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XWarningsSupplier >   m_xExternalWarnings;
        ::com::sun::star::uno::Any                                                      m_aOwnWarnings;

    public:
        WarningsContainer() { }
        WarningsContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XWarningsSupplier >& _rxExternalWarnings )
            :m_xExternalWarnings( _rxExternalWarnings )
        {
        }
        virtual ~WarningsContainer();

        // convenience
        /** appends an SQLWarning instance to the chain
            @param  _rWarning
                the warning message
            @param  _pAsciiSQLState
                the SQLState of the warning
            @param  _rxContext
                the context of the warning
        */
        void appendWarning(
            const ::rtl::OUString& _rWarning,
            const sal_Char* _pAsciiSQLState,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext );

        // IWarningsContainer
        virtual void appendWarning(const ::com::sun::star::sdbc::SQLException& _rWarning);
        virtual void appendWarning(const ::com::sun::star::sdbc::SQLWarning& _rWarning);
        virtual void appendWarning(const ::com::sun::star::sdb::SQLContext& _rContext);

        // XWarningsSupplier
        ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) const;
        void SAL_CALL clearWarnings(  );
    };

//.........................................................................
}   // namespace dbaccess
//.........................................................................

#endif // DBA_CORE_WARNINGS_HXX

