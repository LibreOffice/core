/************************************************************************
 *
 *  SectionStyle.java
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
 *  Version 0.2 (2002-10-24)
 *
 */

package writer2latex.office;

import org.w3c.dom.Node;
import writer2latex.util.Misc;

/** <p> Class representing a section style in OOo Writer </p> */
public class SectionStyle extends StyleWithProperties {
    // TODO: check the documentation, maybe there are no props to read...
    private int nColCount = 0;

    public int getColCount() { return nColCount; }

    public void loadStyleFromDOM(Node node) {
        // TODO: StyleWithProperties should have a protected method
        // loadStyleFromDOMinner - BETA style...
        super.loadStyleFromDOM(node);
        Node properties = Misc.getChildByTagName(node,XMLString.STYLE_PROPERTIES);
        if (properties!=null) {
            Node columns = Misc.getChildByTagName(properties,XMLString.STYLE_COLUMNS);
            if (columns!=null) {
                nColCount = Misc.getPosInteger(Misc.getAttribute(columns,
                            XMLString.FO_COLUMN_COUNT),1);
                // TODO: read individual columns
            }
        }
    }

}