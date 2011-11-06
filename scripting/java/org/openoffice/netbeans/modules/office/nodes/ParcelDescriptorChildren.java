/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
