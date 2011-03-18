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
#include "formtoolbars.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/beans/XPropertySet.hpp>
/** === end UNO includes === **/

#include <svx/svxids.hrc>

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
    void FormToolboxes::toggleToolbox( sal_uInt16 _nSlotId ) const
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
    bool FormToolboxes::isToolboxVisible( sal_uInt16 _nSlotId ) const
    {
        return m_xLayouter.is() && m_xLayouter->isElementVisible(
            getToolboxResourceName( _nSlotId ) );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString FormToolboxes::getToolboxResourceName( sal_uInt16 _nSlotId ) const
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
