/************************************************************************* *
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:  $
 * $Revision:  $
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


#include "precompiled_sw.hxx"

#include <SidebarWinAcc.hxx>

#include <SidebarWin.hxx>
#include <viewsh.hxx>
#include <accmap.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>

namespace css = ::com::sun::star;

namespace sw { namespace sidebarwindows {

// =============================================================================
// declaration and implementation of accessible context for <SidebarWinAccessible> instance
// =============================================================================
class SidebarWinAccessibleContext : public VCLXAccessibleComponent
{
    public:
        explicit SidebarWinAccessibleContext( SwSidebarWin& rSidebarWin,
                                              ViewShell& rViewShell,
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
            vos::OGuard aGuard(maMutex);

            mpAnchorFrm = pAnchorFrm;
        }

        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
            getAccessibleParent() throw (css::uno::RuntimeException)
        {
            vos::OGuard aGuard(maMutex);

            css::uno::Reference< css::accessibility::XAccessible > xAccParent;

            if ( mpAnchorFrm &&
                 mrViewShell.GetAccessibleMap() )
            {
                xAccParent = mrViewShell.GetAccessibleMap()->GetContext( mpAnchorFrm, sal_False );
            }

            return xAccParent;
        }

        virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw (css::uno::RuntimeException)
        {
            vos::OGuard aGuard(maMutex);

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
        ViewShell& mrViewShell;
        const SwFrm* mpAnchorFrm;

        ::vos::OMutex maMutex;
};

// =============================================================================
// implementaion of accessible for <SwSidebarWin> instance
// =============================================================================
SidebarWinAccessible::SidebarWinAccessible( SwSidebarWin& rSidebarWin,
                                            ViewShell& rViewShell,
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

