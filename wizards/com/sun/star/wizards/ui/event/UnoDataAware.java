/*************************************************************************
 *
 *  $RCSfile: UnoDataAware.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:10:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 */

package com.sun.star.wizards.ui.event;

import com.sun.star.awt.*;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;

/**
 * @author rpiterman
 *
 * This class suppoprts imple cases where a UI control can
 * be directly synchronized with a data property.
 * Such controls are: the different text controls
 * (synchronizing the "Text" , "Value", "Date", "Time" property),
 * Checkbox controls, Dropdown listbox controls (synchronizing the
 * SelectedItems[] property.
 * For those controls, static convenience methods are offered, to simplify use.
 */
public class UnoDataAware extends DataAware {

    protected Object unoControl;
    protected Object unoModel;
    protected String unoPropName;
    protected Object[] disableObjects = new Object[0];
    protected boolean inverse = false;

    protected UnoDataAware(Object dataObject, Value value, Object unoObject_, String unoPropName_) {
        super(dataObject, value);
        unoControl = unoObject_;
        unoModel = getModel(unoControl);
        unoPropName = unoPropName_;
    }

    public void setInverse(boolean i) {
        inverse = i;
    }

    protected void enableControls(Object value) {
        Boolean b = getBoolean(value);
        if (inverse)
            b = b.booleanValue() ? Boolean.FALSE : Boolean.TRUE;
        for (int i = 0; i < disableObjects.length; i++)
            setEnabled(disableObjects[i], b);
    }

    protected void setToUI(Object value) {
        //System.out.println("Settings uno property : "+ Helper.getUnoPropertyValue(this.unoModel,"Name") + "<-" +stringof(value));
        Helper.setUnoPropertyValue(unoModel, unoPropName, value);
    }

    private String stringof(Object value) {
        if (value.getClass().isArray()) {
            StringBuffer sb = new StringBuffer("[");
            for (int i = 0 ; i<((short[])value).length; i++)
                sb.append( ((short[])value)[i] ) .append(" , ");
            sb.append("]");
            return sb.toString();
        }
        else return value.toString();



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
    protected Boolean getBoolean(Object value) {
        if (value == null)
            return Boolean.FALSE;
        if (value instanceof Boolean)
            return (Boolean) value;
        else if (value.getClass().isArray())
            return ((short[]) value).length != 0 ? Boolean.TRUE : Boolean.FALSE;
        else if (value.equals(""))
            return Boolean.FALSE;
        else if (value instanceof Number)
            return ((Number) value).intValue() == 0 ? Boolean.TRUE : Boolean.FALSE;
        else
            return Boolean.TRUE;
    }

    public void disableControls(Object[] controls) {
        disableObjects = controls;
    }

    protected Object getFromUI() {
        return Helper.getUnoPropertyValue(unoModel, unoPropName);
    }

    private static UnoDataAware attachTextControl(Object data, String prop, Object unoText, final Listener listener, String unoProperty, boolean field, Object value) {
        XTextComponent text = (XTextComponent) UnoRuntime.queryInterface(XTextComponent.class, unoText);
        final UnoDataAware uda = new UnoDataAware(data,
            field
                ?   DataAwareFields.getFieldValueFor(data,prop,value)
                :   new DataAware.PropertyValue(prop,data),
            text, unoProperty);
        text.addTextListener(new XTextListener() {
            public void textChanged(TextEvent te) {
                uda.updateData();
                if (listener != null)
                    listener.eventPerformed(te);
            }
            public void disposing(EventObject eo) {
            }
        });
        return uda;
    }


    public static UnoDataAware attachEditControl(Object data, String prop, Object unoControl, Listener listener, boolean field) {
        return attachTextControl(data, prop, unoControl, listener, "Text",field, "");
    }


    public static UnoDataAware attachDateControl(Object data, String prop, Object unoControl, Listener listener, boolean field) {
        return attachTextControl(data, prop, unoControl, listener, "Date",field, null);
    }


    public static UnoDataAware attachTimeControl(Object data, String prop, Object unoControl, Listener listener, boolean field) {
        return attachTextControl(data, prop, unoControl, listener, "Time", field, null);
    }


    public static UnoDataAware attachNumericControl(Object data, String prop, Object unoControl, Listener listener, boolean field) {
        return attachTextControl(data, prop, unoControl, listener, "Value",field, new Double(0));
    }

    public static UnoDataAware attachCheckBox(Object data, String prop, Object checkBox, final Listener listener, boolean field) {
        XCheckBox xcheckBox = ((XCheckBox) UnoRuntime.queryInterface(XCheckBox.class, checkBox));
        final UnoDataAware uda = new UnoDataAware(data,
            field
                ? DataAwareFields.getFieldValueFor(data,prop,new Short((short)0))
                : new DataAware.PropertyValue(prop,data),
            checkBox, "State");
        xcheckBox.addItemListener(itemListener(uda, listener));
        return uda;
    }

    static XItemListener itemListener(final DataAware da, final Listener listener) {
        return new XItemListener() {
            public void itemStateChanged(ItemEvent ie) {
                da.updateData();
                if (listener != null )
                    listener.eventPerformed(ie);
            }
            public void disposing(EventObject eo) {
            }
        };
    }

    public static UnoDataAware attachListBox(Object data, String prop, Object listBox, final Listener listener, boolean field) {
        XListBox xListBox = (XListBox) UnoRuntime.queryInterface(XListBox.class, listBox);
        final UnoDataAware uda = new UnoDataAware(data,
            field
                ? DataAwareFields.getFieldValueFor(data,prop,new short[0])
                : new DataAware.PropertyValue(prop,data),
            listBox, "SelectedItems");
        xListBox.addItemListener(itemListener(uda, listener));
        return uda;
    }

    public static Object getModel(Object control) {
        return ((XControl) UnoRuntime.queryInterface(XControl.class, control)).getModel();
    }

    public static void setEnabled(Object control, boolean enabled) {
        setEnabled(control, enabled ? Boolean.TRUE : Boolean.FALSE);
    }

    public static void setEnabled(Object control, Boolean enabled) {
        Helper.setUnoPropertyValue(getModel(control), "Enabled", enabled);
    }

}
