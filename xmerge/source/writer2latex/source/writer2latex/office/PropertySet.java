/************************************************************************
 *
 *  PropertySet.java
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
 *  Copyright: 2002-2003 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.3 (2003-07-01)
 *
 */

package writer2latex.office;

import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import java.util.Hashtable;

/** <p> Class representing a set of style properties in OOo (actually this
    is simply the set of attributes of an element). </p>
  */
public class PropertySet {
    private Hashtable properties = new Hashtable();
    private String sName;

    public PropertySet() {
        properties = new Hashtable();
        sName="";
    }

    public String getProperty(String sPropName) {
        if (sPropName!=null) {
            String sValue = (String) properties.get(sPropName);
            if (sValue!=null && sValue.endsWith("inch")) {
                // Cut of inch to in
                return sValue.substring(0,sValue.length()-2);
            }
            else {
                return sValue;
            }
        }
        else {
            return null;
        }
    }

    public String getName() { return sName; }

    public void loadFromDOM(Node node) {
        sName = node.getNodeName();
        // read the attributes of the node, if any
        if (node!=null) {
            NamedNodeMap attrNodes = node.getAttributes();
            if (attrNodes!=null) {
                int nLen = attrNodes.getLength();
                for (int i=0; i<nLen; i++){
                    Node attr = attrNodes.item(i);
                    properties.put(attr.getNodeName(),attr.getNodeValue());
                }
            }
        }
    }

    public void setProperty(String sProperty, String sValue){
        properties.put(sProperty,sValue);
    }

}