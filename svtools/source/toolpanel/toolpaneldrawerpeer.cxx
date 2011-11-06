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

#include "toolpaneldrawerpeer.hxx"
#include "toolpaneldrawer.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
/** === end UNO includes === **/

#include <tools/diagnose_ex.h>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <vcl/vclevent.hxx>

//......................................................................................................................
namespace svt
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
    using ::com::sun::star::accessibility::XAccessibleContext;
    /** === end UNO using === **/
    namespace AccessibleStateType = ::com::sun::star::accessibility::AccessibleStateType;
    namespace AccessibleEventId = ::com::sun::star::accessibility::AccessibleEventId;

    //==================================================================================================================
    //= ToolPanelDrawerContext
    //==================================================================================================================
    class ToolPanelDrawerContext : public VCLXAccessibleComponent
    {
    public:
        ToolPanelDrawerContext( VCLXWindow& i_rWindow )
            :VCLXAccessibleComponent( &i_rWindow )
        {
        }

        virtual void    ProcessWindowEvent( const VclWindowEvent& i_rVclWindowEvent );
        virtual void    FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& i_rStateSet );

    protected:
        ~ToolPanelDrawerContext()
        {
        }
    };

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawerContext::ProcessWindowEvent( const VclWindowEvent& i_rVclWindowEvent )
    {
        VCLXAccessibleComponent::ProcessWindowEvent( i_rVclWindowEvent );

        switch ( i_rVclWindowEvent.GetId() )
        {
        case VCLEVENT_ITEM_EXPANDED:
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, Any(), makeAny( AccessibleStateType::EXPANDED ) );
            break;
        case VCLEVENT_ITEM_COLLAPSED:
            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, makeAny( AccessibleStateType::EXPANDED ), Any() );
            break;
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawerContext::FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& i_rStateSet )
    {
        VCLXAccessibleComponent::FillAccessibleStateSet( i_rStateSet );
        if ( !GetWindow() )
            return;

        i_rStateSet.AddState( AccessibleStateType::EXPANDABLE );
        i_rStateSet.AddState( AccessibleStateType::FOCUSABLE );

        const ToolPanelDrawer* pDrawer( dynamic_cast< const ToolPanelDrawer* > ( GetWindow() ) );
        ENSURE_OR_RETURN_VOID( pDrawer, "ToolPanelDrawerContext::FillAccessibleStateSet: illegal window!" );
        if ( pDrawer->IsExpanded() )
            i_rStateSet.AddState( AccessibleStateType::EXPANDED );

        if ( pDrawer->HasChildPathFocus() )
            i_rStateSet.AddState( AccessibleStateType::FOCUSED );
    }

    //==================================================================================================================
    //= ToolPanelDrawerPeer
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ToolPanelDrawerPeer::ToolPanelDrawerPeer()
        :VCLXWindow()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ToolPanelDrawerPeer::~ToolPanelDrawerPeer()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessibleContext > ToolPanelDrawerPeer::CreateAccessibleContext()
    {
        ::vos::OGuard aSolarGuard( GetMutex() );
        return new ToolPanelDrawerContext( *this );
    }

//......................................................................................................................
} // namespace svt
//......................................................................................................................
