/*************************************************************************
 *
 *  $RCSfile: formtoolbars.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2005-01-05 12:22:16 $
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
    using namespace ::drafts::com::sun::star::frame;

    //====================================================================
    //= FormToolboxes
    //====================================================================
    //--------------------------------------------------------------------
    FormToolboxes::FormToolboxes( const Reference< XFrame >& _rxFrame, DocumentType _eDocType )
        :m_eDocumentType( _eDocType )
    {
        // the document type
        if ( eUnknownDocumentType == m_eDocumentType )
        {
            Reference< XController > xController;
            if ( _rxFrame.is() )
                xController = _rxFrame->getController();
            OSL_ENSURE( xController.is() && xController->getModel().is(), "FormToolboxes::FormToolboxes: can't determine the document type!" );
            if ( xController.is() )
                m_eDocumentType = DocumentClassification::classifyDocument( xController->getModel() );
        }

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

        const sal_Char* pNames[3][3] = {
            { "formcontrols", "xformcontrols", "databasecontrols" },
            { "moreformcontrols", "morexformcontrols", "moredatabasecontrols" },
            { "formdesign", "xformdesign", "databaseformdesign" }
        };
        sal_Int32 nDocIndex = 0, nToolbarIndex = 0;
        switch ( m_eDocumentType )
        {
        case ::svxform::eEnhancedForm:      nDocIndex = 1; break;
        case ::svxform::eDatabaseForm:      nDocIndex = 2; break;
        default:                            nDocIndex = 0; break;
        }
        switch ( _nSlotId )
        {
        case SID_FM_MORE_CONTROLS:      nToolbarIndex = 1; break;
        case SID_FM_FORM_DESIGN_TOOLS:  nToolbarIndex = 2; break;
        default:                        nToolbarIndex = 0; break;
        }

        const sal_Char* pToolBarName = pNames[nToolbarIndex][nDocIndex];

        ::rtl::OUString aToolBarResStr( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/" ));
        aToolBarResStr += ::rtl::OUString::createFromAscii( pToolBarName );
        return aToolBarResStr;
    }

//........................................................................
} // namespace svxform
//........................................................................

