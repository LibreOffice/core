/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ResourceViewer.java,v $
 * $Revision: 1.3 $
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

package com.sun.star.lib.sandbox;

import java.applet.Applet;

import java.awt.BorderLayout;
import java.awt.Button;
import java.awt.Frame;
import java.awt.Graphics;
import java.awt.List;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import java.util.Enumeration;

import java.util.Observable;
import java.util.Observer;

public class ResourceViewer extends Frame implements Observer {
    List list;
    boolean refill;

    public ResourceViewer() {
        super("ResourceViewer");
        list = new List();

        setLayout(new BorderLayout());
        add("Center", list);

        Button clearCashButton = new Button("clear the cache");
        clearCashButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent actionEvent) {
                WeakTable.clear();
            }
        });
        add("South", clearCashButton);

        setSize(500, 200);

        refill = true;

        WeakTable.AddObserver(this);

        setVisible(true);
    }

    void refresh() {
        list.removeAll();

        Enumeration elements = WeakTable.elements();
        while(elements.hasMoreElements()) {
            WeakRef weakRef = ((WeakEntry)elements.nextElement()).weakRef;
            list.add(weakRef.getRefCount() + " " + weakRef.getKey().toString());
        }
    }

    public void update(Observable observable, Object arg) {
          refill = true;
        repaint(500);
    }

    public void paint(Graphics g) {
        if(refill) {
            refill = false;
            refresh();

        }
        super.paint(g);
    }
}

