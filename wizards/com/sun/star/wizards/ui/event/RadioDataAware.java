/*************************************************************************
 *
 *  $RCSfile: RadioDataAware.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $  $Date: 2004-05-19 13:09:20 $
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
