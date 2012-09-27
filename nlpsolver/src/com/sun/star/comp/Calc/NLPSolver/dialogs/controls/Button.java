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

import com.sun.star.awt.XActionListener;
import com.sun.star.awt.XButton;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import java.util.logging.Level;
import java.util.logging.Logger;
import com.sun.star.comp.Calc.NLPSolver.dialogs.BaseDialog;

public class Button extends LabeledControl {
    
    private String name;
    private XButton xButton;
    
    public Button(BaseDialog owner, String name) {
        super(owner.context);
        try {
            setUnoModel(owner.getMultiServiceFactory().createInstance("com.sun.star.awt.UnoControlButtonModel"));
            this.name = name;
            setProperty("Name", name);
        } catch (Exception ex) {
            Logger.getLogger(Button.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    @Override
    public String getName() {
        return name;
    }

    @Override
    public void setParentControl(BaseControl parentControl) {
        super.setParentControl(parentControl);
        xButton = (XButton) UnoRuntime.queryInterface(XButton.class, unoControl);
    }
    
    public void addActionListener(XActionListener actionListener) {
        xButton.addActionListener(actionListener);
    }
    
    public void setActionCommand(String actionCommand) {
        xButton.setActionCommand(actionCommand);
    }

}
