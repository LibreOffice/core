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

#include <comphelper/accessiblecontexthelper.hxx>
#include <comphelper/accessibleselectionhelper.hxx>


namespace comphelper
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::accessibility;

    OCommonAccessibleSelection::OCommonAccessibleSelection( )
    {
    }

    OCommonAccessibleSelection::~OCommonAccessibleSelection() {}


    void OCommonAccessibleSelection::selectAccessibleChild( sal_Int64 nChildIndex )
    {
        implSelect( nChildIndex, true );
    }


    bool OCommonAccessibleSelection::isAccessibleChildSelected( sal_Int64 nChildIndex )
    {
        return implIsSelected( nChildIndex );
    }


    void OCommonAccessibleSelection::clearAccessibleSelection(  )
    {
        implSelect( ACCESSIBLE_SELECTION_CHILD_ALL, false );
    }


    void OCommonAccessibleSelection::selectAllAccessibleChildren(  )
    {
        implSelect( ACCESSIBLE_SELECTION_CHILD_ALL, true );
    }


    sal_Int64 OCommonAccessibleSelection::getSelectedAccessibleChildCount(  )
    {
        sal_Int64 nRet = 0;
        Reference< XAccessibleContext > xParentContext( implGetAccessibleContext() );

        OSL_ENSURE( xParentContext.is(), "OCommonAccessibleSelection::getSelectedAccessibleChildCount: no parent context!" );

        if( xParentContext.is() )
        {
            for( sal_Int64 i = 0, nChildCount = xParentContext->getAccessibleChildCount(); i < nChildCount; i++ )
                if( implIsSelected( i ) )
                    ++nRet;
        }

        return nRet;
    }


    Reference< XAccessible > OCommonAccessibleSelection::getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex )
    {
        Reference< XAccessible >        xRet;
        Reference< XAccessibleContext > xParentContext( implGetAccessibleContext() );

        OSL_ENSURE( xParentContext.is(), "OCommonAccessibleSelection::getSelectedAccessibleChildCount: no parent context!" );

        if( xParentContext.is() )
        {
            for( sal_Int64 i = 0, nChildCount = xParentContext->getAccessibleChildCount(), nPos = 0; ( i < nChildCount ) && !xRet.is(); i++ )
                if( implIsSelected( i ) && ( nPos++ == nSelectedChildIndex ) )
                    xRet = xParentContext->getAccessibleChild( i );
        }

        return xRet;
    }


    void OCommonAccessibleSelection::deselectAccessibleChild( sal_Int64 nSelectedChildIndex )
    {
        implSelect( nSelectedChildIndex, false );
    }

    OAccessibleSelectionHelper::OAccessibleSelectionHelper()
    {
    }


    Reference< XAccessibleContext > OAccessibleSelectionHelper::implGetAccessibleContext()
    {
        return this;
    }


    void SAL_CALL OAccessibleSelectionHelper::selectAccessibleChild( sal_Int64 nChildIndex )
    {
        OExternalLockGuard aGuard( this );
        OCommonAccessibleSelection::selectAccessibleChild( nChildIndex );
    }


    sal_Bool SAL_CALL OAccessibleSelectionHelper::isAccessibleChildSelected( sal_Int64 nChildIndex )
    {
        OExternalLockGuard aGuard( this );
        return OCommonAccessibleSelection::isAccessibleChildSelected( nChildIndex );
    }


    void SAL_CALL OAccessibleSelectionHelper::clearAccessibleSelection(  )
    {
        OExternalLockGuard aGuard( this );
        OCommonAccessibleSelection::clearAccessibleSelection();
    }


    void SAL_CALL OAccessibleSelectionHelper::selectAllAccessibleChildren(  )
    {
        OExternalLockGuard aGuard( this );
        OCommonAccessibleSelection::selectAllAccessibleChildren();
    }


    sal_Int64 SAL_CALL OAccessibleSelectionHelper::getSelectedAccessibleChildCount(  )
    {
        OExternalLockGuard aGuard( this );
        return OCommonAccessibleSelection::getSelectedAccessibleChildCount();
    }


    Reference< XAccessible > SAL_CALL OAccessibleSelectionHelper::getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex )
    {
        OExternalLockGuard aGuard( this );
        return OCommonAccessibleSelection::getSelectedAccessibleChild( nSelectedChildIndex );
    }


    void SAL_CALL OAccessibleSelectionHelper::deselectAccessibleChild( sal_Int64 nSelectedChildIndex )
    {
        OExternalLockGuard aGuard( this );
        OCommonAccessibleSelection::deselectAccessibleChild( nSelectedChildIndex );
    }


}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
