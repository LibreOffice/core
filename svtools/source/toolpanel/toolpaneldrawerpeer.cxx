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


#include "toolpaneldrawerpeer.hxx"
#include "toolpaneldrawer.hxx"

#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>

#include <tools/diagnose_ex.h>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/svapp.hxx>


namespace svt
{


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

    namespace AccessibleStateType = ::com::sun::star::accessibility::AccessibleStateType;
    namespace AccessibleEventId = ::com::sun::star::accessibility::AccessibleEventId;


    //= ToolPanelDrawerContext

    class ToolPanelDrawerContext : public VCLXAccessibleComponent
    {
    public:
        explicit ToolPanelDrawerContext( VCLXWindow& i_rWindow )
            :VCLXAccessibleComponent( &i_rWindow )
        {
        }

        virtual void    ProcessWindowEvent( const VclWindowEvent& i_rVclWindowEvent ) override;
        virtual void    FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& i_rStateSet ) override;

    protected:
        virtual ~ToolPanelDrawerContext()
        {
        }
    };


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


    void ToolPanelDrawerContext::FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& i_rStateSet )
    {
        VCLXAccessibleComponent::FillAccessibleStateSet( i_rStateSet );
        if ( !GetWindow() )
            return;

        i_rStateSet.AddState( AccessibleStateType::EXPANDABLE );
        i_rStateSet.AddState( AccessibleStateType::FOCUSABLE );

        VclPtr< ToolPanelDrawer > pDrawer = GetAsDynamic< ToolPanelDrawer > ();
        ENSURE_OR_RETURN_VOID( pDrawer, "ToolPanelDrawerContext::FillAccessibleStateSet: illegal window!" );
        if ( pDrawer->IsExpanded() )
            i_rStateSet.AddState( AccessibleStateType::EXPANDED );

        if ( pDrawer->HasChildPathFocus() )
            i_rStateSet.AddState( AccessibleStateType::FOCUSED );
    }


    //= ToolPanelDrawerPeer


    ToolPanelDrawerPeer::ToolPanelDrawerPeer()
        :VCLXWindow()
    {
    }


    ToolPanelDrawerPeer::~ToolPanelDrawerPeer()
    {
    }


    Reference< XAccessibleContext > ToolPanelDrawerPeer::CreateAccessibleContext()
    {
        SolarMutexGuard aSolarGuard;
        return new ToolPanelDrawerContext( *this );
    }


} // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
