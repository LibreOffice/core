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


