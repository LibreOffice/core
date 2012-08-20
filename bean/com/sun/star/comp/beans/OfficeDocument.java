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

package com.sun.star.comp.beans;

import com.sun.star.uno.UnoRuntime;

//==========================================================================
/** Wrapper class for service OfficeDocument which emulates the upcoming
     mode of automatic runtime Java classes to get rid of the need for
    queryInterface.

    See further information on the wrapping and compatibility limitations
    in the base class Wrapper.

    @since OOo 2.0.0
 */
public class OfficeDocument extends Wrapper
    implements
        com.sun.star.frame.XModel,
        com.sun.star.util.XModifiable,
        com.sun.star.frame.XStorable,
        com.sun.star.view.XPrintable
{
    private com.sun.star.frame.XModel xModel;
    private com.sun.star.util.XModifiable xModifiable;
    private com.sun.star.view.XPrintable xPrintable;
    private com.sun.star.frame.XStorable xStorable;

    public OfficeDocument( com.sun.star.frame.XModel xModel )
    {
        super( xModel );

        this.xModel = xModel;
        this.xModifiable = UnoRuntime.queryInterface(
            com.sun.star.util.XModifiable.class, xModel );
        this.xPrintable = UnoRuntime.queryInterface(
            com.sun.star.view.XPrintable.class, xModel );
        this.xStorable = UnoRuntime.queryInterface(
            com.sun.star.frame.XStorable.class, xModel );
    }

    //==========================================================
    // com.sun.star.frame.XModel
    //----------------------------------------------------------

    public boolean attachResource( /*IN*/String aURL,
            /*IN*/com.sun.star.beans.PropertyValue[] aArguments )
    {
        return xModel.attachResource( aURL, aArguments );
    }

    public String getURL(  )
    {
        return xModel.getURL();
    }

    public com.sun.star.beans.PropertyValue[] getArgs(  )
    {
        return xModel.getArgs();
    }

    public void connectController(
        /*IN*/ com.sun.star.frame.XController xController )
    {
        xModel.connectController( xController );
    }

    public void disconnectController(
        /*IN*/ com.sun.star.frame.XController xController )
    {
        xModel.disconnectController( xController );
    }

    public void lockControllers(  )
    {
        xModel.lockControllers();
    }

    public void unlockControllers(  )
    {
        xModel.unlockControllers();
    }

    public boolean hasControllersLocked(  )
    {
        return xModel.hasControllersLocked();
    }

    public com.sun.star.frame.XController getCurrentController(  )
    {
        return xModel.getCurrentController();
    }

    public void setCurrentController(
        /*IN*/ com.sun.star.frame.XController xController )
        throws com.sun.star.container.NoSuchElementException
    {
        xModel.setCurrentController( xController );
    }

    public java.lang.Object getCurrentSelection(  )
    {
        return xModel.getCurrentSelection();
    }

    //==========================================================
    // com.sun.star.util.XModifyBroadcaster
    //----------------------------------------------------------

    public void addModifyListener(
        /*IN*/ com.sun.star.util.XModifyListener xListener )
    {
        xModifiable.addModifyListener( xListener );
    }

    public void removeModifyListener(
        /*IN*/ com.sun.star.util.XModifyListener xListener )
    {
        xModifiable.removeModifyListener( xListener );
    }

    //==========================================================
    // com.sun.star.util.XModifiable
    //----------------------------------------------------------

    public boolean isModified(  )
    {
        return xModifiable.isModified();
    }

    public void setModified( /*IN*/boolean bModified )
        throws com.sun.star.beans.PropertyVetoException
    {
        xModifiable.setModified( bModified );
    }

    //==========================================================
    // com.sun.star.view.XPrintable
    //----------------------------------------------------------

    public com.sun.star.beans.PropertyValue[] getPrinter(  )
    {
        return xPrintable.getPrinter();
    }

    public void setPrinter( /*IN*/ com.sun.star.beans.PropertyValue[] aPrinter )
        throws com.sun.star.lang.IllegalArgumentException
    {
        xPrintable.setPrinter( aPrinter );
    }

    public void print( /*IN*/ com.sun.star.beans.PropertyValue[] xOptions )
        throws com.sun.star.lang.IllegalArgumentException
    {
        xPrintable.print( xOptions );
    }

    //==========================================================
    // com.sun.star.frame.XStorable
    //----------------------------------------------------------

    public boolean hasLocation(  )
    {
        return xStorable.hasLocation();
    }

    public String getLocation(  )
    {
        return xStorable.getLocation();
    }

    public boolean isReadonly(  )
    {
        return xStorable.isReadonly();
    }

    public void store(  )
        throws com.sun.star.io.IOException
    {
        xStorable.store();
    }

    public void storeAsURL( /*IN*/ String aURL, /*IN*/ com.sun.star.beans.PropertyValue[] aArguments )
        throws com.sun.star.io.IOException
    {
        xStorable.storeAsURL( aURL, aArguments );
    }

    public void storeToURL( /*IN*/ String aURL, /*IN*/ com.sun.star.beans.PropertyValue[] aArguments )
        throws com.sun.star.io.IOException
    {
        xStorable.storeToURL( aURL, aArguments );
    }

};



