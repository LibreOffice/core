/*************************************************************************
 *
 *  $RCSfile: Controller.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mi $ $Date: 2004-10-14 10:37:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., September, 2004
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2004 by Sun Microsystems, Inc.
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
 *  Copyright: 2004 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

