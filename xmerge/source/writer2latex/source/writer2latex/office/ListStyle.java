/************************************************************************
 *
 *  ListStyle.java
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
 *  Copyright: 2002 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.3.3 (2004-02-16)
 *
 */

package writer2latex.office;

import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import writer2latex.util.Misc;

/** <p> Class representing a list style (including outline numbering) in OOo Writer </p> */
public class ListStyle extends OfficeStyle {
    // the file format doesn't specify a maximum nesting level, but OOo
    // currently supports 10
    private static final int MAX_LEVEL = 10;
    private PropertySet[] level;
    private PropertySet[] levelStyle;

    public ListStyle() {
        level = new PropertySet[MAX_LEVEL+1];
        levelStyle = new PropertySet[MAX_LEVEL+1];
        for (int i=1; i<=MAX_LEVEL; i++) {
            level[i] = new PropertySet();
            levelStyle[i] = new PropertySet();
        }
    }

    public String getLevelType(int i) {
        if (i>=1 && i<=MAX_LEVEL) {
            return level[i].getName();
        }
        else {
            return null;
        }
    }

    public boolean isNumber(int i) {
        return XMLString.TEXT_LIST_LEVEL_STYLE_NUMBER.equals(level[i].getName());
    }

    public boolean isBullet(int i) {
        return XMLString.TEXT_LIST_LEVEL_STYLE_BULLET.equals(level[i].getName());
    }

    public boolean isImage(int i) {
        return XMLString.TEXT_LIST_LEVEL_STYLE_IMAGE.equals(level[i].getName());
    }

    public String getLevelProperty(int i, String sName) {
        if (i>=1 && i<=MAX_LEVEL) {
            return level[i].getProperty(sName);
        }
        else {
            return null;
        }
    }

    public String getLevelStyleProperty(int i, String sName) {
        if (i>=1 && i<=MAX_LEVEL) {
            return levelStyle[i].getProperty(sName);
        }
        else {
            return null;
        }
    }

    public void loadStyleFromDOM(Node node) {
        super.loadStyleFromDOM(node);
        // Collect level information from child elements:
        if (node.hasChildNodes()){
            NodeList nl = node.getChildNodes();
            int nLen = nl.getLength();
            for (int i = 0; i < nLen; i++ ) {
                Node child=nl.item(i);
                if (child.getNodeType()==Node.ELEMENT_NODE){
                    String sLevel = Misc.getAttribute(child,XMLString.TEXT_LEVEL);
                    if (sLevel!=null) {
                        int nLevel = Misc.getPosInteger(sLevel,1);
                        if (nLevel>=1 && nLevel<=MAX_LEVEL) {
                            level[nLevel].loadFromDOM(child);
                            // Also include style:properties
                            if (child.hasChildNodes()){
                                NodeList nl2 = child.getChildNodes();
                                int nLen2 = nl2.getLength();
                                for (int i2 = 0; i2 < nLen2; i2++ ) {
                                    Node child2=nl2.item(i2);
                                    if (child2.getNodeType()==Node.ELEMENT_NODE){
                                        if (child2.getNodeName().equals(XMLString.STYLE_PROPERTIES)) {
                                            levelStyle[nLevel].loadFromDOM(child2);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}