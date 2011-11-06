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

