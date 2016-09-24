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

import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.style.VerticalAlignment;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import java.util.logging.Level;
import java.util.logging.Logger;
import com.sun.star.comp.Calc.NLPSolver.dialogs.BaseDialog;

public class Label extends LabeledControl {

    private String name;

    public Label(BaseDialog owner, String name) {
        super(owner.context);
        try {
            setUnoModel(owner.getMultiServiceFactory().createInstance("com.sun.star.awt.UnoControlFixedTextModel"));
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

    public void setVerticalAlign(VerticalAlignment align) {
        setProperty("VerticalAlign", align);
    }


    public void setTextColor(int RGB) {
        setProperty("TextColor", Integer.valueOf(RGB));
    }

    public int getTextColor() {
        try {
            Object prop = getProperty("TextColor");
            if (AnyConverter.getType(prop) == Type.LONG)
                return AnyConverter.toInt(prop);
        } catch (IllegalArgumentException ex) {
            Logger.getLogger(LabeledControl.class.getName()).log(Level.SEVERE, null, ex);
        }
        return 0;
    }

}
