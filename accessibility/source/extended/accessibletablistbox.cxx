/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessibletablistbox.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:33:59 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"

#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOX_HXX_
#include "accessibility/extended/accessibletablistbox.hxx"
#endif
#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOXTABLE_HXX
#include "accessibility/extended/accessibletablistboxtable.hxx"
#endif
#ifndef _SVTABBX_HXX
#include <svtools/svtabbx.hxx>
#endif

#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

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

