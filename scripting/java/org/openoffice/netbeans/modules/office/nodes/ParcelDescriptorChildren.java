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
