/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

    /** Acquire the solar mutex. */
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
        TLBSolarGuard aSolarGuard;
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

