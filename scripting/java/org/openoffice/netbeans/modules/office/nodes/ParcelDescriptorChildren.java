/*************************************************************************
 *
 *  $RCSfile: ParcelDescriptorChildren.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: toconnor $ $Date: 2002-11-13 17:44:37 $
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
