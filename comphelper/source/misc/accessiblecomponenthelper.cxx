/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessiblecomponenthelper.cxx,v $
 * $Revision: 1.9 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include <comphelper/accessiblecomponenthelper.hxx>

//.........................................................................
namespace comphelper
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::accessibility;

    //=====================================================================
    //= OCommonAccessibleComponent
    //=====================================================================
    //---------------------------------------------------------------------
    OCommonAccessibleComponent::OCommonAccessibleComponent( )
    {
    }

    //---------------------------------------------------------------------
    OCommonAccessibleComponent::OCommonAccessibleComponent( IMutex* _pExternalLock )
        :OAccessibleContextHelper( _pExternalLock )
    {
    }

    //---------------------------------------------------------------------
    OCommonAccessibleComponent::~OCommonAccessibleComponent( )
    {
        forgetExternalLock();
            // this ensures that the lock, which may be already destroyed as part of the derivee,
            // is not used anymore
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL OCommonAccessibleComponent::containsPoint( const Point& _rPoint ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );
        Rectangle aBounds( implGetBounds() );
        return  ( _rPoint.X >= 0 )
            &&  ( _rPoint.Y >= 0 )
            &&  ( _rPoint.X < aBounds.Width )
            &&  ( _rPoint.Y < aBounds.Height );
    }

    //--------------------------------------------------------------------
    Point SAL_CALL OCommonAccessibleComponent::getLocation(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );
        Rectangle aBounds( implGetBounds() );
        return Point( aBounds.X, aBounds.Y );
    }

    //--------------------------------------------------------------------
    Point SAL_CALL OCommonAccessibleComponent::getLocationOnScreen(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );
        Rectangle aBounds( implGetBounds() );

        Point aScreenLoc( 0, 0 );

        Reference< XAccessibleComponent > xParentComponent( implGetParentContext(), UNO_QUERY );
        OSL_ENSURE( xParentComponent.is(), "OCommonAccessibleComponent::getLocationOnScreen: no parent component!" );
        if ( xParentComponent.is() )
        {
            Point aParentScreenLoc( xParentComponent->getLocationOnScreen() );
            Point aOwnRelativeLoc( getLocation() );
            aScreenLoc.X = aParentScreenLoc.X + aOwnRelativeLoc.X;
            aScreenLoc.Y = aParentScreenLoc.Y + aOwnRelativeLoc.Y;
        }

        return aScreenLoc;
    }

    //--------------------------------------------------------------------
    Size SAL_CALL OCommonAccessibleComponent::getSize(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );
        Rectangle aBounds( implGetBounds() );
        return Size( aBounds.Width, aBounds.Height );
    }

    //--------------------------------------------------------------------
    Rectangle SAL_CALL OCommonAccessibleComponent::getBounds(  ) throw (RuntimeException)
    {
        OExternalLockGuard aGuard( this );
        return implGetBounds();
    }

    //=====================================================================
    //= OAccessibleComponentHelper
    //=====================================================================
    //---------------------------------------------------------------------
    OAccessibleComponentHelper::OAccessibleComponentHelper( )
    {
    }

    //---------------------------------------------------------------------
    OAccessibleComponentHelper::OAccessibleComponentHelper( IMutex* _pExternalLock )
        :OCommonAccessibleComponent( _pExternalLock )
    {
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( OAccessibleComponentHelper, OCommonAccessibleComponent, OAccessibleComponentHelper_Base )
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OAccessibleComponentHelper, OCommonAccessibleComponent, OAccessibleComponentHelper_Base )
        // (order matters: the first is the class name, the second is the class doing the ref counting)

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL OAccessibleComponentHelper::containsPoint( const Point& _rPoint ) throw (RuntimeException)
    {
        return OCommonAccessibleComponent::containsPoint( _rPoint );
    }

    //--------------------------------------------------------------------
    Point SAL_CALL OAccessibleComponentHelper::getLocation(  ) throw (RuntimeException)
    {
        return OCommonAccessibleComponent::getLocation( );
    }

    //--------------------------------------------------------------------
    Point SAL_CALL OAccessibleComponentHelper::getLocationOnScreen(  ) throw (RuntimeException)
    {
        return OCommonAccessibleComponent::getLocationOnScreen( );
    }

    //--------------------------------------------------------------------
    Size SAL_CALL OAccessibleComponentHelper::getSize(  ) throw (RuntimeException)
    {
        return OCommonAccessibleComponent::getSize( );
    }

    //--------------------------------------------------------------------
    Rectangle SAL_CALL OAccessibleComponentHelper::getBounds(  ) throw (RuntimeException)
    {
        return OCommonAccessibleComponent::getBounds( );
    }

    //=====================================================================
    //= OAccessibleExtendedComponentHelper
    //=====================================================================
    //---------------------------------------------------------------------
    OAccessibleExtendedComponentHelper::OAccessibleExtendedComponentHelper( )
    {
    }

    //---------------------------------------------------------------------
    OAccessibleExtendedComponentHelper::OAccessibleExtendedComponentHelper( IMutex* _pExternalLock )
        :OCommonAccessibleComponent( _pExternalLock )
    {
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( OAccessibleExtendedComponentHelper, OCommonAccessibleComponent, OAccessibleExtendedComponentHelper_Base )
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OAccessibleExtendedComponentHelper, OCommonAccessibleComponent, OAccessibleExtendedComponentHelper_Base )
        // (order matters: the first is the class name, the second is the class doing the ref counting)

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL OAccessibleExtendedComponentHelper::containsPoint( const Point& _rPoint ) throw (RuntimeException)
    {
        return OCommonAccessibleComponent::containsPoint( _rPoint );
    }

    //--------------------------------------------------------------------
    Point SAL_CALL OAccessibleExtendedComponentHelper::getLocation(  ) throw (RuntimeException)
    {
        return OCommonAccessibleComponent::getLocation( );
    }

    //--------------------------------------------------------------------
    Point SAL_CALL OAccessibleExtendedComponentHelper::getLocationOnScreen(  ) throw (RuntimeException)
    {
        return OCommonAccessibleComponent::getLocationOnScreen( );
    }

    //--------------------------------------------------------------------
    Size SAL_CALL OAccessibleExtendedComponentHelper::getSize(  ) throw (RuntimeException)
    {
        return OCommonAccessibleComponent::getSize( );
    }

    //--------------------------------------------------------------------
    Rectangle SAL_CALL OAccessibleExtendedComponentHelper::getBounds(  ) throw (RuntimeException)
    {
        return OCommonAccessibleComponent::getBounds( );
    }

//.........................................................................
}   // namespace comphelper
//.........................................................................


