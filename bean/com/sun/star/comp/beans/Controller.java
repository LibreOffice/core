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

