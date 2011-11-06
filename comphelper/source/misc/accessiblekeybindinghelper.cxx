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
#include "precompiled_comphelper.hxx"

// includes --------------------------------------------------------------
#include <comphelper/accessiblekeybindinghelper.hxx>


//..............................................................................
namespace comphelper
{
//..............................................................................

    using namespace ::com::sun::star; // MT 04/2003: was ::drafts::com::sun::star - otherwise to many changes
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::accessibility;

    //==============================================================================
    // OAccessibleKeyBindingHelper
    //==============================================================================

    OAccessibleKeyBindingHelper::OAccessibleKeyBindingHelper()
    {
    }

    // -----------------------------------------------------------------------------

    OAccessibleKeyBindingHelper::OAccessibleKeyBindingHelper( const OAccessibleKeyBindingHelper& rHelper )
        : cppu::WeakImplHelper1<XAccessibleKeyBinding>( rHelper )
        , m_aKeyBindings( rHelper.m_aKeyBindings )
    {
    }

    // -----------------------------------------------------------------------------

    OAccessibleKeyBindingHelper::~OAccessibleKeyBindingHelper()
    {
    }

    // -----------------------------------------------------------------------------

    void OAccessibleKeyBindingHelper::AddKeyBinding( const Sequence< awt::KeyStroke >& rKeyBinding ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        m_aKeyBindings.push_back( rKeyBinding );
    }

    // -----------------------------------------------------------------------------

    void OAccessibleKeyBindingHelper::AddKeyBinding( const awt::KeyStroke& rKeyStroke ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Sequence< awt::KeyStroke > aSeq(1);
        aSeq[0] = rKeyStroke;
        m_aKeyBindings.push_back( aSeq );
    }

    // -----------------------------------------------------------------------------
    // XAccessibleKeyBinding
    // -----------------------------------------------------------------------------

    sal_Int32 OAccessibleKeyBindingHelper::getAccessibleKeyBindingCount() throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return m_aKeyBindings.size();
    }

    // -----------------------------------------------------------------------------

    Sequence< awt::KeyStroke > OAccessibleKeyBindingHelper::getAccessibleKeyBinding( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( nIndex < 0 || nIndex >= (sal_Int32)m_aKeyBindings.size() )
            throw IndexOutOfBoundsException();

        return m_aKeyBindings[nIndex];
    }

    // -----------------------------------------------------------------------------

//..............................................................................
}   // namespace comphelper
//..............................................................................
