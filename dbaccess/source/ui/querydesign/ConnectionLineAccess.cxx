/*************************************************************************
 *
 *  $RCSfile: ConnectionLineAccess.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:12:02 $
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
#ifndef DBACCESS_CONNECTIONLINEACCESS_HXX
#include "ConnectionLineAccess.hxx"
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
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef DBAUI_TABLECONNECTION_HXX
#include "TableConnection.hxx"
#endif
#ifndef DBAUI_TABLEWINDOW_HXX
#include "TableWindow.hxx"
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef DBAUI_JOINDESIGNVIEW_HXX
#include "JoinDesignView.hxx"
#endif
#ifndef DBAUI_JOINCONTROLLER_HXX
#include "JoinController.hxx"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

namespace dbaui
{
    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    //  using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star;

    OConnectionLineAccess::OConnectionLineAccess(OTableConnection* _pLine)
        : VCLXAccessibleComponent(_pLine->GetComponentInterface().is() ? _pLine->GetWindowPeer() : NULL)
        ,m_pLine(_pLine)
    {
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL OConnectionLineAccess::disposing()
    {
        m_pLine = NULL;
        VCLXAccessibleComponent::disposing();
    }
    // -----------------------------------------------------------------------------
    Any SAL_CALL OConnectionLineAccess::queryInterface( const Type& aType ) throw (RuntimeException)
    {
        Any aRet(VCLXAccessibleComponent::queryInterface( aType ));
        return aRet.hasValue() ? aRet : OConnectionLineAccess_BASE::queryInterface( aType );
    }
    // -----------------------------------------------------------------------------
    Sequence< Type > SAL_CALL OConnectionLineAccess::getTypes(  ) throw (RuntimeException)
    {
        return ::comphelper::concatSequences(VCLXAccessibleComponent::getTypes(),OConnectionLineAccess_BASE::getTypes());
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OConnectionLineAccess::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }
    // -----------------------------------------------------------------------------
    // XServiceInfo - static methods
    // -----------------------------------------------------------------------------
    ::rtl::OUString OConnectionLineAccess::getImplementationName_Static(void) throw( RuntimeException )
    {
        return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbu.ConnectionLineAccessibility");
    }
    // -----------------------------------------------------------------------------
    // XAccessibleContext
    sal_Int32 SAL_CALL OConnectionLineAccess::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        return 0;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL OConnectionLineAccess::getAccessibleChild( sal_Int32 i ) throw (RuntimeException)
    {
        return Reference< XAccessible >();
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL OConnectionLineAccess::getAccessibleIndexInParent(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        sal_Int32 nIndex = -1;
        if( m_pLine )
        {
            // search the postion of our table window in the table window map
            nIndex = m_pLine->GetParent()->GetTabWinMap()->size();
            const ::std::vector<OTableConnection*>* pVec = m_pLine->GetParent()->getTableConnections();
            ::std::vector<OTableConnection*>::const_iterator aIter = pVec->begin();
            for (; aIter != pVec->end() && (*aIter) != m_pLine; ++nIndex,++aIter)
                ;
            nIndex = ( aIter != pVec->end() ) ? nIndex : -1;
        }
        return nIndex;
    }
    // -----------------------------------------------------------------------------
    sal_Int16 SAL_CALL OConnectionLineAccess::getAccessibleRole(  ) throw (RuntimeException)
    {
        return AccessibleRole::UNKNOWN; // ? or may be an AccessibleRole::WINDOW
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OConnectionLineAccess::getAccessibleDescription(  ) throw (RuntimeException)
    {
        static ::rtl::OUString sDescription(RTL_CONSTASCII_USTRINGPARAM("Relation"));
        return sDescription;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessibleRelationSet > SAL_CALL OConnectionLineAccess::getAccessibleRelationSet(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        return this;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleComponent
    sal_Bool SAL_CALL OConnectionLineAccess::contains( const awt::Point& _aPoint ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        Point aPoint(_aPoint.X,_aPoint.Y);
        return m_pLine ? m_pLine->CheckHit(aPoint) : sal_False;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL OConnectionLineAccess::getAccessibleAtPoint( const awt::Point& _aPoint ) throw (RuntimeException)
    {
        return Reference< XAccessible >();
    }
    // -----------------------------------------------------------------------------
    awt::Rectangle SAL_CALL OConnectionLineAccess::getBounds(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        Rectangle aRect(m_pLine ? m_pLine->GetBoundingRect() : Rectangle());
        return awt::Rectangle(aRect.getX(),aRect.getY(),aRect.getWidth(),aRect.getHeight());
    }
    // -----------------------------------------------------------------------------
    awt::Point SAL_CALL OConnectionLineAccess::getLocation(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        Point aPoint(m_pLine ? m_pLine->GetBoundingRect().TopLeft() : Point());
        return awt::Point(aPoint.X(),aPoint.Y());
    }
    // -----------------------------------------------------------------------------
    awt::Point SAL_CALL OConnectionLineAccess::getLocationOnScreen(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        Point aPoint(m_pLine ? m_pLine->GetParent()->ScreenToOutputPixel(m_pLine->GetBoundingRect().TopLeft()) : Point());
        return awt::Point(aPoint.X(),aPoint.Y());
    }
    // -----------------------------------------------------------------------------
    awt::Size SAL_CALL OConnectionLineAccess::getSize(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        Size aSize(m_pLine ? m_pLine->GetBoundingRect().GetSize() : Size());
        return awt::Size(aSize.Width(),aSize.Height());
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL OConnectionLineAccess::isShowing(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        return m_pLine ? m_pLine->GetParent()->GetWindowRegionPixel().IsInside(m_pLine->GetBoundingRect()) : sal_False;
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL OConnectionLineAccess::isVisible(  ) throw (RuntimeException)
    {
        return sal_True;
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL OConnectionLineAccess::isFocusTraversable(  ) throw (RuntimeException)
    {
        return sal_True;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleRelationSet
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL OConnectionLineAccess::getRelationCount(  ) throw (RuntimeException)
    {
        return 1;
    }
    // -----------------------------------------------------------------------------
    AccessibleRelation SAL_CALL OConnectionLineAccess::getRelation( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        if( nIndex < 0 || nIndex >= getRelationCount() )
            throw IndexOutOfBoundsException();

        Sequence< Reference<XInterface> > aSeq(m_pLine ? 2 : 0);
        if( m_pLine )
        {
            aSeq[0] = m_pLine->GetSourceWin()->GetAccessible();
            aSeq[1] = m_pLine->GetDestWin()->GetAccessible();
        }

        return AccessibleRelation(AccessibleRelationType::CONTROLLED_BY,aSeq);
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL OConnectionLineAccess::containsRelation( sal_Int16 aRelationType ) throw (RuntimeException)
    {
        return AccessibleRelationType::CONTROLLED_BY == aRelationType;
    }
    // -----------------------------------------------------------------------------
    AccessibleRelation SAL_CALL OConnectionLineAccess::getRelationByType( sal_Int16 aRelationType ) throw (RuntimeException)
    {
        if( AccessibleRelationType::CONTROLLED_BY == aRelationType )
            return getRelation(0);
        return AccessibleRelation();
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > OTableConnection::CreateAccessible()
    {
        return new OConnectionLineAccess(this);
    }
    // -----------------------------------------------------------------------------
    OTableConnection::~OTableConnection()
    {
        DBG_DTOR(OTableConnection,NULL);
        //////////////////////////////////////////////////////////////////////
        // clear vector
        clearLineData();
    }
    // -----------------------------------------------------------------------------
    sal_Bool OConnectionLineAccess::isEditable() const
    {
        return m_pLine ? !m_pLine->GetParent()->getDesignView()->getController()->isReadOnly() : sal_False;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessibleContext > SAL_CALL OConnectionLineAccess::getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return this;
    }
    // -----------------------------------------------------------------------------
}
// -----------------------------------------------------------------------------

