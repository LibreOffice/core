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

#include <formtoolbars.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <osl/diagnose.h>

#include <svx/svxids.hrc>


namespace svxform
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::beans;

    FormToolboxes::FormToolboxes( const Reference< XFrame >& _rxFrame )
    {
        // the layout manager
        Reference< XPropertySet > xFrameProps( _rxFrame, UNO_QUERY );
        if ( xFrameProps.is() )
            xFrameProps->getPropertyValue("LayoutManager") >>= m_xLayouter;
    }


    void FormToolboxes::toggleToolbox( sal_uInt16 _nSlotId ) const
    {
        try
        {
            Reference< XLayoutManager > xManager( m_xLayouter );
            OSL_ENSURE( xManager. is(), "FormToolboxes::toggleToolbox: couldn't obtain the layout manager!" );
            if ( xManager. is() )
            {
                OUString sToolboxResource( getToolboxResourceName( _nSlotId ) );
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
            OSL_FAIL( "FormToolboxes::toggleToolbox: caught an exception!" );
        }
    }


    bool FormToolboxes::isToolboxVisible( sal_uInt16 _nSlotId ) const
    {
        return m_xLayouter.is() && m_xLayouter->isElementVisible(
            getToolboxResourceName( _nSlotId ) );
    }


    OUString FormToolboxes::getToolboxResourceName( sal_uInt16 _nSlotId )
    {
        OSL_ENSURE( _nSlotId == SID_FM_FORM_DESIGN_TOOLS ,
            "FormToolboxes::getToolboxResourceName: unsupported slot!" );

        return "private:resource/toolbar/formdesign";
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
