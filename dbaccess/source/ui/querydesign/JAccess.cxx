/*************************************************************************
 *
 *  $RCSfile: JAccess.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:22:05 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBACCESS_JACCESS_HXX
#include "JAccess.hxx"
#endif
#ifndef DBAUI_JOINTABLEVIEW_HXX
#include "JoinTableView.hxx"
#endif
#ifndef DBAUI_JOINTABLEVIEW_HXX
#include "JoinTableView.hxx"
#endif
#ifndef DBAUI_TABLEWINDOW_HXX
#include "TableWindow.hxx"
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef DBAUI_JOINDESIGNVIEW_HXX
#include "JoinDesignView.hxx"
#endif
#ifndef DBAUI_JOINCONTROLLER_HXX
#include "JoinController.hxx"
#endif
#ifndef DBAUI_TABLECONNECTION_HXX
#include "TableConnection.hxx"
#endif

namespace dbaui
{
    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;

    OJoinDesignViewAccess::OJoinDesignViewAccess(OJoinTableView* _pTableView,
                                                 const Reference< XAccessible >& _xParent)
        :OAccessibleBase(_pTableView,_xParent)
        ,m_pTableView(_pTableView)
    {
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OJoinDesignViewAccess::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString OJoinDesignViewAccess::getImplementationName_Static(void) throw( RuntimeException )
    {
        return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbu.JoinViewAccessibility");
    }
    // -----------------------------------------------------------------------------
    // XAccessibleContext
    sal_Int32 SAL_CALL OJoinDesignViewAccess::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        // TODO may be this will change to only visible windows
        // this is the same assumption mt implements
        ::osl::MutexGuard aGuard( m_aMutex  );
        sal_Int32 nChildCount = 0;
        if ( m_pTableView )
            nChildCount = m_pTableView->GetTabWinCount() + m_pTableView->getTableConnections()->size();
        return nChildCount;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL OJoinDesignViewAccess::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException,RuntimeException)
    {
        Reference< XAccessible > aRet;
        ::osl::MutexGuard aGuard( m_aMutex  );
        if(i >= 0 && i < getAccessibleChildCount() && m_pTableView )
        {
            // check if we should return a table window or a connection
            sal_Int32 nTableWindowCount = m_pTableView->GetTabWinCount();
            if( i < nTableWindowCount )
            {
                OJoinTableView::OTableWindowMap::iterator aIter = m_pTableView->GetTabWinMap()->begin();
                for (sal_Int32 j=i; j; ++aIter,--j)
                    ;
                aRet = aIter->second->GetAccessible();
            }
            else if( size_t(i - nTableWindowCount) < m_pTableView->getTableConnections()->size() )
                aRet = (*m_pTableView->getTableConnections())[i - nTableWindowCount]->getAccessible();
        }
        else
            throw IndexOutOfBoundsException();
        return aRet;
    }
    // -----------------------------------------------------------------------------
    sal_Bool OJoinDesignViewAccess::isEditable() const
    {
        return m_pTableView && !m_pTableView->getDesignView()->getController()->isReadOnly();
    }
    // -----------------------------------------------------------------------------
    sal_Int16 SAL_CALL OJoinDesignViewAccess::getAccessibleRole(  ) throw (RuntimeException)
    {
        return AccessibleRole::VIEW_PORT;
    }
    // -----------------------------------------------------------------------------
}

// -----------------------------------------------------------------------------

