/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OfficeDocument.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:02:34 $
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
        this.xModifiable = (com.sun.star.util.XModifiable)
            UnoRuntime.queryInterface(
                com.sun.star.util.XModifiable.class, xModel );
        this.xPrintable = (com.sun.star.view.XPrintable)
            UnoRuntime.queryInterface(
                com.sun.star.view.XPrintable.class, xModel );
        this.xStorable = (com.sun.star.frame.XStorable)
            UnoRuntime.queryInterface(
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



