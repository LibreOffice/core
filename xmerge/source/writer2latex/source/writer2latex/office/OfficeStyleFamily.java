/************************************************************************
 *
 *  OfficeStyleFamily.java
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
 *  Version 0.3.1 (2003-11-21)
 *
 */

package writer2latex.office;

import org.w3c.dom.Node;
import java.util.Hashtable;
import java.util.Enumeration;
import writer2latex.util.Misc;

/** <p> Class representing a style family in OOo </p> */
public class OfficeStyleFamily{
    private Hashtable styles = new Hashtable();
    private Class styleClass;

    private OfficeStyle defaultStyle = null;

    public OfficeStyleFamily(Class styleClass) {
        // styleClass must be a descendant of OfficeStyle
        this.styleClass = styleClass;
    }

    public void setDefaultStyle(OfficeStyle style) {
        defaultStyle = style;
    }

    public OfficeStyle getDefaultStyle() {
        return defaultStyle;
    }

    public OfficeStyle getStyle(String sName) {
        if (sName==null) { return null; }
        else { return (OfficeStyle) styles.get(sName); }
    }

    public Enumeration getStylesEnumeration(){
        return styles.elements();
    }

    public void loadStyleFromDOM(Node node, boolean bAutomatic) {
        String sName = Misc.getAttribute(node,XMLString.STYLE_NAME);
        if (sName!=null) {
            try {
                OfficeStyle style = (OfficeStyle) styleClass.newInstance();
                style.sName=sName;
                style.family=this;
                style.bAutomatic=bAutomatic;
                style.loadStyleFromDOM(node);
                styles.put(sName,style);
            }
            catch (InstantiationException e) {
                e.printStackTrace();
            }
            catch (IllegalAccessException e) {
                e.printStackTrace();
            }
        }
    }

}