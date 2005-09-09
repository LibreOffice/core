/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SimpleDataAware.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:52:14 $
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
package com.sun.star.wizards.ui.event;


public class SimpleDataAware extends DataAware {

    protected Object control;
    protected Object[] disableObjects = new Object[0];
    protected Value controlValue;

    public SimpleDataAware(Object dataObject, Value value, Object control_, Value controlValue_ ) {
        super(dataObject, value);
        control = control_;
        controlValue = controlValue_;
    }

    /*
    protected void enableControls(Object value) {
        Boolean b = getBoolean(value);
        for (int i = 0; i<disableObjects.length; i++)
            UIHelper.setEnabled(disableObjects[i],b);
    }
    */

    protected void setToUI(Object value) {
        controlValue.set(value,control);
    }

    /**
     * Try to get from an arbitrary object a boolean value.
     * Null returns Boolean.FALSE;
     * A Boolean object returns itself.
     * An Array returns true if it not empty.
     * An Empty String returns Boolean.FALSE.
     * everything else returns a Boolean.TRUE.
     * @param value
     * @return
     */
    /*protected Boolean getBoolean(Object value) {
        if (value==null)
            return Boolean.FALSE;
        if (value instanceof Boolean)
          return (Boolean)value;
        else if (value.getClass().isArray())
          return ((short[])value).length != 0 ? Boolean.TRUE : Boolean.FALSE;
        else if (value.equals("")) return Boolean.FALSE;
        else return Boolean.TRUE;
    }

    public void disableControls(Object[] controls) {
        disableObjects = controls;
    }
    */

    protected Object getFromUI() {
        return controlValue.get(control);
    }

}
