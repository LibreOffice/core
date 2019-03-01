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


#include <svx/tbxcolor.hxx>
#include <sfx2/viewfrm.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>


namespace svx
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::beans;

    #define TOOLBAR_RESNAME         "private:resource/toolbar/"
    #define PROPNAME_LAYOUTMANAGER  "LayoutManager"

    ToolboxAccess::ToolboxAccess( const OUString& rToolboxName ) :
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
            catch ( Exception const & )
            {
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN( "svx.tbxcrtls", "ToolboxAccess::Ctor(): exception " << exceptionToString(ex) );
            }
        }
    }


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
                }
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "ToolboxAccess::toggleToolbox: caught an exception!" );
        }
    }


    bool ToolboxAccess::isToolboxVisible() const
    {
        return ( m_xLayouter.is() && m_xLayouter->isElementVisible( m_sToolboxResName ) );
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
