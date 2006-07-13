/************************************************************************
 *
 *  TableLine.java
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

/**
 * <p> This class represents the properties of a row or column in a table</p>
 */
public class TableLine {
    private String sStyleName;
    private String sVisibility;
    private String sDefaultCellStyleName;
    private boolean bDisplay;
    private boolean bHeader;

    public TableLine(Node node, boolean bHeader, boolean bDisplay) {
        // Node must be table:table-column or table:table-row
        sStyleName = Misc.getAttribute(node,XMLString.TABLE_STYLE_NAME);
        sVisibility = Misc.getAttribute(node,XMLString.TABLE_VISIBILITY);
        if (sVisibility==null) { sVisibility = "visible"; }
        sDefaultCellStyleName = Misc.getAttribute(node,XMLString.TABLE_DEFAULT_CELL_STYLE_NAME);
        this.bDisplay = bDisplay;
        this.bHeader = bHeader;
    }

    public String getStyleName() { return sStyleName; }

    public String getVisibility() { return sVisibility; }

    public String getDefaultCellStyleName() { return sDefaultCellStyleName; }

    public boolean isDisplay() { return bDisplay; }

    public boolean isHeader() { return bHeader; }

}
