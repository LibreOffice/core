/************************************************************************
 *
 *  ParStyle.java
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
 *  Version 0.2 (2002-12-28)
 *
 */

package writer2latex.office;

import org.w3c.dom.Node;
import writer2latex.util.Misc;

/** <p> Class representing a paragraph style in OOo Writer </p> */
public class ParStyle extends StyleWithProperties {
    private String sMasterPageName = null;

    public String getMasterPageName() {
        return sMasterPageName;
    }

    public void loadStyleFromDOM(Node node) {
        super.loadStyleFromDOM(node);
        sMasterPageName = Misc.getAttribute(node,XMLString.STYLE_MASTER_PAGE_NAME);
    }

}