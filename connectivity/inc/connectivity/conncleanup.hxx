/*************************************************************************
 *
 *  $RCSfile: conncleanup.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:38:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_CONNCLEANUP_HXX_
#define _CONNECTIVITY_CONNCLEANUP_HXX_

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif

//.........................................................................
namespace dbtools
{
//.........................................................................

    //=====================================================================
    //= OAutoConnectionDisposer
    //=====================================================================
    typedef ::cppu::WeakImplHelper2 <   ::com::sun::star::beans::XPropertyChangeListener,
                                        ::com::sun::star::sdbc::XRowSetListener
                                    >   OAutoConnectionDisposer_Base;

    class OAutoConnectionDisposer : public OAutoConnectionDisposer_Base
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                    m_xOriginalConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > m_xRowSet; // needed to add as listener
        sal_Bool    m_bRSListening          : 1; // true when we're listening on rowset
        sal_Bool    m_bPropertyListening    : 1; // true when we're listening for property changes

    public:
        /** constructs an object
            <p>The connection given will be set on the rowset (as ActiveConnection), and the object adds itself as property
            change listener for the connection. Once somebody sets a new ActiveConnection, the old one (the one given
            here) will be disposed.</p>
        */
        OAutoConnectionDisposer(
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rxRowSet,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
            );

    protected:
        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& _rEvent ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException);

        // XRowSetListener
        virtual void SAL_CALL cursorMoved( const ::com::sun::star::lang::EventObject& event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL rowChanged( const ::com::sun::star::lang::EventObject& event ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL rowSetChanged( const ::com::sun::star::lang::EventObject& event ) throw (::com::sun::star::uno::RuntimeException);

    private:
        void clearConnection();

        void        startRowSetListening();
        void        stopRowSetListening();
        sal_Bool    isRowSetListening() const { return m_bRSListening; }

        void        startPropertyListening( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxProps );
        void        stopPropertyListening( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxEventSource );
        sal_Bool    isPropertyListening() const { return m_bPropertyListening; }
    };

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // _CONNECTIVITY_CONNCLEANUP_HXX_

