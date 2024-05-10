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
#include <osl/diagnose.h>
#include <comphelper/diagnose_ex.hxx>


namespace svx
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::beans;

    ToolboxAccess::ToolboxAccess( std::u16string_view rToolboxName ) :
        m_sToolboxResName   ( u"private:resource/toolbar/"_ustr )
    {
        m_sToolboxResName += rToolboxName;

        // the layout manager
        SfxViewFrame* pViewFrm = SfxViewFrame::Current();
        if (!pViewFrm)
            return;

        try
        {
            Reference< XFrame > xFrame = pViewFrm->GetFrame().GetFrameInterface();
            Reference< XPropertySet > xFrameProps( xFrame, UNO_QUERY );
            if ( xFrameProps.is() )
                xFrameProps->getPropertyValue( u"LayoutManager"_ustr ) >>= m_xLayouter;
        }
        catch ( Exception const & )
        {
            TOOLS_WARN_EXCEPTION( "svx.tbxcrtls", "ToolboxAccess::Ctor()" );
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
            TOOLS_WARN_EXCEPTION( "svx", "ToolboxAccess::toggleToolbox" );
        }
    }


    bool ToolboxAccess::isToolboxVisible() const
    {
        return ( m_xLayouter.is() && m_xLayouter->isElementVisible( m_sToolboxResName ) );
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
