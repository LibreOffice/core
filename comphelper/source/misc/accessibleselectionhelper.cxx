/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessibleselectionhelper.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:47:35 $
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

#ifndef COMPHELPER_ACCESSIBLE_SELECTION_HELPER_HXX
#include <comphelper/accessibleselectionhelper.hxx>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::accessibility;

    //=====================================================================
    //= OCommonAccessibleSelection
    //=====================================================================
    //---------------------------------------------------------------------
    OCommonAccessibleSelection::OCommonAccessibleSelection( )
    {
    }

    //--------------------------------------------------------------------
    void SAL_CALL OCommonAccessibleSelection::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        implSelect( nChildIndex, sal_True );
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL OCommonAccessibleSelection::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        return( implIsSelected( nChildIndex ) );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OCommonAccessibleSelection::clearAccessibleSelection(  ) throw (RuntimeException)
    {
        implSelect( ACCESSIBLE_SELECTION_CHILD_ALL, sal_False );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OCommonAccessibleSelection::selectAllAccessibleChildren(  ) throw (RuntimeException)
    {
        implSelect( ACCESSIBLE_SELECTION_CHILD_ALL, sal_True );
    }

    //--------------------------------------------------------------------
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

    //--------------------------------------------------------------------
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

    //--------------------------------------------------------------------
    void SAL_CALL OCommonAccessibleSelection::deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        implSelect( nSelectedChildIndex, sal_False );
    }

    //=====================================================================
    //= OAccessibleSelectionHelper
    //=====================================================================
    //---------------------------------------------------------------------
    OAccessibleSelectionHelper::OAccessibleSelectionHelper( )
    {
    }

    //--------------------------------------------------------------------
    OAccessibleSelectionHelper::OAccessibleSelectionHelper( IMutex* _pExternalLock ) : OAccessibleComponentHelper(_pExternalLock)
    {
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( OAccessibleSelectionHelper, OAccessibleComponentHelper, OAccessibleSelectionHelper_Base )
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OAccessibleSelectionHelper, OAccessibleComponentHelper, OAccessibleSelectionHelper_Base )
    // (order matters: the first is the class name, the second is the class doing the ref counting)

    //--------------------------------------------------------------------
    Reference< XAccessibleContext > OAccessibleSelectionHelper::implGetAccessibleContext() throw ( RuntimeException )
    {
        return( this );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OAccessibleSelectionHelper::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );
        OCommonAccessibleSelection::selectAccessibleChild( nChildIndex );
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL OAccessibleSelectionHelper::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );
        return( OCommonAccessibleSelection::isAccessibleChildSelected( nChildIndex ) );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OAccessibleSelectionHelper::clearAccessibleSelection(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );
        OCommonAccessibleSelection::clearAccessibleSelection();
    }

    //--------------------------------------------------------------------
    void SAL_CALL OAccessibleSelectionHelper::selectAllAccessibleChildren(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );
        OCommonAccessibleSelection::selectAllAccessibleChildren();
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL OAccessibleSelectionHelper::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );
        return( OCommonAccessibleSelection::getSelectedAccessibleChildCount() );
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL OAccessibleSelectionHelper::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );
        return( OCommonAccessibleSelection::getSelectedAccessibleChild( nSelectedChildIndex ) );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OAccessibleSelectionHelper::deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        OExternalLockGuard aGuard( this );
        OCommonAccessibleSelection::deselectAccessibleChild( nSelectedChildIndex );
    }

//.........................................................................
}   // namespace comphelper
//.........................................................................

