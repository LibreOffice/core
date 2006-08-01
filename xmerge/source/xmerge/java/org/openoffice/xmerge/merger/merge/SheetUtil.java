/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SheetUtil.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:38:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package org.openoffice.xmerge.merger.merge;

import org.w3c.dom.Node;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.w3c.dom.NamedNodeMap;

import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.merger.diff.CellNodeIterator;
import org.openoffice.xmerge.converter.xml.OfficeConstants;


/**
 *  Utility methods to handle sheet XML tree.
 */
public class SheetUtil {

    /**
     *  <p>Empty the content of a cell value.   This includes the following:
     *  </p>
     *
     *  <p><ul><li>
     *    Remove all of the supported attributes.
     *  </li><li>
     *    Remove the first <i>text:p</i> <code>Node</code> for most of the cells.
     *  </li></ul></p>
     *
     *  @param  cc    The <code>ConverterCapabilities</code>.
     *  @param  node  The <code>Node</code>.
     */
    public static void emptyCell(ConverterCapabilities cc, Node node) {

        NamedNodeMap attrNodes = node.getAttributes();

        if (attrNodes != null) {

            // empty the first text:p node.
            // Note: it's not necessary only string type cell contain text:p
            // basically, all different type of cell will contain one
            Element cell = (Element)node;

            // get the paragraph node list
            NodeList paraNodes =
                cell.getElementsByTagName(OfficeConstants.TAG_PARAGRAPH);

            Node firstParaNode = paraNodes.item(0);

            // remove the first paragraph element node
            if (firstParaNode != null) {
                Node parent = firstParaNode.getParentNode();
                parent.removeChild(firstParaNode);
            }

            // check all the attributes and remove those we supported in
            // converter
            // NOTE: for attribute list, refer to section 4.7.2 in specification
            int len = attrNodes.getLength();

            for (int i = 0; i < len; ) {
                Node attr = attrNodes.item(i);

                // when we hit the end of the attribute nodes, return
                // it may happen sooner as we keep on removing nodes
                if (attr == null) {
                    break;
                }
                // remove the supported attr except columns repeated attribute
                if (cc.canConvertAttribute(OfficeConstants.TAG_TABLE_CELL,
                                           attr.getNodeName()) &&
                    !attr.getNodeName().equals(
                        OfficeConstants.ATTRIBUTE_TABLE_NUM_COLUMNS_REPEATED)) {

                    attrNodes.removeNamedItem(attr.getNodeName());
                } else {
                    i++;
                }
            }
        }
    }
}

