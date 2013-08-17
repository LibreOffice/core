/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "ConnectionLineAccess.hxx"
#include "JoinTableView.hxx"
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <toolkit/awt/vclxwindow.hxx>
#include "TableConnection.hxx"
#include "TableWindow.hxx"
#include <comphelper/uno3.hxx>
#include "JoinDesignView.hxx"
#include "JoinController.hxx"
#include <comphelper/sequence.hxx>

namespace dbaui
{
    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;

    OConnectionLineAccess::OConnectionLineAccess(OTableConnection* _pLine)
        : VCLXAccessibleComponent(_pLine->GetComponentInterface().is() ? _pLine->GetWindowPeer() : NULL)
        ,m_pLine(_pLine)
    {
    }
    void SAL_CALL OConnectionLineAccess::disposing()
    {
        m_pLine = NULL;
        VCLXAccessibleComponent::disposing();
    }
    Any SAL_CALL OConnectionLineAccess::queryInterface( const Type& aType ) throw (RuntimeException)
    {
        Any aRet(VCLXAccessibleComponent::queryInterface( aType ));
        return aRet.hasValue() ? aRet : OConnectionLineAccess_BASE::queryInterface( aType );
    }
    Sequence< Type > SAL_CALL OConnectionLineAccess::getTypes(  ) throw (RuntimeException)
    {
        return ::comphelper::concatSequences(VCLXAccessibleComponent::getTypes(),OConnectionLineAccess_BASE::getTypes());
    }
    OUString SAL_CALL OConnectionLineAccess::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }
    // XServiceInfo - static methods
    OUString OConnectionLineAccess::getImplementationName_Static(void) throw( RuntimeException )
    {
        return OUString("org.openoffice.comp.dbu.ConnectionLineAccessibility");
    }
    // XAccessibleContext
    sal_Int32 SAL_CALL OConnectionLineAccess::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        return 0;
    }
    Reference< XAccessible > SAL_CALL OConnectionLineAccess::getAccessibleChild( sal_Int32 /*i*/ ) throw (RuntimeException)
    {
        return Reference< XAccessible >();
    }
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
            ::std::vector<OTableConnection*>::const_iterator aEnd = pVec->end();
            for (; aIter != aEnd && (*aIter) != m_pLine; ++nIndex,++aIter)
                ;
            nIndex = ( aIter != aEnd ) ? nIndex : -1;
        }
        return nIndex;
    }
    sal_Int16 SAL_CALL OConnectionLineAccess::getAccessibleRole(  ) throw (RuntimeException)
    {
        return AccessibleRole::UNKNOWN; // ? or may be an AccessibleRole::WINDOW
    }
    OUString SAL_CALL OConnectionLineAccess::getAccessibleDescription(  ) throw (RuntimeException)
    {
        static OUString sDescription("Relation");
        return sDescription;
    }
    Reference< XAccessibleRelationSet > SAL_CALL OConnectionLineAccess::getAccessibleRelationSet(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        return this;
    }
    // XAccessibleComponent
    sal_Bool SAL_CALL OConnectionLineAccess::contains( const awt::Point& _aPoint ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        Point aPoint(_aPoint.X,_aPoint.Y);
        return m_pLine ? m_pLine->CheckHit(aPoint) : sal_False;
    }
    Reference< XAccessible > SAL_CALL OConnectionLineAccess::getAccessibleAtPoint( const awt::Point& /*_aPoint*/ ) throw (RuntimeException)
    {
        return Reference< XAccessible >();
    }
    awt::Rectangle SAL_CALL OConnectionLineAccess::getBounds(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        Rectangle aRect(m_pLine ? m_pLine->GetBoundingRect() : Rectangle());
        return awt::Rectangle(aRect.getX(),aRect.getY(),aRect.getWidth(),aRect.getHeight());
    }
    awt::Point SAL_CALL OConnectionLineAccess::getLocation(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        Point aPoint(m_pLine ? m_pLine->GetBoundingRect().TopLeft() : Point());
        return awt::Point(aPoint.X(),aPoint.Y());
    }
    awt::Point SAL_CALL OConnectionLineAccess::getLocationOnScreen(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        Point aPoint(m_pLine ? m_pLine->GetParent()->ScreenToOutputPixel(m_pLine->GetBoundingRect().TopLeft()) : Point());
        return awt::Point(aPoint.X(),aPoint.Y());
    }
    awt::Size SAL_CALL OConnectionLineAccess::getSize(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        Size aSize(m_pLine ? m_pLine->GetBoundingRect().GetSize() : Size());
        return awt::Size(aSize.Width(),aSize.Height());
    }
    sal_Bool SAL_CALL OConnectionLineAccess::isShowing(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        return m_pLine ? m_pLine->GetParent()->GetWindowRegionPixel().IsInside(m_pLine->GetBoundingRect()) : sal_False;
    }
    sal_Bool SAL_CALL OConnectionLineAccess::isVisible(  ) throw (RuntimeException)
    {
        return sal_True;
    }
    sal_Bool SAL_CALL OConnectionLineAccess::isFocusTraversable(  ) throw (RuntimeException)
    {
        return sal_True;
    }
    // XAccessibleRelationSet
    sal_Int32 SAL_CALL OConnectionLineAccess::getRelationCount(  ) throw (RuntimeException)
    {
        return 1;
    }
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
    sal_Bool SAL_CALL OConnectionLineAccess::containsRelation( sal_Int16 aRelationType ) throw (RuntimeException)
    {
        return AccessibleRelationType::CONTROLLED_BY == aRelationType;
    }
    AccessibleRelation SAL_CALL OConnectionLineAccess::getRelationByType( sal_Int16 aRelationType ) throw (RuntimeException)
    {
        if( AccessibleRelationType::CONTROLLED_BY == aRelationType )
            return getRelation(0);
        return AccessibleRelation();
    }
    Reference< XAccessible > OTableConnection::CreateAccessible()
    {
        return new OConnectionLineAccess(this);
    }
    OTableConnection::~OTableConnection()
    {
        DBG_DTOR(OTableConnection,NULL);
        // clear vector
        clearLineData();
    }
    sal_Bool OConnectionLineAccess::isEditable() const
    {

        return m_pLine ? !m_pLine->GetParent()->getDesignView()->getController().isReadOnly() : sal_False;
    }
    Reference< XAccessibleContext > SAL_CALL OConnectionLineAccess::getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return this;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
