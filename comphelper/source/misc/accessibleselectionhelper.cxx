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

#include <comphelper/accessibleselectionhelper.hxx>


namespace comphelper
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::accessibility;


    //= OCommonAccessibleSelection


    OCommonAccessibleSelection::OCommonAccessibleSelection( )
    {
    }

    OCommonAccessibleSelection::~OCommonAccessibleSelection() {}


    void SAL_CALL OCommonAccessibleSelection::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        implSelect( nChildIndex, sal_True );
    }


    bool SAL_CALL OCommonAccessibleSelection::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        return( implIsSelected( nChildIndex ) );
    }


    void SAL_CALL OCommonAccessibleSelection::clearAccessibleSelection(  ) throw (RuntimeException)
    {
        implSelect( ACCESSIBLE_SELECTION_CHILD_ALL, sal_False );
    }


    void SAL_CALL OCommonAccessibleSelection::selectAllAccessibleChildren(  ) throw (RuntimeException)
    {
        implSelect( ACCESSIBLE_SELECTION_CHILD_ALL, sal_True );
    }


    sal_Int32 SAL_CALL OCommonAccessibleSelection::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
    {
        sal_Int32                       nRet = 0;
        Reference< XAccessibleContext > xParentContext( implGetAccessibleContext() );

        OSL_ENSURE( xParentContext.is(), "OCommonAccessibleSelection::getSelectedAccessibleChildCount: no parent context!" );

        if( xParentContext.is() )
        {
            for( sal_Int32 i = 0, nChildCount = xParentContext->getAccessibleChildCount(); i < nChildCount; i++ )
                if( implIsSelected( i ) )
                    ++nRet;
        }

        return( nRet );
    }


    Reference< XAccessible > SAL_CALL OCommonAccessibleSelection::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        Reference< XAccessible >        xRet;
        Reference< XAccessibleContext > xParentContext( implGetAccessibleContext() );

        OSL_ENSURE( xParentContext.is(), "OCommonAccessibleSelection::getSelectedAccessibleChildCount: no parent context!" );

        if( xParentContext.is() )
        {
            for( sal_Int32 i = 0, nChildCount = xParentContext->getAccessibleChildCount(), nPos = 0; ( i < nChildCount ) && !xRet.is(); i++ )
                if( implIsSelected( i ) && ( nPos++ == nSelectedChildIndex ) )
                    xRet = xParentContext->getAccessibleChild( i );
        }

        return( xRet );
    }


    void SAL_CALL OCommonAccessibleSelection::deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        implSelect( nSelectedChildIndex, sal_False );
    }


    //= OAccessibleSelectionHelper


    OAccessibleSelectionHelper::OAccessibleSelectionHelper( IMutex* _pExternalLock ) : OAccessibleComponentHelper(_pExternalLock)
    {
    }


    IMPLEMENT_FORWARD_XINTERFACE2( OAccessibleSelectionHelper, OAccessibleComponentHelper, OAccessibleSelectionHelper_Base )
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OAccessibleSelectionHelper, OAccessibleComponentHelper, OAccessibleSelectionHelper_Base )
    // (order matters: the first is the class name, the second is the class doing the ref counting)


    Reference< XAccessibleContext > OAccessibleSelectionHelper::implGetAccessibleContext() throw ( RuntimeException )
    {
        return( this );
    }


    void SAL_CALL OAccessibleSelectionHelper::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );
        OCommonAccessibleSelection::selectAccessibleChild( nChildIndex );
    }


    sal_Bool SAL_CALL OAccessibleSelectionHelper::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );
        return( OCommonAccessibleSelection::isAccessibleChildSelected( nChildIndex ) );
    }


    void SAL_CALL OAccessibleSelectionHelper::clearAccessibleSelection(  ) throw (RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );
        OCommonAccessibleSelection::clearAccessibleSelection();
    }


    void SAL_CALL OAccessibleSelectionHelper::selectAllAccessibleChildren(  ) throw (RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );
        OCommonAccessibleSelection::selectAllAccessibleChildren();
    }


    sal_Int32 SAL_CALL OAccessibleSelectionHelper::getSelectedAccessibleChildCount(  ) throw (RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );
        return( OCommonAccessibleSelection::getSelectedAccessibleChildCount() );
    }


    Reference< XAccessible > SAL_CALL OAccessibleSelectionHelper::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );
        return( OCommonAccessibleSelection::getSelectedAccessibleChild( nSelectedChildIndex ) );
    }


    void SAL_CALL OAccessibleSelectionHelper::deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        OExternalLockGuard aGuard( this );
        OCommonAccessibleSelection::deselectAccessibleChild( nSelectedChildIndex );
    }


}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
