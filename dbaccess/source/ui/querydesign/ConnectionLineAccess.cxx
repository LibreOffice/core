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
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
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
        : VCLXAccessibleComponent(_pLine->GetComponentInterface().is() ? _pLine->GetWindowPeer() : nullptr)
        ,m_pLine(_pLine)
    {
    }
    void SAL_CALL OConnectionLineAccess::disposing()
    {
        m_pLine = nullptr;
        VCLXAccessibleComponent::disposing();
    }
    Any SAL_CALL OConnectionLineAccess::queryInterface( const Type& aType )
    {
        Any aRet(VCLXAccessibleComponent::queryInterface( aType ));
        return aRet.hasValue() ? aRet : OConnectionLineAccess_BASE::queryInterface( aType );
    }
    Sequence< Type > SAL_CALL OConnectionLineAccess::getTypes(  )
    {
        return ::comphelper::concatSequences(VCLXAccessibleComponent::getTypes(),OConnectionLineAccess_BASE::getTypes());
    }
    OUString SAL_CALL OConnectionLineAccess::getImplementationName()
    {
        return OUString("org.openoffice.comp.dbu.ConnectionLineAccessibility");
    }
    // XAccessibleContext
    sal_Int32 SAL_CALL OConnectionLineAccess::getAccessibleChildCount(  )
    {
        return 0;
    }
    Reference< XAccessible > SAL_CALL OConnectionLineAccess::getAccessibleChild( sal_Int32 /*i*/ )
    {
        return Reference< XAccessible >();
    }
    sal_Int32 SAL_CALL OConnectionLineAccess::getAccessibleIndexInParent(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        sal_Int32 nIndex = -1;
        if( m_pLine )
        {
            // search the position of our table window in the table window map
            nIndex = m_pLine->GetParent()->GetTabWinMap().size();
            const auto& rVec = m_pLine->GetParent()->getTableConnections();
            auto aIter = rVec.begin();
            auto aEnd = rVec.end();
            for (; aIter != aEnd && (*aIter).get() != m_pLine; ++nIndex,++aIter)
                ;
            nIndex = ( aIter != aEnd ) ? nIndex : -1;
        }
        return nIndex;
    }
    sal_Int16 SAL_CALL OConnectionLineAccess::getAccessibleRole(  )
    {
        return AccessibleRole::UNKNOWN; // ? or may be an AccessibleRole::WINDOW
    }
    OUString SAL_CALL OConnectionLineAccess::getAccessibleDescription(  )
    {
        return OUString("Relation");
    }
    Reference< XAccessibleRelationSet > SAL_CALL OConnectionLineAccess::getAccessibleRelationSet(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        return this;
    }
    // XAccessibleComponent
    Reference< XAccessible > SAL_CALL OConnectionLineAccess::getAccessibleAtPoint( const awt::Point& /*_aPoint*/ )
    {
        return Reference< XAccessible >();
    }
    awt::Rectangle SAL_CALL OConnectionLineAccess::getBounds(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        Rectangle aRect(m_pLine ? m_pLine->GetBoundingRect() : Rectangle());
        return awt::Rectangle(aRect.getX(),aRect.getY(),aRect.getWidth(),aRect.getHeight());
    }
    awt::Point SAL_CALL OConnectionLineAccess::getLocation(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        Point aPoint(m_pLine ? m_pLine->GetBoundingRect().TopLeft() : Point());
        return awt::Point(aPoint.X(),aPoint.Y());
    }
    awt::Point SAL_CALL OConnectionLineAccess::getLocationOnScreen(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        Point aPoint(m_pLine ? m_pLine->GetParent()->ScreenToOutputPixel(m_pLine->GetBoundingRect().TopLeft()) : Point());
        return awt::Point(aPoint.X(),aPoint.Y());
    }
    awt::Size SAL_CALL OConnectionLineAccess::getSize(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        Size aSize(m_pLine ? m_pLine->GetBoundingRect().GetSize() : Size());
        return awt::Size(aSize.Width(),aSize.Height());
    }
    // XAccessibleRelationSet
    sal_Int32 SAL_CALL OConnectionLineAccess::getRelationCount(  )
    {
        return 1;
    }
    AccessibleRelation SAL_CALL OConnectionLineAccess::getRelation( sal_Int32 nIndex )
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
    sal_Bool SAL_CALL OConnectionLineAccess::containsRelation( sal_Int16 aRelationType )
    {
        return AccessibleRelationType::CONTROLLED_BY == aRelationType;
    }
    AccessibleRelation SAL_CALL OConnectionLineAccess::getRelationByType( sal_Int16 aRelationType )
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
        disposeOnce();
    }
    void OTableConnection::dispose()
    {
        // clear vector
        clearLineData();
        m_pParent.clear();
        vcl::Window::dispose();
    }
    Reference< XAccessibleContext > SAL_CALL OConnectionLineAccess::getAccessibleContext(  )
    {
        return this;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
