/************************************************************************
 *
 *  StyleWithProperties.java
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
 *  Copyright: 2002-2004 by Henrik Just
 *
 *  All Rights Reserved.
 *
 *  Version 0.3.3g (2004-10-22)
 *
 */

package writer2latex.office;

import org.w3c.dom.Node;
import org.w3c.dom.NamedNodeMap;
import java.util.Hashtable;
import writer2latex.util.Misc;

/** <p> Class representing a style in OOo which contains a style:properties
  * element </p>
  */
public class StyleWithProperties extends OfficeStyle {
    private Hashtable properties = new Hashtable();

    public void loadStyleFromDOM(Node node) {
        super.loadStyleFromDOM(node);
        // read the properties of the style, if any
        if (node.hasChildNodes()){
            Node properties = Misc.getChildByTagName(node,XMLString.STYLE_PROPERTIES);
            if (properties!=null) {
                NamedNodeMap attrNodes = properties.getAttributes();
                if (attrNodes!=null) {
                    int nLen = attrNodes.getLength();
                    for (int i=0; i<nLen; i++){
                        Node attr = attrNodes.item(i);
                        setProperty(attr.getNodeName(),attr.getNodeValue());
                    }
                }
            }
        }
    }

    public void setProperty(String sProperty, String sValue){
        properties.put(sProperty,sValue);
    }

    public String getProperty(String sProperty, boolean bInherit){
        if (bInherit) {
            return getProperty(sProperty);
        }
        else if (properties.containsKey(sProperty)){
            String sValue = (String) properties.get(sProperty);
            return Misc.truncateLength(sValue);
        }
        // no value;
        return null;
    }

    public String getProperty(String sProperty){
        if (properties.containsKey(sProperty)){
            String sValue = (String) properties.get(sProperty);
            return Misc.truncateLength(sValue);
        }
        else if (sParentName!=null){ // inherit from parent
            StyleWithProperties parentStyle
                = (StyleWithProperties) family.getStyle(sParentName);
            if (parentStyle!=null) {
                return parentStyle.getProperty(sProperty);
            }
        }
        // no value;
        return null;
    }

    public String getAbsoluteProperty(String sProperty){
        if (properties.containsKey(sProperty)){
            String sValue=(String) properties.get(sProperty);
            if (sValue.endsWith("%")) {
                StyleWithProperties parentStyle
                    = (StyleWithProperties) family.getStyle(sParentName);
                if (parentStyle!=null) {
                    String sParentValue = parentStyle.getAbsoluteProperty(sProperty);
                    if (sParentValue!=null) { return Misc.multiply(sValue,sParentValue); }
                }
                else if (getFamily()!=null && getFamily().getDefaultStyle()!=null) {
                    StyleWithProperties style = (StyleWithProperties) getFamily().getDefaultStyle();
                    String sDefaultValue=(String) style.getProperty(sProperty);
                    if (sValue !=null) { return Misc.multiply(sValue,sDefaultValue); }
                }
            }
            else {
                return Misc.truncateLength(sValue);
            }
        }
        else if (sParentName!=null){
            StyleWithProperties parentStyle
                = (StyleWithProperties) family.getStyle(sParentName);
            if (parentStyle!=null) {
                return parentStyle.getAbsoluteProperty(sProperty);
            }
        }
        else if (getFamily()!=null && getFamily().getDefaultStyle()!=null) {
            StyleWithProperties style = (StyleWithProperties) getFamily().getDefaultStyle();
            String sValue=(String) style.getProperty(sProperty);
            if (sValue !=null) { return sValue; }
        }
        // no value!
        return null;
    }

}