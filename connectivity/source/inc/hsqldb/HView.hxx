/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HView.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 15:06:08 $
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

#ifndef CONNECTIVITY_HVIEW_HXX
#define CONNECTIVITY_HVIEW_HXX

#include "connectivity/sdbcx/VView.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/sdbcx/XAlterView.hpp>
/** === end UNO includes === **/

#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase1.hxx>

//........................................................................
namespace connectivity { namespace hsqldb
{
//........................................................................

    //====================================================================
    //= HView
    //====================================================================
    typedef ::connectivity::sdbcx::OView                                HView_Base;
    typedef ::cppu::ImplHelper1< ::com::sun::star::sdbcx::XAlterView >  HView_IBASE;
    class HView :public HView_Base
                ,public HView_IBASE
    {
    public:
        HView(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            sal_Bool _bCaseSensitive,
            const ::rtl::OUString& _rSchemaName,
            const ::rtl::OUString& _rName
        );

        // UNO
        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        // XAlterView
        virtual void SAL_CALL alterCommand( const ::rtl::OUString& NewCommand ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~HView();

    protected:
        // OPropertyContainer
        virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& _rValue, sal_Int32 _nHandle ) const;

    private:
        /** retrieves the current command of the View

            @throws ::com::sun::star::lang::WrappedTargetException
                if an error occurs while retrieving the command from the database and
                <arg>_bAllowSQLExceptin</arg> is <FALSE/>
            @throws ::com::sun::star::sdbc::SQLException
                if an error occurs while retrieving the command from the database and
                <arg>_bAllowSQLException</arg> is <TRUE/>
        */
        ::rtl::OUString impl_getCommand_throw( bool _bAllowSQLException ) const;

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;
    private:
        using HView_Base::getFastPropertyValue;
    };

//........................................................................
} } // namespace connectivity::hsqldb
//........................................................................

#endif // CONNECTIVITY_HVIEW_HXX
