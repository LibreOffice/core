/*************************************************************************
 *
 *  $RCSfile: conncleanup.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-06-21 14:13:24 $
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
#include <connectivity/conncleanup.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//.........................................................................
namespace dbtools
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::lang;

    //=====================================================================
    static ::rtl::OUString getActiveConnectionPropertyName()
    {
        static ::rtl::OUString s_sActiveConnectionPropertyName = ::rtl::OUString::createFromAscii("ActiveConnection");
        return s_sActiveConnectionPropertyName;
    }

    //=====================================================================
    //= OAutoConnectionDisposer
    //=====================================================================
    //---------------------------------------------------------------------
    OAutoConnectionDisposer::OAutoConnectionDisposer(const Reference< XRowSet >& _rxRowSet, const Reference< XConnection >& _rxConnection)
        : m_xRowSet(_rxRowSet)
        ,m_bWasAttached(sal_False)
    {
        Reference< XPropertySet > xProps(_rxRowSet, UNO_QUERY);
        OSL_ENSURE(xProps.is(), "OAutoConnectionDisposer::OAutoConnectionDisposer: invalid rowset (no XPropertySet)!");

        if (!xProps.is())
            return;

        try
        {
            xProps->setPropertyValue(getActiveConnectionPropertyName(), makeAny(_rxConnection));
            m_xOriginalConnection = _rxConnection;
            xProps->addPropertyChangeListener(getActiveConnectionPropertyName(), this);
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OAutoConnectionDisposer::OAutoConnectionDisposer: caught an exception!");
        }
    }

    //---------------------------------------------------------------------
    void OAutoConnectionDisposer::detach(const EventObject& _rReason)
    {
        // prevent deletion of ourself while we're herein
        Reference< XInterface > xKeepAlive(static_cast< XWeak* >(this));

        try
        {
            // remove ourself as property change listener
            Reference< XPropertySet > xProps(_rReason.Source, UNO_QUERY);
            OSL_ENSURE(xProps.is(), "OAutoConnectionDisposer::detach: invalid event source (no XPropertySet)!");
            xProps->removePropertyChangeListener(getActiveConnectionPropertyName(), this);
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OAutoConnectionDisposer::detach: caught an exception!");
        }
    }

    //---------------------------------------------------------------------
    void SAL_CALL OAutoConnectionDisposer::propertyChange( const PropertyChangeEvent& _rEvent ) throw (RuntimeException)
    {
        if (_rEvent.PropertyName.equals(getActiveConnectionPropertyName()))
        {   // somebody set a new ActiveConnection
#ifdef _DEBUG
            Reference< XConnection > xOldConnection;
            _rEvent.OldValue >>= xOldConnection;
            OSL_ENSURE(xOldConnection.get() == m_xOriginalConnection.get(), "OAutoConnectionDisposer::propertyChange: unexpected (original) property value!");
#endif
            try
            {
                // add as listener
                m_xRowSet->addRowSetListener(this);
            }
            catch(Exception&)
            {
                OSL_ENSURE(sal_False, "OAutoConnectionDisposer::propertyChange: caught an exception!");
            }
            m_bWasAttached = sal_True;
            // detach
            detach(_rEvent);
        }
    }

    //---------------------------------------------------------------------
    void SAL_CALL OAutoConnectionDisposer::disposing( const EventObject& _rSource ) throw (RuntimeException)
    {
        // the rowset is beeing disposed, and nobody has set a new ActiveConnection in the meantime
        try
        {
            if(m_bWasAttached)
                m_xRowSet->removeRowSetListener(this);
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OAutoConnectionDisposer::disposing: caught an exception!");
        }
        clearConnection();
        detach(_rSource);
    }
    //---------------------------------------------------------------------
    void OAutoConnectionDisposer::clearConnection()
    {
        try
        {
        // dispose the old connection
            Reference< XComponent > xComp(m_xOriginalConnection, UNO_QUERY);
            if (xComp.is())
                xComp->dispose();
            m_xOriginalConnection.clear();
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OAutoConnectionDisposer::clearConnection: caught an exception!");
        }
    }
    //---------------------------------------------------------------------
    void SAL_CALL OAutoConnectionDisposer::cursorMoved( const ::com::sun::star::lang::EventObject& event ) throw (::com::sun::star::uno::RuntimeException)
    {
    }
    //---------------------------------------------------------------------
    void SAL_CALL OAutoConnectionDisposer::rowChanged( const ::com::sun::star::lang::EventObject& event ) throw (::com::sun::star::uno::RuntimeException)
    {
    }
    //---------------------------------------------------------------------
    void SAL_CALL OAutoConnectionDisposer::rowSetChanged( const ::com::sun::star::lang::EventObject& event ) throw (::com::sun::star::uno::RuntimeException)
    {
        try
        {
            m_xRowSet->removeRowSetListener(this);
            m_bWasAttached = sal_False;
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OAutoConnectionDisposer::rowSetChanged: caught an exception!");
        }
        clearConnection();

    }
    //---------------------------------------------------------------------

//.........................................................................
}   // namespace dbtools
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2001/04/12 09:48:11  fs
 *  initial checkin - helper for automatically disposing a rowset's connection
 *
 *
 *  Revision 1.0 12.04.01 09:36:29  fs
 ************************************************************************/

