/*
 * XMLHelper.java
 *
 * Created on 30. September 2003, 15:38
 */

package com.sun.star.wizards.common;

import org.w3c.dom.*;

/**
 *
 * @author  rpiterman
 */
public class XMLHelper {

    public static Node addElement(Node parent, String name, String[] attNames, String[] attValues) {
        Document doc = parent.getOwnerDocument();
        if (doc == null)
            doc = (Document) parent;
        Element e = doc.createElement(name);
        for (int i = 0; i < attNames.length; i++)
            if (attValues[i] != null && (!attValues[i].equals("")))
                e.setAttribute(attNames[i], attValues[i]);
        parent.appendChild(e);
        return e;
    }

    public static Node addElement(Node parent, String name, String attNames, String attValues) {
        return addElement(parent, name, new String[] { attNames }, new String[] { attValues });
    }

}
