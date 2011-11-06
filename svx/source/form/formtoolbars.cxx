/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include "formtoolbars.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/beans/XPropertySet.hpp>
/** === end UNO includes === **/

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
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

