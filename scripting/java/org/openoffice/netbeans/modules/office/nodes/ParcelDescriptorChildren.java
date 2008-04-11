/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ParcelDescriptorChildren.java,v $
 * $Revision: 1.4 $
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

package org.openoffice.netbeans.modules.office.nodes;

import java.util.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.w3c.dom.NodeList;
import org.w3c.dom.Element;

import org.openide.nodes.*;
import org.openoffice.netbeans.modules.office.actions.ParcelDescriptorParserCookie;

/** List of children of a containing node.
 * Remember to document what your permitted keys are!
 *
 * @author tomaso
 */
public class ParcelDescriptorChildren extends Children.Keys implements ChangeListener {

    private ParcelDescriptorParserCookie parserCookie = null;

    public ParcelDescriptorChildren(ParcelDescriptorParserCookie cookie) {
        parserCookie = cookie;
    }

    private void refreshKeys() {
        NodeList nl;
        int len;

        if (parserCookie == null ||
            (nl = parserCookie.getScriptElements()) == null ||
            (len = nl.getLength()) == 0) {
            setKeys(Collections.EMPTY_SET);
            return;
        }

        ArrayList keys = new ArrayList(len);
        for (int i = 0; i < len; i++)
            keys.add(nl.item(i));
        setKeys(keys);
    }

    protected void addNotify() {
        super.addNotify();
        parserCookie.addChangeListener(this);
        refreshKeys();
    }

    protected void removeNotify() {
        super.removeNotify();
        parserCookie.removeChangeListener(this);
        setKeys(Collections.EMPTY_SET);
    }

    protected Node[] createNodes(Object key) {
        Element el = (Element)key;
        System.out.println("element is: " + el);
        return new Node[] {new ScriptNode(el)};
    }

    public void stateChanged(ChangeEvent e) {
        refreshKeys();
    }
}
