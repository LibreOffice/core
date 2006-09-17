/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tbxcolor.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 06:07:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "tbxcolor.hxx"

#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

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
                Reference< XFrame > xFrame = SfxViewFrame::Current()->GetFrame()->GetFrameInterface();
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

