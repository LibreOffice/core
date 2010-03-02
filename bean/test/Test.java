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
package test;


import java.io.*;
import java.awt.*;
import java.awt.event.*;
import com.sun.star.comp.beans.OOoBean;
import com.sun.star.uno.UnoRuntime;


import java.awt.*;

public class Test
{

    public static void main(String [] args)
    {
        try {
        Frame f = new Frame();
        final OOoBean b = new OOoBean();
        f.add(b, BorderLayout.CENTER);
        f.pack();
        f.setSize(500, 400);
        f.setVisible(true);
        b.loadFromURL("private:factory/swriter", null);
        f.validate();

        f.addWindowListener(new WindowAdapter() {
                public void windowClosing( java.awt.event.WindowEvent e){
                    b.stopOOoConnection();
                    System.exit(0);

                }});
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }
}


