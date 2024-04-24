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

#include <TableWindowAccess.hxx>
#include <TableWindow.hxx>
#include <TableWindowListBox.hxx>
#include <JoinTableView.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <vcl/vclevent.hxx>

namespace dbaui
{
    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;

    OTableWindowAccess::OTableWindowAccess(OTableWindow* _pTable)
        :ImplInheritanceHelper(_pTable->GetComponentInterface().is() ? _pTable->GetWindowPeer() : nullptr)
        ,m_pTable(_pTable)
    {
    }
    void SAL_CALL OTableWindowAccess::disposing()
    {
        m_pTable = nullptr;
        VCLXAccessibleComponent::disposing();
    }
    void OTableWindowAccess::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
    {
        if ( rVclWindowEvent.GetId() == VclEventId::ObjectDying )
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            m_pTable = nullptr;
        }

        VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
    }
    OUString SAL_CALL OTableWindowAccess::getImplementationName()
    {
        return "org.openoffice.comp.dbu.TableWindowAccessibility";
    }
    Sequence< OUString > SAL_CALL OTableWindowAccess::getSupportedServiceNames()
    {
        return { "com.sun.star.accessibility.Accessible",
                 "com.sun.star.accessibility.AccessibleContext" };
    }
    // XAccessibleContext
    sal_Int64 SAL_CALL OTableWindowAccess::getAccessibleChildCount(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        sal_Int64 nCount = 0;
        if(m_pTable)
        {
            ++nCount;
            if(m_pTable->GetListBox())
                ++nCount;
        }
        return nCount;
    }
    Reference< XAccessible > SAL_CALL OTableWindowAccess::getAccessibleChild( sal_Int64 i )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        Reference< XAccessible > aRet;
        if (m_pTable && !m_pTable->isDisposed())
        {
            switch(i)
            {
                case 0:
                {
                    VclPtr<OTableWindowTitle> xCtrl(m_pTable->GetTitleCtrl());
                    if (xCtrl)
                        aRet = xCtrl->GetAccessible();
                    break;
                }
                case 1:
                {
                    VclPtr<OTableWindowListBox> xCtrl(m_pTable->GetListBox());
                    if (xCtrl)
                        aRet = xCtrl->GetAccessible();
                    break;
                }
                default:
                    throw IndexOutOfBoundsException();
            }
        }
        return aRet;
    }
    sal_Int64 SAL_CALL OTableWindowAccess::getAccessibleIndexInParent(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        sal_Int64 nIndex = -1;
        if( m_pTable )
        {
            // search the position of our table window in the table window map
            bool bFoundElem = false;
            for (auto const& tabWin : m_pTable->getTableView()->GetTabWinMap())
            {
                if (tabWin.second == m_pTable)
                {
                    bFoundElem = true;
                    break;
                }
                ++nIndex;
            }
            nIndex = bFoundElem? nIndex : -1;
        }
        return nIndex;
    }
    sal_Int16 SAL_CALL OTableWindowAccess::getAccessibleRole(  )
    {
        return AccessibleRole::PANEL; // ? or may be an AccessibleRole::WINDOW
    }
    Reference< XAccessibleRelationSet > SAL_CALL OTableWindowAccess::getAccessibleRelationSet(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        return this;
    }
    // XAccessibleComponent
    Reference< XAccessible > SAL_CALL OTableWindowAccess::getAccessibleAtPoint( const awt::Point& _aPoint )
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        Reference< XAccessible > aRet;
        if(m_pTable && !m_pTable->isDisposed())
        {
            AbsoluteScreenPixelPoint aPoint(_aPoint.X,_aPoint.Y);
            AbsoluteScreenPixelRectangle aRect(m_pTable->GetDesktopRectPixel());
            if( aRect.Contains(aPoint) )
                aRet = this;
            else if( m_pTable->GetListBox()->GetDesktopRectPixel().Contains(aPoint))
                aRet = m_pTable->GetListBox()->GetAccessible();
        }
        return aRet;
    }
    Reference< XAccessible > OTableWindowAccess::getParentChild(sal_Int64 _nIndex)
    {
        Reference< XAccessible > xReturn;
        Reference< XAccessible > xParent = getAccessibleParent();
        if ( xParent.is() )
        {
            Reference< XAccessibleContext > xParentContext = xParent->getAccessibleContext();
            if ( xParentContext.is() )
            {
                xReturn = xParentContext->getAccessibleChild(_nIndex);
            }
        }
        return xReturn;
    }

    sal_Int32 SAL_CALL OTableWindowAccess::getRelationCount(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        return m_pTable ? m_pTable->getTableView()->getConnectionCount(m_pTable) : sal_Int32(0);
    }
    AccessibleRelation SAL_CALL OTableWindowAccess::getRelation( sal_Int32 nIndex )
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        if( nIndex < 0 || nIndex >= getRelationCount() )
            throw IndexOutOfBoundsException();

        AccessibleRelation aRet;
        if( m_pTable )
        {
            OJoinTableView* pView = m_pTable->getTableView();
            auto aIter = pView->getTableConnections(m_pTable) + nIndex;
            aRet.TargetSet = { getParentChild(aIter - pView->getTableConnections().begin()) };
            aRet.RelationType = AccessibleRelationType::CONTROLLER_FOR;
        }
        return aRet;
    }
    sal_Bool SAL_CALL OTableWindowAccess::containsRelation( sal_Int16 aRelationType )
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        return      AccessibleRelationType::CONTROLLER_FOR == aRelationType
                &&  m_pTable && m_pTable->getTableView()->ExistsAConn(m_pTable);
    }
    AccessibleRelation SAL_CALL OTableWindowAccess::getRelationByType( sal_Int16 aRelationType )
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        if( AccessibleRelationType::CONTROLLER_FOR == aRelationType && m_pTable)
        {
            OJoinTableView* pView = m_pTable->getTableView();
            const auto& rConnectionList = pView->getTableConnections();

            auto aIter = pView->getTableConnections(m_pTable);
            auto aEnd = rConnectionList.end();
            std::vector< Reference<css::accessibility::XAccessible> > aRelations;
            aRelations.reserve(5); // just guessing
            // TODO JNA aIter comes from pView->getTableConnections(m_pTable)
            // and aEnd comes from pView->getTableConnections().end()
            for (; aIter != aEnd ; ++aIter )
            {
                uno::Reference<css::accessibility::XAccessible> xAccessible(
                    getParentChild(aIter - rConnectionList.begin()));
                aRelations.push_back(xAccessible);
            }

            Sequence<Reference<css::accessibility::XAccessible>> aSeq(aRelations.data(), aRelations.size());
            return AccessibleRelation(AccessibleRelationType::CONTROLLER_FOR, aSeq);
        }
        return AccessibleRelation();
    }
    OUString SAL_CALL OTableWindowAccess::getTitledBorderText(  )
    {
        return getAccessibleName(  );
    }
    OUString SAL_CALL OTableWindowAccess::getAccessibleName(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex  );
        OUString sAccessibleName;
        if ( m_pTable )
            sAccessibleName = m_pTable->getTitle();
        return sAccessibleName;
    }
    Reference< XAccessibleContext > SAL_CALL OTableWindowAccess::getAccessibleContext(  )
    {
        return this;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
