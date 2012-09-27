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
