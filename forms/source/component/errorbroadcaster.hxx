/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: errorbroadcaster.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 11:12:58 $
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

#ifndef FORMS_ERRORBROADCASTER_HXX
#define FORMS_ERRORBROADCASTER_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLERRORBROADCASTER_HPP_
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLEXCEPTION_HPP_
#include <com/sun/star/sdbc/SQLException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLERROREVENT_HPP_
#include <com/sun/star/sdb/SQLErrorEvent.hpp>
#endif

//.........................................................................
namespace frm
{
//.........................................................................

    //=====================================================================
    //= OErrorBroadcaster
    //=====================================================================
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::sdb::XSQLErrorBroadcaster
                                >   OErrorBroadcaster_BASE;

    class OErrorBroadcaster : public OErrorBroadcaster_BASE
    {
    private:
        ::cppu::OBroadcastHelper&           m_rBHelper;
        ::cppu::OInterfaceContainerHelper   m_aErrorListeners;

    protected:
        OErrorBroadcaster( ::cppu::OBroadcastHelper& _rBHelper );
        virtual ~OErrorBroadcaster( );

        void SAL_CALL disposing();

        void SAL_CALL onError( const ::com::sun::star::sdbc::SQLException& _rException, const ::rtl::OUString& _rContextDescription );
        void SAL_CALL onError( const ::com::sun::star::sdb::SQLErrorEvent& _rException );

    protected:
    // XSQLErrorBroadcaster
        virtual void SAL_CALL addSQLErrorListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLErrorListener >& _rListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeSQLErrorListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLErrorListener >& _rListener ) throw (::com::sun::star::uno::RuntimeException);
    };

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // FORMS_ERRORBROADCASTER_HXX

