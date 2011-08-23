/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package org.openoffice.xmerge.merger.merge;

import org.w3c.dom.Node;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.w3c.dom.NamedNodeMap;

import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.merger.Iterator;
import org.openoffice.xmerge.merger.NodeMergeAlgorithm;
import org.openoffice.xmerge.merger.diff.CellNodeIterator;
import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.util.XmlUtil;


/**
 *  This is an implementation of the <code>NodeMergeAlgorithm</code>
 *  interface.  It is used to merge two rows using a positional
 *  comparison base method.
 */
public final class PositionBaseRowMerge implements NodeMergeAlgorithm {

    /**  The capabilities of this converter. */
    private ConverterCapabilities cc_;


    /**
     *  Constructor.
     * 
     *  @param  cc  The <code>ConverterCapabilities</code>.
     */
    public PositionBaseRowMerge(ConverterCapabilities cc) {
        cc_ = cc;
    }


    public void merge(Node orgRow, Node modRow) {

        Iterator orgCells = new CellNodeIterator(cc_, orgRow);
        Iterator modCells = new CellNodeIterator(cc_, modRow);

        mergeCellSequences(orgCells, modCells);
    }


    // used to compare the cell 1 by 1
    private void mergeCellSequences(Iterator orgSeq, Iterator modSeq) {

        boolean needMerge = true;
        Element orgCell, modCell;

        Object orgSeqObject = orgSeq.start();
        Object modSeqObject = modSeq.start();

        while (orgSeqObject != null) {


            needMerge = true;

            if (modSeqObject ==  null)  {
                // no corresponding cell in the target, empty out the cell
                SheetUtil.emptyCell(cc_, (Node)orgSeqObject);
                orgSeqObject = orgSeq.next();

            } else {

                // compare the cell directly
                if (!orgSeq.equivalent(orgSeqObject, modSeqObject)) {

                    orgCell = (Element)orgSeqObject;
                    modCell = (Element)modSeqObject;

                    // check whether the original cell with multiple column
                    // if so, need to split one out for merge
                    String orgColRepeated = orgCell.getAttribute(
                        OfficeConstants.ATTRIBUTE_TABLE_NUM_COLUMNS_REPEATED);
                    String modColRepeated = modCell.getAttribute(
                        OfficeConstants.ATTRIBUTE_TABLE_NUM_COLUMNS_REPEATED);

                    int orgColNum = 1;
                    int modColNum = 1;

                    if (orgColRepeated.length() > 0) {
                        orgColNum =
                            Integer.valueOf(orgColRepeated).intValue();
                    }
                    if (modColRepeated.length() > 0) {
                        modColNum =
                            Integer.valueOf(modColRepeated).intValue();
                    }

                    // try to find out the common number of repeated cols
                    if (orgColNum == modColNum) {
                        orgSeqObject = orgSeq.next();
                        modSeqObject = modSeq.next();

                    // cut the original cell into 2 half, first half
                    // have the repeated attribute = modify cell attr
                    } else if (orgColNum > modColNum) {
                        Element orgSplitCell = splitColRepeatedCell(
                                    orgCell, modColNum,
                                    orgColNum - modColNum);
                        // it may equal after the split!
                        if (orgSeq.equivalent(orgSplitCell, modCell)) {
                            needMerge = false;
                        }
                        orgCell = orgSplitCell;
                        modSeqObject = modSeq.next();

                    // cut the modified cell into 2 half, first half
                    // have the repeated attribute = original cell attr
                    } else {
                        Element modSplitCell = splitColRepeatedCell(
                                    modCell, orgColNum,
                                    modColNum - orgColNum);
                        // it may equal after the split!
                        if (modSeq.equivalent(orgCell, modSplitCell)) {
                            needMerge = false;
                        }
                        modCell = modSplitCell;
                        orgSeqObject = orgSeq.next();
                    }

                    if (needMerge) {
                        mergeCells(orgCell, modCell);
                    }

                } else {
                    // cells are equivalent, move on to next one.
                    orgSeqObject = orgSeq.next();
                    modSeqObject = modSeq.next();
                } // end if-else
            } // end if-else
        } // end while loop

        // get the one of the original cell, so that the cloned node
        // can base it to find the document node
        orgCell = (Element)orgSeq.start();

        // add any extra cells to the original cell sequence.
        for (; modSeqObject != null; modSeqObject = modSeq.next()) {
            Node clonedNode = XmlUtil.deepClone(orgCell, (Node)modSeqObject);
            Node parent = orgCell.getParentNode();
            parent.appendChild(clonedNode);
        }
    }


    private Element splitColRepeatedCell(Element orgCell,
                                         int splitNum, int orgNum) {
        // NOTE: should we really want to do deep clone?
        // in most the case, it is an empty cell, but the
        // specification didn't forbid any node to use multiple
        // column attributes. i.e. the node can contain text
        // nodes or other things under it.
        Element splitCell = (Element)(orgCell.cloneNode(true));

        if (splitNum > 1) {
            splitCell.setAttribute(
              OfficeConstants.ATTRIBUTE_TABLE_NUM_COLUMNS_REPEATED,
              String.valueOf(splitNum));
        } else if (splitNum == 1) {
            splitCell.removeAttribute(
              OfficeConstants.ATTRIBUTE_TABLE_NUM_COLUMNS_REPEATED);
        }
        if (orgNum > 1) {
            orgCell.setAttribute(
              OfficeConstants.ATTRIBUTE_TABLE_NUM_COLUMNS_REPEATED,
              String.valueOf(orgNum));
        } else if (orgNum == 1) {
            orgCell.removeAttribute(
              OfficeConstants.ATTRIBUTE_TABLE_NUM_COLUMNS_REPEATED);
        }

        Node parentNode = orgCell.getParentNode();
        parentNode.insertBefore(splitCell, orgCell);

        return splitCell;
    }


    private void mergeCells(Element orgCell, Element modCell) {

        // remove all the supported attributes and possible text child for
        // string cells
        SheetUtil.emptyCell(cc_, orgCell);

        // copy all the supported attributes and possible text child from
        // the modified cell
        NamedNodeMap attrNodes = modCell.getAttributes();

        if (attrNodes != null) {

            // copy the first text:p node. As it's not necessary only string
            // type cell can have a text:p section. 
            NodeList paraNodes =
                modCell.getElementsByTagName(OfficeConstants.TAG_PARAGRAPH);

            Node firstParaNode = paraNodes.item(0);

            // try to clone the node
            if (firstParaNode != null) {

                Node clonedNode = XmlUtil.deepClone(orgCell, firstParaNode);

                // insert as the first child of the original cell
                Node firstChild = orgCell.getFirstChild();
                if (firstChild != null) {
                    orgCell.insertBefore(clonedNode, firstChild);
                } else {
                    orgCell.appendChild(clonedNode);
                }
            }

            // check all the attributes and copy those we supported in
            // converter
            // NOTE: for attribute list, refer to section 4.7.2 in specification
            int len = attrNodes.getLength();

            for (int i = 0; i < len; i++) {
                Node attr = attrNodes.item(i);

                // copy the supported attrs
                if (cc_.canConvertAttribute(OfficeConstants.TAG_TABLE_CELL,
                                           attr.getNodeName())) {
                    orgCell.setAttribute(attr.getNodeName(),
                                         attr.getNodeValue());
                }
            }
        }
    }
}

