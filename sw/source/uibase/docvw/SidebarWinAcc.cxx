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

#include <SidebarWinAcc.hxx>

#include <SidebarWin.hxx>
#include <viewsh.hxx>
#include <accmap.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>

namespace sw { namespace sidebarwindows {

// declaration and implementation of accessible context for <SidebarWinAccessible> instance
class SidebarWinAccessibleContext : public VCLXAccessibleComponent
{
    public:
        explicit SidebarWinAccessibleContext( SwSidebarWin& rSidebarWin,
                                              SwViewShell& rViewShell,
                                              const SwFrm* pAnchorFrm )
            : VCLXAccessibleComponent( rSidebarWin.GetWindowPeer() )
            , mrViewShell( rViewShell )
            , mpAnchorFrm( pAnchorFrm )
            , maMutex()
        {
            rSidebarWin.SetAccessibleRole( css::accessibility::AccessibleRole::COMMENT );
        }

        virtual ~SidebarWinAccessibleContext()
        {}

        void ChangeAnchor( const SwFrm* pAnchorFrm )
        {
            osl::MutexGuard aGuard(maMutex);

            mpAnchorFrm = pAnchorFrm;
        }

        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
            getAccessibleParent() throw (css::uno::RuntimeException, std::exception) override
        {
            osl::MutexGuard aGuard(maMutex);

            css::uno::Reference< css::accessibility::XAccessible > xAccParent;

            if ( mpAnchorFrm &&
                 mrViewShell.GetAccessibleMap() )
            {
                xAccParent = mrViewShell.GetAccessibleMap()->GetContext( mpAnchorFrm, false );
            }

            return xAccParent;
        }

        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw (css::uno::RuntimeException, std::exception) override
        {
            osl::MutexGuard aGuard(maMutex);

            sal_Int32 nIndex( -1 );

            if ( mpAnchorFrm && GetWindow() &&
                 mrViewShell.GetAccessibleMap() )
            {
                nIndex = mrViewShell.GetAccessibleMap()->GetChildIndex( *mpAnchorFrm,
                                                                        *GetWindow() );
            }

            return nIndex;
        }

    private:
        SwViewShell& mrViewShell;
        const SwFrm* mpAnchorFrm;

        ::osl::Mutex maMutex;
};

// implementation of accessible for <SwSidebarWin> instance
SidebarWinAccessible::SidebarWinAccessible( SwSidebarWin& rSidebarWin,
                                            SwViewShell& rViewShell,
                                            const SwSidebarItem& rSidebarItem )
    : VCLXWindow()
    , mrSidebarWin( rSidebarWin )
    , mrViewShell( rViewShell )
    , mpAnchorFrm( rSidebarItem.maLayoutInfo.mpAnchorFrm )
    , bAccContextCreated( false )
{
    SetWindow( &mrSidebarWin );
}

SidebarWinAccessible::~SidebarWinAccessible()
{
}

void SidebarWinAccessible::ChangeSidebarItem( const SwSidebarItem& rSidebarItem )
{
    if ( bAccContextCreated )
    {
        css::uno::Reference< css::accessibility::XAccessibleContext > xAcc
                                                    = getAccessibleContext();
        if ( xAcc.is() )
        {
            SidebarWinAccessibleContext* pAccContext =
                        dynamic_cast<SidebarWinAccessibleContext*>(xAcc.get());
            if ( pAccContext )
            {
                pAccContext->ChangeAnchor( rSidebarItem.maLayoutInfo.mpAnchorFrm );
            }
        }
    }
}

css::uno::Reference< css::accessibility::XAccessibleContext > SidebarWinAccessible::CreateAccessibleContext()
{
    SidebarWinAccessibleContext* pAccContext =
                                new SidebarWinAccessibleContext( mrSidebarWin,
                                                                 mrViewShell,
                                                                 mpAnchorFrm );
    css::uno::Reference< css::accessibility::XAccessibleContext > xAcc( pAccContext );
    bAccContextCreated = true;
    return xAcc;
}

} } // end of namespace sw::sidebarwindows

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
