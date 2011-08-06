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
package com.sun.star.wizards.ui.event;

import com.sun.star.awt.XItemListener;
import com.sun.star.awt.XRadioButton;
import com.sun.star.uno.UnoRuntime;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class RadioDataAware extends DataAware
{

    protected XRadioButton[] radioButtons;

    public RadioDataAware(Object data, Value value, Object[] radioButs)
    {
        super(data, value);
        radioButtons = new XRadioButton[radioButs.length];
        for (int i = 0; i < radioButs.length; i++)
        {
            radioButtons[i] = UnoRuntime.queryInterface(XRadioButton.class, radioButs[i]);
        }
    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.ui.DataAware#setToUI(java.lang.Object)
     */
    protected void setToUI(Object value)
    {
        int selected = ((Number) value).intValue();
        if (selected == -1)
        {
            for (int i = 0; i < radioButtons.length; i++)
            {
                radioButtons[i].setState(false);
            }
        }
        else
        {
            radioButtons[selected].setState(true);
        }
    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.ui.DataAware#getFromUI()
     */
    protected Object getFromUI()
    {
        for (int i = 0; i < radioButtons.length; i++)
        {
            if (radioButtons[i].getState())
            {
                return new Integer(i);
            }
        }
        return new Integer(-1);
    }

    public static DataAware attachRadioButtons(Object data, String dataProp, Object[] buttons, final Listener listener, boolean field)
    {
        final RadioDataAware da = new RadioDataAware(data,
                field
                ? DataAwareFields.getFieldValueFor(data, dataProp, 0)
                : new DataAware.PropertyValue(dataProp, data), buttons);
        XItemListener xil = UnoDataAware.itemListener(da, listener);
        for (int i = 0; i < da.radioButtons.length; i++)
        {
            da.radioButtons[i].addItemListener(xil);
        }
        return da;
    }
}
