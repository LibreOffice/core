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

#ifndef FORMS_ERRORBROADCASTER_HXX
#define FORMS_ERRORBROADCASTER_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/sdbc/SQLException.hpp>
namespace binfilter {

//.........................................................................
namespace frm
{
//.........................................................................

    //=====================================================================
    //= OErrorBroadcaster
    //=====================================================================
    typedef ::cppu::ImplHelper1	<	::com::sun::star::sdb::XSQLErrorBroadcaster
                                >	OErrorBroadcaster_BASE;

    class OErrorBroadcaster : public OErrorBroadcaster_BASE
    {
    private:
        ::cppu::OBroadcastHelper&			m_rBHelper;
        ::cppu::OInterfaceContainerHelper	m_aErrorListeners;

    protected:
        OErrorBroadcaster( ::cppu::OBroadcastHelper& _rBHelper );
        ~OErrorBroadcaster( );

        void SAL_CALL disposing();

        void SAL_CALL onError( const ::com::sun::star::sdbc::SQLException& _rException, const ::rtl::OUString& _rContextDescription );
        void SAL_CALL onError( const ::com::sun::star::sdb::SQLErrorEvent& _rException );

    protected:
    // XSQLErrorBroadcaster
        virtual void SAL_CALL addSQLErrorListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLErrorListener >& _rListener ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeSQLErrorListener( const ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLErrorListener >& _rListener ) throw (::com::sun::star::uno::RuntimeException);
    };

//.........................................................................
}	// namespace frm
//.........................................................................

}//end of namespace binfilter
#endif // FORMS_ERRORBROADCASTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
