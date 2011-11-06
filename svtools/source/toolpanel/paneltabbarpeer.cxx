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



#include "precompiled_svtools.hxx"

#include "paneltabbarpeer.hxx"
#include "svtools/toolpanel/paneltabbar.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/DisposedException.hpp>
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>

//........................................................................
namespace svt
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::accessibility::XAccessibleContext;
    using ::com::sun::star::lang::DisposedException;
    /** === end UNO using === **/

    //==================================================================================================================
    //= PanelTabBarPeer
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    PanelTabBarPeer::PanelTabBarPeer( PanelTabBar& i_rTabBar )
        :VCLXWindow()
        ,m_pTabBar( &i_rTabBar )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    PanelTabBarPeer::~PanelTabBarPeer()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessibleContext > PanelTabBarPeer::CreateAccessibleContext()
    {
        ::vos::OGuard aSolarGuard( GetMutex() );
        if ( m_pTabBar == NULL )
            throw DisposedException( ::rtl::OUString(), *this );



        Window* pAccessibleParent( m_pTabBar->GetAccessibleParentWindow() );
        ENSURE_OR_RETURN( pAccessibleParent != NULL, "no accessible parent => no accessible context", NULL );
        Reference< XAccessible > xAccessibleParent( pAccessibleParent->GetAccessible(), UNO_SET_THROW );
        return m_aAccessibleFactory.getFactory().createAccessibleToolPanelTabBar( xAccessibleParent, m_pTabBar->GetPanelDeck(), *m_pTabBar );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL PanelTabBarPeer::dispose() throw(RuntimeException)
    {
        {
            ::vos::OGuard aSolarGuard( GetMutex() );
            m_pTabBar = NULL;
        }
        VCLXWindow::dispose();
    }

//........................................................................
} // namespace svt
//........................................................................
