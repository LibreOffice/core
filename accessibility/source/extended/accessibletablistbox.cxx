/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessibletablistbox.cxx,v $
 * $Revision: 1.3 $
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
#include "precompiled_accessibility.hxx"

#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOX_HXX_
#include "accessibility/extended/accessibletablistbox.hxx"
#endif
#include "accessibility/extended/accessibletablistboxtable.hxx"
#include <svtools/svtabbx.hxx>
#include <comphelper/sequence.hxx>

//........................................................................
namespace accessibility
{
//........................................................................

    // class TLBSolarGuard ---------------------------------------------------------

    /** Aquire the solar mutex. */
    class TLBSolarGuard : public ::vos::OGuard
    {
    public:
        inline TLBSolarGuard() : ::vos::OGuard( Application::GetSolarMutex() ) {}
    };

    // class AccessibleTabListBox -----------------------------------------------------

    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;

    DBG_NAME(AccessibleTabListBox)

    // -----------------------------------------------------------------------------
    // Ctor() and Dtor()
    // -----------------------------------------------------------------------------
    AccessibleTabListBox::AccessibleTabListBox( const Reference< XAccessible >& rxParent, SvHeaderTabListBox& rBox )
        :AccessibleBrowseBox( rxParent, NULL, rBox )
        ,m_pTabListBox( &rBox )

    {
        DBG_CTOR( AccessibleTabListBox, NULL );

        osl_incrementInterlockedCount( &m_refCount );
        {
            setCreator( this );
        }
        osl_decrementInterlockedCount( &m_refCount );
    }

    // -----------------------------------------------------------------------------
    AccessibleTabListBox::~AccessibleTabListBox()
    {
        DBG_DTOR( AccessibleTabListBox, NULL );

        if ( isAlive() )
        {
            // increment ref count to prevent double call of Dtor
            osl_incrementInterlockedCount( &m_refCount );
            dispose();
        }
    }
    // -----------------------------------------------------------------------------
    AccessibleBrowseBoxTable* AccessibleTabListBox::createAccessibleTable()
    {
        return new AccessibleTabListBoxTable( this, *m_pTabListBox );
    }

    // XInterface -----------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( AccessibleTabListBox, AccessibleBrowseBox, AccessibleTabListBox_Base )

    // XTypeProvider --------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( AccessibleTabListBox, AccessibleBrowseBox, AccessibleTabListBox_Base )

    // XAccessibleContext ---------------------------------------------------------

    sal_Int32 SAL_CALL AccessibleTabListBox::getAccessibleChildCount()
        throw ( uno::RuntimeException )
    {
        return 1; // no header, only table
    }

    // -----------------------------------------------------------------------------
    Reference< XAccessibleContext > SAL_CALL AccessibleTabListBox::getAccessibleContext() throw ( RuntimeException )
    {
        return this;
    }

    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL
    AccessibleTabListBox::getAccessibleChild( sal_Int32 nChildIndex )
        throw ( IndexOutOfBoundsException, RuntimeException )
    {
        TLBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        ensureIsAlive();

        if ( nChildIndex != 0 )
            // only one child
            throw IndexOutOfBoundsException();

        Reference< XAccessible > xRet = implGetFixedChild( ::svt::BBINDEX_TABLE );

        if ( !xRet.is() )
            throw RuntimeException();

        return xRet;
    }

//........................................................................
}// namespace accessibility
//........................................................................

