/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RadioDataAware.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:51:59 $
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
 ************************************************************************/package com.sun.star.wizards.ui.event;

import com.sun.star.awt.XItemListener;
import com.sun.star.awt.XRadioButton;
import com.sun.star.uno.UnoRuntime;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class RadioDataAware extends DataAware {
    protected XRadioButton[] radioButtons;

    public RadioDataAware(Object data, Value value, Object[] radioButs) {
        super(data, value);
        radioButtons = new XRadioButton[radioButs.length];
        for (int i = 0; i < radioButs.length; i++)
            radioButtons[i] = (XRadioButton) UnoRuntime.queryInterface(XRadioButton.class, radioButs[i]);
    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.ui.DataAware#setToUI(java.lang.Object)
     */
    protected void setToUI(Object value) {
        int selected = ((Number) value).intValue();
        if (selected == -1)
            for (int i = 0; i < radioButtons.length; i++)
                radioButtons[i].setState(false);
        else
            radioButtons[selected].setState(true);
    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.ui.DataAware#getFromUI()
     */
    protected Object getFromUI() {
        for (int i = 0; i < radioButtons.length; i++)
            if (radioButtons[i].getState())
                return new Integer(i);
        return new Integer(-1);
    }

    public static DataAware attachRadioButtons(Object data, String dataProp, Object[] buttons, final Listener listener, boolean field) {
        final RadioDataAware da = new RadioDataAware(data,
            field
                ? DataAwareFields.getFieldValueFor(data,dataProp,new Integer(0))
                : new DataAware.PropertyValue(dataProp,data)
            , buttons);
        XItemListener xil = UnoDataAware.itemListener(da, listener);
        for (int i = 0; i < da.radioButtons.length; i++)
            da.radioButtons[i].addItemListener(xil);
        return da;
    }

}
