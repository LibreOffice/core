/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2009 by Sun Microsystems, Inc.
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

package com.sun.star.comp.Calc.NLPSolver.dialogs.controls;

import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.ElementExistException;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import java.util.logging.Level;
import java.util.logging.Logger;

public abstract class BaseControl {

    protected XComponentContext context;
    private Object unoModel;
    protected Object unoControl;
    private XPropertySet properties;


    public abstract String getName();

    public Object getUnoModel() {
        return unoModel;
    }

    /**
     * This is used <b>internally</b> to update the UnoModel and refresh the
     * associated PropertySet.
     * @param unoModel The new UnoModel for this control.
     */
    protected void setUnoModel(Object unoModel) {
        this.unoModel = unoModel;
        properties = UnoRuntime.queryInterface(XPropertySet.class, unoModel);
    }

    public void setParentControl(BaseControl parentControl) {
        //TODO : remove from existing parentControl
        try {
            String name = getName();
            XNameContainer nameContainer = UnoRuntime.queryInterface(XNameContainer.class, parentControl.unoModel);
            nameContainer.insertByName(name, unoModel);

            XControlContainer controlContainer = UnoRuntime.queryInterface(XControlContainer.class, parentControl.unoControl);
            unoControl = controlContainer.getControl(name);

        } catch (IllegalArgumentException ex) {
            Logger.getLogger(BaseControl.class.getName()).log(Level.SEVERE, null, ex);
        } catch (ElementExistException ex) {
            Logger.getLogger(BaseControl.class.getName()).log(Level.SEVERE, null, ex);
        } catch (WrappedTargetException ex) {
            Logger.getLogger(BaseControl.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    public BaseControl(XComponentContext context) {
        this.context = context;
        unoModel = null;
        unoControl = null;
    }

    protected void setProperty(String name, Object value) {
        try {
            properties.setPropertyValue(name, value);
        } catch (UnknownPropertyException ex) {
            Logger.getLogger(BaseControl.class.getName()).log(Level.SEVERE, null, ex);
        } catch (PropertyVetoException ex) {
            Logger.getLogger(BaseControl.class.getName()).log(Level.SEVERE, null, ex);
        } catch (IllegalArgumentException ex) {
            Logger.getLogger(BaseControl.class.getName()).log(Level.SEVERE, null, ex);
        } catch (WrappedTargetException ex) {
            Logger.getLogger(BaseControl.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    protected Object getProperty(String name) {
        try {
            return properties.getPropertyValue(name);
        } catch (UnknownPropertyException ex) {
            Logger.getLogger(BaseControl.class.getName()).log(Level.SEVERE, null, ex);
        } catch (WrappedTargetException ex) {
            Logger.getLogger(BaseControl.class.getName()).log(Level.SEVERE, null, ex);
        }
        return null;
    }

    // <editor-fold defaultstate="collapsed" desc="Uno Properties">

    public void setPosition(int x, int y) {
        setProperty("PositionX", Integer.valueOf(x));
        setProperty("PositionY", Integer.valueOf(y));
    }

    public void setSize(int width, int height) {
        setProperty("Width", Integer.valueOf(width));
        setProperty("Height", Integer.valueOf(height));
    }

    public void setEnabled(boolean enabled) {
        setProperty("Enabled", Boolean.valueOf(enabled));
    }

    public void setVisible(boolean visible) {
        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class, unoControl);
        xWindow.setVisible(visible);
    }

    // </editor-fold>

}
