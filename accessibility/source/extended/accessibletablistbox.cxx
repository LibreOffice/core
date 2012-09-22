/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include "accessibility/extended/accessibletablistbox.hxx"
#include "accessibility/extended/accessibletablistboxtable.hxx"
#include <svtools/svtabbx.hxx>
#include <comphelper/sequence.hxx>

//........................................................................
namespace accessibility
{
//........................................................................

    // class AccessibleTabListBox -----------------------------------------------------

    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;

    // -----------------------------------------------------------------------------
    // Ctor() and Dtor()
    // -----------------------------------------------------------------------------
    AccessibleTabListBox::AccessibleTabListBox( const Reference< XAccessible >& rxParent, SvHeaderTabListBox& rBox )
        :AccessibleBrowseBox( rxParent, NULL, rBox )
        ,m_pTabListBox( &rBox )

    {
        osl_atomic_increment( &m_refCount );
        {
            setCreator( this );
        }
        osl_atomic_decrement( &m_refCount );
    }

    // -----------------------------------------------------------------------------
    AccessibleTabListBox::~AccessibleTabListBox()
    {
        if ( isAlive() )
        {
            // increment ref count to prevent double call of Dtor
            osl_atomic_increment( &m_refCount );
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
        return 2; // header and table
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
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );
        ensureIsAlive();

        if ( nChildIndex < 0 || nChildIndex > 1 )
            throw IndexOutOfBoundsException();

        Reference< XAccessible > xRet;
        if (nChildIndex == 0)
        {
            //! so far the actual implementation object only supports column headers
            xRet = implGetFixedChild( ::svt::BBINDEX_COLUMNHEADERBAR );
        }
        else if (nChildIndex == 1)
            xRet = implGetFixedChild( ::svt::BBINDEX_TABLE );

        if ( !xRet.is() )
            throw RuntimeException();

        return xRet;
    }

//........................................................................
}// namespace accessibility
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
