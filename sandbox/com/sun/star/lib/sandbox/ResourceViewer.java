/*************************************************************************
 *
 *  $RCSfile: ResourceViewer.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:24:28 $
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

