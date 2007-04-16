/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formtoolbars.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2007-04-16 16:21:44 $
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

#ifndef SVX_SOURCE_INC_FORMTOOLBARS_HXX
#include "formtoolbars.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
/** === end UNO includes === **/

#ifndef _SVX_SVXIDS_HRC
#include "svxids.hrc"
#endif

//........................................................................
namespace svxform
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::frame;

    //====================================================================
    //= FormToolboxes
    //====================================================================
    //--------------------------------------------------------------------
    FormToolboxes::FormToolboxes( const Reference< XFrame >& _rxFrame )
    {
        // the layout manager
        Reference< XPropertySet > xFrameProps( _rxFrame, UNO_QUERY );
        if ( xFrameProps.is() )
            xFrameProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" ) ) ) >>= m_xLayouter;
    }

    //--------------------------------------------------------------------
    void FormToolboxes::toggleToolbox( USHORT _nSlotId ) const
    {
        try
        {
            Reference< XLayoutManager > xManager( m_xLayouter );
            OSL_ENSURE( xManager. is(), "FormToolboxes::toggleToolbox: couldn't obtain the layout manager!" );
            if ( xManager. is() )
            {
                ::rtl::OUString sToolboxResource( getToolboxResourceName( _nSlotId ) );
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
            OSL_ENSURE( sal_False, "FormToolboxes::toggleToolbox: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    bool FormToolboxes::isToolboxVisible( USHORT _nSlotId ) const
    {
        return m_xLayouter.is() && m_xLayouter->isElementVisible(
            getToolboxResourceName( _nSlotId ) );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString FormToolboxes::getToolboxResourceName( USHORT _nSlotId ) const
    {
        OSL_ENSURE( ( _nSlotId == SID_FM_MORE_CONTROLS ) || ( _nSlotId == SID_FM_FORM_DESIGN_TOOLS ) || ( _nSlotId == SID_FM_CONFIG ),
            "FormToolboxes::getToolboxResourceName: unsupported slot!" );

        const sal_Char* pToolBarName = "formcontrols";
        if ( _nSlotId == SID_FM_MORE_CONTROLS )
            pToolBarName = "moreformcontrols";
        else if ( _nSlotId == SID_FM_FORM_DESIGN_TOOLS )
            pToolBarName = "formdesign";

        ::rtl::OUString aToolBarResStr( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/" ));
        aToolBarResStr += ::rtl::OUString::createFromAscii( pToolBarName );
        return aToolBarResStr;
    }

//........................................................................
} // namespace svxform
//........................................................................

