/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DialogFocusTraversalPolicy.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 12:00:42 $
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

package org.openoffice.setup.Util;

import java.awt.FocusTraversalPolicy;
import javax.swing.JComponent;

public class DialogFocusTraversalPolicy extends FocusTraversalPolicy {

    private JComponent order[];
    private java.util.List list;

    public DialogFocusTraversalPolicy(JComponent _order[]) {
        order = _order;
        list = java.util.Arrays.asList(order);
    }

    public java.awt.Component getFirstComponent(java.awt.Container focusCycleRoot) {
        return order[0];
    }

    public java.awt.Component getLastComponent(java.awt.Container focusCycleRoot) {
        return order[order.length - 1];
    }

    public java.awt.Component getComponentAfter(java.awt.Container focusCycleRoot, java.awt.Component aComponent) {
        int index = 0,x = -1;
        index = list.indexOf(aComponent);
        index++; // increasing automatically
        if(!order[index % order.length].isEnabled() ||
           !order[index % order.length].isVisible()) {
            x = index;
            index = -1;
            for (; x != order.length; x++) {
                if(order[x].isEnabled() && order[x].isVisible()) {
                    index = x;
                    break;
                }
            }
            if(index == -1) {
                x = list.indexOf(aComponent);
                for(int y = 0; y <= x; y++) {
                    if(order[y].isEnabled() && order[x].isVisible()) {
                        index = y;
                        break;
                    }
                }
            }
        }
        return order[ index % order.length];
    }

    public java.awt.Component getComponentBefore(java.awt.Container focusCycleRoot, java.awt.Component aComponent) {
        int index = list.indexOf(aComponent), x = -1;
        index--;
        if(!order[(index + order.length) % order.length].isEnabled() ||
           !order[(index + order.length) % order.length].isVisible()) {
            x = index;
            index = -1;
            for(; x >= 0; x--) {
                if(order[x].isEnabled() && order[x].isVisible()) {
                    index = x;
                    break;
                }
            }
            // if nothing has changed
            if(index == -1) {
                x = list.indexOf(aComponent);
                for(int y = order.length -1; y >= x; y--) {
                    if(order[y].isEnabled() && order[x].isVisible()) {
                        index = y;
                        break;
                    }
                }
            }
        }
        return order[ (index + order.length) % order.length];
    }

    public java.awt.Component getDefaultComponent(java.awt.Container focusCycleRoot) {
        return order[0];
    }

    public java.awt.Component getInitialComponent(java.awt.Window window) {
        return order[0];
    }

}


