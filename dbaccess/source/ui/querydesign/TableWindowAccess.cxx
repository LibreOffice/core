/*************************************************************************
 *
 *  $RCSfile: TableWindowAccess.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:22:58 $
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
#ifndef DBACCESS_TABLEWINDOWACCESS_HXX
#include "TableWindowAccess.hxx"
#endif
#ifndef DBACCESS_JACCESS_HXX
#include "JAccess.hxx"
#endif
#ifndef DBAUI_TABLEWINDOW_HXX
#include "TableWindow.hxx"
#endif
#ifndef DBAUI_TABLEWINDOWLISTBOX_HXX
#include "TableWindowListBox.hxx"
#endif
#ifndef DBAUI_JOINDESIGNVIEW_HXX
#include "JoinDesignView.hxx"
#endif
#ifndef DBAUI_JOINCONTROLLER_HXX
#include "JoinController.hxx"
#endif
#ifndef DBAUI_JOINTABLEVIEW_HXX
#include "JoinTableView.hxx"
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLERELATIONTYPE_HPP_
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
#endif


namespace dbaui
{
    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    //  using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star;

    OTableWindowAccess::OTableWindowAccess(const Reference< XAccessible>& _xParent,
                                            OTableWindow* _pTable)
        :OAccessibleBase(_pTable,_xParent)
        ,m_pTable(_pTable)
    {
        OSL_ENSURE(m_pTable,"Table isn't valid!");
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL OTableWindowAccess::disposing()
    {
        m_pTable = NULL;
        OAccessibleBase::disposing();
    }
    // -----------------------------------------------------------------------------
    Any SAL_CALL OTableWindowAccess::queryInterface( const Type& aType ) throw (RuntimeException)
    {
        Any aRet(OAccessibleBase::queryInterface( aType ));
        return aRet.hasValue() ? aRet : OTableWindowAccess_BASE::queryInterface( aType );
    }
    // -----------------------------------------------------------------------------
    Sequence< Type > SAL_CALL OTableWindowAccess::getTypes(  ) throw (RuntimeException)
    {
        return ::comphelper::concatSequences(OAccessibleBase::getTypes(),OTableWindowAccess_BASE::getTypes());
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OTableWindowAccess::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }
    // -----------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL OTableWindowAccess::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }
    // -----------------------------------------------------------------------------
    // XServiceInfo - static methods
    Sequence< ::rtl::OUString > OTableWindowAccess::getSupportedServiceNames_Static(void) throw( RuntimeException )
    {
        Sequence< ::rtl::OUString > aSupported(2);
        aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.accessibility.Accessible");
        aSupported[1] = ::rtl::OUString::createFromAscii("com.sun.star.accessibility.AccessibleContext");
        return aSupported;
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString OTableWindowAccess::getImplementationName_Static(void) throw( RuntimeException )
    {
        return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbu.TableWindowAccessibility");
    }
    // -----------------------------------------------------------------------------
    // XAccessibleContext
    sal_Int32 SAL_CALL OTableWindowAccess::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        return 2;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL OTableWindowAccess::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException,RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        Reference< XAccessible > aRet;
        if(i == 0 && m_pTable)
            aRet = m_pTable->GetTitleCtrl()->GetAccessible();
        else if(i == 1 && m_pTable)
            aRet = m_pTable->GetListBox()->GetAccessible();
        else
            throw IndexOutOfBoundsException();
        return aRet;
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL OTableWindowAccess::getAccessibleIndexInParent(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        sal_Int32 nIndex = -1;
        if( m_pTable )
        {
            // search the postion of our table window in the table window map
            OJoinTableView::OTableWindowMap* pMap = m_pTable->getTableView()->GetTabWinMap();
            OJoinTableView::OTableWindowMap::iterator aIter = pMap->begin();
            for (nIndex = 0; aIter != pMap->end() && aIter->second != m_pTable; ++nIndex,++aIter)
                ;
            nIndex = aIter != pMap->end() ? nIndex : -1;
        }
        return nIndex;
    }
    // -----------------------------------------------------------------------------
    sal_Int16 SAL_CALL OTableWindowAccess::getAccessibleRole(  ) throw (RuntimeException)
    {
        return AccessibleRole::PANEL; // ? or may be an AccessibleRole::WINDOW
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessibleRelationSet > SAL_CALL OTableWindowAccess::getAccessibleRelationSet(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        return this;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleComponent
    Reference< XAccessible > SAL_CALL OTableWindowAccess::getAccessibleAtPoint( const awt::Point& _aPoint ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        Reference< XAccessible > aRet;
        if( m_pTable )
        {
            Point aPoint(_aPoint.X,_aPoint.Y);
            Rectangle aRect(m_pTable->GetDesktopRectPixel());
            if( aRect.IsInside(aPoint) )
                aRet = this;
            else if( m_pTable->GetListBox()->GetDesktopRectPixel().IsInside(aPoint))
                aRet = m_pTable->GetListBox()->GetAccessible();
        }
        return aRet;
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL OTableWindowAccess::getRelationCount(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        return m_pTable ? m_pTable->getTableView()->getConnectionCount(m_pTable) : sal_Int32(0);
    }
    // -----------------------------------------------------------------------------
    AccessibleRelation SAL_CALL OTableWindowAccess::getRelation( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        if( nIndex < 0 || nIndex >= getRelationCount() )
            throw IndexOutOfBoundsException();

        AccessibleRelation aRet;
        if( m_pTable )
        {
            OJoinTableView* pView = m_pTable->getTableView();
            ::std::vector<OTableConnection*>::const_iterator aIter = pView->getTableConnections(m_pTable) + nIndex;
            aRet.TargetSet.realloc(1);
            aRet.TargetSet[0] = m_xParentContext->getAccessibleChild(aIter - pView->getTableConnections()->begin());
            aRet.RelationType = AccessibleRelationType::CONTROLLER_FOR;
        }
        return aRet;
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL OTableWindowAccess::containsRelation( sal_Int16 aRelationType ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        return      AccessibleRelationType::CONTROLLER_FOR == aRelationType
                &&  m_pTable->getTableView()->ExistsAConn(m_pTable);
    }
    // -----------------------------------------------------------------------------
    AccessibleRelation SAL_CALL OTableWindowAccess::getRelationByType( sal_Int16 aRelationType ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        if( AccessibleRelationType::CONTROLLER_FOR == aRelationType && m_pTable)
        {
            OJoinTableView* pView = m_pTable->getTableView();
            const ::std::vector<OTableConnection*>* pConnectionList = pView->getTableConnections();

            ::std::vector<OTableConnection*>::const_iterator aIter = pView->getTableConnections(m_pTable);
            ::std::vector< Reference<XInterface> > aRelations;
            aRelations.reserve(5); // just guessing
            for (; aIter != pConnectionList->end() ; ++aIter )
                aRelations.push_back(m_xParentContext->getAccessibleChild(aIter - pConnectionList->begin()));

            Sequence< Reference<XInterface> > aSeq(aRelations.begin(),aRelations.size());
            return AccessibleRelation(AccessibleRelationType::CONTROLLER_FOR,aSeq);
        }
        return AccessibleRelation();
    }
    // -----------------------------------------------------------------------------
    sal_Bool OTableWindowAccess::isEditable() const
    {
        return m_pTable && !m_pTable->getTableView()->getDesignView()->getController()->isReadOnly();
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OTableWindowAccess::getTitledBorderText(  ) throw (RuntimeException)
    {
        return getAccessibleName(  );
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OTableWindowAccess::getAccessibleName(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        ::rtl::OUString sAccessibleName;
        if ( m_pTable )
            sAccessibleName = m_pTable->getTitle();
        return sAccessibleName;
    }
    // -----------------------------------------------------------------------------
}
// -----------------------------------------------------------------------------

