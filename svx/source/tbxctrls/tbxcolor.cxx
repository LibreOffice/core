/*************************************************************************
 *
 *  $RCSfile: tbxcolor.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-10-12 10:17:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
    using namespace ::drafts::com::sun::star::frame;

    #define DECLARE_ASCII(s)        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(s) )
    #define TOOLBAR_RESNAME         DECLARE_ASCII("private:resource/toolbar/colorbar")
    #define PROPNAME_LAYOUTMANAGER  DECLARE_ASCII("LayoutManager")

    //====================================================================
    //= ColorToolboxAccess
    //====================================================================
    ColorToolboxAccess::ColorToolboxAccess()
    {
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
                DBG_ERRORFILE( "ColorToolboxAccess::Ctor(): exception" );
            }
        }
    }

    //--------------------------------------------------------------------
    void ColorToolboxAccess::toggleToolbox() const
    {
        try
        {
            Reference< XLayoutManager > xManager( m_xLayouter );
            OSL_ENSURE( xManager. is(), "ColorToolboxAccess::toggleToolbox: couldn't obtain the layout manager!" );
            if ( xManager. is() )
            {
                ::rtl::OUString sToolboxResource( TOOLBAR_RESNAME );
                if ( xManager->isElementVisible( sToolboxResource ) )
                {
                    xManager->hideElement( sToolboxResource );
                    xManager->destroyElement( sToolboxResource );
                }
                else
                {
                    xManager->createElement( sToolboxResource );
                    xManager->showElement( sToolboxResource );
                }
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "ColorToolboxAccess::toggleToolbox: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    bool ColorToolboxAccess::isToolboxVisible() const
    {
        return ( m_xLayouter.is() && m_xLayouter->isElementVisible( TOOLBAR_RESNAME ) );
    }

//........................................................................
} // namespace svx
//........................................................................

