/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Controller.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:59:02 $
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

