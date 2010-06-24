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

package com.sun.star.comp.beans;

import com.sun.star.uno.UnoRuntime;

/** Wrapper class for a com.sun.star.frame.XController.
 *
 * @since OOo 2.0.0
 */
public class Controller
    extends Wrapper
    implements
        com.sun.star.frame.XController
{
    private com.sun.star.frame.XController xController;
    private com.sun.star.frame.XDispatchProvider xDispatchProvider;

    Controller( com.sun.star.frame.XController xController )
    {
        super( xController );
        this.xController = xController;
        xDispatchProvider = (com.sun.star.frame.XDispatchProvider)
            UnoRuntime.queryInterface( com.sun.star.frame.XDispatchProvider.class,
                xController );
    }

    //==============================================================
    // com.sun.star.frame.XController
    //--------------------------------------------------------------

    public void attachFrame( /*IN*/ com.sun.star.frame.XFrame xFrame )
    {
        xController.attachFrame( xFrame );
    }

    public boolean attachModel( /*IN*/ com.sun.star.frame.XModel xModel )
    {
        return xController.attachModel( xModel );
    }

    public boolean suspend( /*IN*/boolean bSuspend )
    {
        return xController.suspend( bSuspend );
    }

    public java.lang.Object getViewData(  )
    {
        return xController.getViewData();
    }

    public void restoreViewData( /*IN*/java.lang.Object aData )
    {
        xController.restoreViewData( aData );
    }

    public com.sun.star.frame.XModel getModel(  )
    {
        return xController.getModel();
    }

    public com.sun.star.frame.XFrame getFrame(  )
    {
        return xController.getFrame();
    }

    //==============================================================
    // com.sun.star.frame.XDispatchProvider
    //--------------------------------------------------------------

    public com.sun.star.frame.XDispatch queryDispatch(
            /*IN*/ com.sun.star.util.URL aURL,
            /*IN*/ String aTargetFrameName,
            /*IN*/ int nSearchFlags )
    {
        return xDispatchProvider.queryDispatch( aURL, aTargetFrameName, nSearchFlags );
    }

    public com.sun.star.frame.XDispatch[] queryDispatches(
            /*IN*/ com.sun.star.frame.DispatchDescriptor[] aRequests )
    {
        return xDispatchProvider.queryDispatches( aRequests );
    }
}

