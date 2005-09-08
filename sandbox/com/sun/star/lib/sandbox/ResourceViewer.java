/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ResourceViewer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:08:26 $
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

