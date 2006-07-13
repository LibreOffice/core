/************************************************************************
 *
 *  OfficeStyle.java
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
import writer2latex.util.Misc;

/** <p> Abstract class representing a style in OOo </p> */
public abstract class OfficeStyle {
    protected String sName;
    protected OfficeStyleFamily family;
    protected String sParentName;
    protected boolean bAutomatic;

    public String getName() { return sName; }

    public String getParentName() { return sParentName; }

    public OfficeStyleFamily getFamily() { return family; }

    public boolean isAutomatic() { return bAutomatic; }

    public void loadStyleFromDOM(Node node){
        sParentName = Misc.getAttribute(node,XMLString.STYLE_PARENT_STYLE_NAME);
    }

}