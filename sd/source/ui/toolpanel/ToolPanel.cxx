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



#include "precompiled_sd.hxx"

#include "ToolPanel.hxx"
#include "MethodGuard.hxx"
#include <taskpane/TaskPaneTreeNode.hxx>

/** === begin UNO includes === **/
#include <com/sun/star/lang/DisposedException.hpp>
/** === end UNO includes === **/

#include <vcl/window.hxx>

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

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
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::accessibility::XAccessible;
    /** === end UNO using === **/

    typedef MethodGuard< ToolPanel > ToolPanelGuard;

    //==================================================================================================================
    //= ToolPanel
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ToolPanel::ToolPanel( ::std::auto_ptr< TreeNode >& i_rControl )
        :ToolPanel_Base( m_aMutex )
        ,m_pControl( i_rControl )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ToolPanel::~ToolPanel()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanel::checkDisposed()
    {
        if ( m_pControl.get() == NULL )
            throw DisposedException( ::rtl::OUString(), *this );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XWindow > SAL_CALL ToolPanel::getWindow() throw (RuntimeException)
    {
        ToolPanelGuard aGuard( *this );
        return Reference< XWindow >( m_pControl->GetWindow()->GetComponentInterface(), UNO_QUERY_THROW );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL ToolPanel::createAccessible( const Reference< XAccessible >& i_rParentAccessible ) throw (RuntimeException)
    {
        ToolPanelGuard aGuard( *this );
        Reference< XAccessible > xAccessible( m_pControl->GetWindow()->GetAccessible( false ) );
        if ( !xAccessible.is() )
        {
            xAccessible.set( m_pControl->CreateAccessibleObject( i_rParentAccessible ) );
            m_pControl->GetWindow()->SetAccessible( xAccessible );
        }
        return xAccessible;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL ToolPanel::disposing()
    {
        m_pControl.reset();
    }

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................
