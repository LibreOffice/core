/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "svx/tbxcolor.hxx"
#include <sfx2/viewfrm.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

//........................................................................
namespace svx
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::frame;

    #define DECLARE_ASCII(s)        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(s) )
    #define TOOLBAR_RESNAME         DECLARE_ASCII("private:resource/toolbar/")
    #define PROPNAME_LAYOUTMANAGER  DECLARE_ASCII("LayoutManager")

    //====================================================================
    //= ToolboxAccess
    //====================================================================
    ToolboxAccess::ToolboxAccess( const ::rtl::OUString& rToolboxName ) :

        m_bDocking          ( false ),
        m_sToolboxResName   ( TOOLBAR_RESNAME )

    {
        m_sToolboxResName += rToolboxName;

        // the layout manager
        if ( SfxViewFrame::Current() )
        {
            try
            {
                Reference< XFrame > xFrame = SfxViewFrame::Current()->GetFrame().GetFrameInterface();
                Reference< XPropertySet > xFrameProps( xFrame, UNO_QUERY );
                if ( xFrameProps.is() )
                    xFrameProps->getPropertyValue( PROPNAME_LAYOUTMANAGER ) >>= m_xLayouter;
            }
            catch ( Exception& )
            {
                DBG_ERRORFILE( "ToolboxAccess::Ctor(): exception" );
            }
        }
    }

    //--------------------------------------------------------------------
    void ToolboxAccess::toggleToolbox() const
    {
        try
        {
            Reference< XLayoutManager > xManager( m_xLayouter );
            OSL_ENSURE( xManager. is(), "ToolboxAccess::toggleToolbox: couldn't obtain the layout manager!" );
            if ( xManager. is() )
            {
                if ( xManager->isElementVisible( m_sToolboxResName ) )
                {
                    xManager->hideElement( m_sToolboxResName );
                    xManager->destroyElement( m_sToolboxResName );
                }
                else
                {
                    xManager->createElement( m_sToolboxResName );
                    xManager->showElement( m_sToolboxResName );
                    ::com::sun::star::awt::Point aPos;

                    if ( m_bDocking )
                        xManager->dockWindow( m_sToolboxResName,
                            ::com::sun::star::ui::DockingArea_DOCKINGAREA_BOTTOM, aPos );
                }
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "ToolboxAccess::toggleToolbox: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    bool ToolboxAccess::isToolboxVisible() const
    {
        return ( m_xLayouter.is() && m_xLayouter->isElementVisible( m_sToolboxResName ) );
    }

//........................................................................
} // namespace svx
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
