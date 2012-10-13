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
package com.sun.star.wizards.ui.event;

import com.sun.star.awt.XItemListener;
import com.sun.star.awt.XRadioButton;
import com.sun.star.uno.UnoRuntime;

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
