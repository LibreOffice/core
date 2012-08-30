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
package test;


import java.awt.*;
import java.awt.event.*;
import com.sun.star.comp.beans.OOoBean;

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


