package org.openoffice.netbeans.modules.office.actions;

import org.w3c.dom.NodeList;
import javax.swing.event.ChangeListener;
import org.openide.nodes.Node;

public interface ParcelDescriptorParserCookie extends Node.Cookie
{
    // should return a NodeList of org.w3c.dom.Element
    public NodeList getScriptElements();

    public void addChangeListener(ChangeListener cl);

    public void removeChangeListener(ChangeListener cl);
}