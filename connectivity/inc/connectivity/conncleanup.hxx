/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: conncleanup.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:00:26 $
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

