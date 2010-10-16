/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
        ::osl::SolarMutexGuard aSolarGuard( GetMutex() );
        return new ToolPanelDrawerContext( *this );
    }

//......................................................................................................................
} // namespace svt
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
