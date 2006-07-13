/************************************************************************
 *
 *  PageMaster.java
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
 *  Version 0.2 (2003-01-16)
 *
 */

package writer2latex.office;

import org.w3c.dom.Node;
import writer2latex.util.Misc;

/** <p> Class representing a page master in OOo Writer </p> */
public class PageMaster extends SectionStyle {
    private String sPageUsage = null;

    private boolean bHasFootnoteSep = false;
    private PropertySet footnoteSep = new PropertySet();

    private boolean bHasHeaderStyle = false;
    private PropertySet headerStyle = new PropertySet();

    private boolean bHasFooterStyle = false;
    private PropertySet footerStyle = new PropertySet();


    public String getPageUsage() {
        return sPageUsage;
    }

    public boolean hasFootnoteSep() { return bHasFootnoteSep; }

    public String getFootnoteProperty(String sPropName) {
        return footnoteSep.getProperty(sPropName);
    }

    public boolean hasHeaderStyle() { return bHasHeaderStyle; }

    public String getHeaderProperty(String sPropName) {
        return headerStyle.getProperty(sPropName);
    }

    public boolean hasFooterStyle() { return bHasFooterStyle; }

    public String getFooterProperty(String sPropName) {
        return footerStyle.getProperty(sPropName);
    }

    public void loadStyleFromDOM(Node node) {
        super.loadStyleFromDOM(node);
        sPageUsage = Misc.getAttribute(node,XMLString.STYLE_PAGE_USAGE);

        Node hsNode = Misc.getChildByTagName(node,XMLString.STYLE_HEADER_STYLE);
        if (hsNode!=null) {
            bHasHeaderStyle = true;
            Node hsProperties = Misc.getChildByTagName(hsNode,XMLString.STYLE_PROPERTIES);
            if (hsProperties!=null) { headerStyle.loadFromDOM(hsProperties); }
        }

        Node fsNode = Misc.getChildByTagName(node,XMLString.STYLE_FOOTER_STYLE);
        if (fsNode!=null) {
            bHasFooterStyle = true;
            Node fsProperties = Misc.getChildByTagName(fsNode,XMLString.STYLE_PROPERTIES);
            if (fsProperties!=null) { footerStyle.loadFromDOM(fsProperties); }
        }

        Node properties = Misc.getChildByTagName(node,XMLString.STYLE_PROPERTIES);
        if (properties!=null) {
            Node fnsNode = Misc.getChildByTagName(properties,XMLString.STYLE_FOOTNOTE_SEP);
            if (fnsNode!=null) {
                bHasFootnoteSep = true;
                footnoteSep.loadFromDOM(fnsNode);
            }
        }
    }

}