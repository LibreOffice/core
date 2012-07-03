/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package org.openoffice.xmerge.merger.diff;

import org.w3c.dom.Node;
import org.w3c.dom.Element;

import java.util.ArrayList;
import org.openoffice.xmerge.merger.DiffAlgorithm;
import org.openoffice.xmerge.merger.Difference;
import org.openoffice.xmerge.merger.Iterator;
import org.openoffice.xmerge.converter.xml.OfficeConstants;

/**
 *  <p>A very simple and direct difference algorithm for row
 *  <code>Node</code> objects in a spreadsheet.  Basically, it will
 *  compare objects in sequence and does not look ahead (unlike LCS).</p>
 *
 *  <p><ol><li>
 *    If two objects are the same, skip to next one.
 *  </li><li>
 *    Otherwise check whether the row repeated attribute is the same.
 *  </li><li>
 *    If the row repeated attribute is the same, then compare two rows
 *    and mark it as <i>change</i> if those rows are different.
 *  </li><li>
 *    If the row repeated attribute is different, then split the rows and
 *    continue to compare.
 *  </li><li>
 *    If there are more objects in the modseq than the original sequence,
 *    then all of the extra ones in the modified sequence are marked as add.
 *  </li><li>
 *    If there are more objects in the original sequence than the modified
 *    sequence, then all the extra one in the modified sequence are marked
 *    as delete.
 *  </li></ol></p>
 *
 *  <p>NOTE: The algorithm will have potential side effect to split rows.</p>
 *
 *  @author smak
 */

public class IteratorRowCompare implements DiffAlgorithm {

    /**
     *  Compute the differences of the given two sequences.
     *  Refer to the class description.
     *
     *  Return an array of <code>Difference</code> objects.  This method finds
     *  out the difference between two sequences.
     *
     *  @param  orgSeq  The original sequence.
     *  @param  modSeq  The modified (or changed) sequence to
     *                 compare against with the origial.
     *
     *  @return  An array of Difference objects.
     */
    public Difference[] computeDiffs(Iterator orgSeq, Iterator modSeq) {

        ArrayList<Difference> diffVector = new ArrayList<Difference>();

        // i and j are counters to keep track the current position in the
        // iterator
        int i = 0;
        int j = 0;
        Object orgSeqObject = orgSeq.start();
        Object modSeqObject = modSeq.start();
        Element orgRow, modRow;
        boolean different = false;
        boolean orgSplited = false;
        boolean modSplited = false;

        while (orgSeqObject != null) {

            different = true;

            if (modSeqObject ==  null)  {
                // no more modsequence, all the remaining org sequence objs
                // should consider as a delete.
                Difference diff = new Difference(Difference.DELETE, i, j);
                diffVector.add(diff);
                orgSeqObject = orgSeq.next();

            } else {
                if (!orgSeq.equivalent(orgSeqObject, modSeqObject)) {

                    orgRow = (Element)orgSeqObject;
                    modRow = (Element)modSeqObject;

                    // check whether the original Row with multiple row
                    // if so, need to split one out for merge
                    String orgRowRepeated = orgRow.getAttribute(
                        OfficeConstants.ATTRIBUTE_TABLE_NUM_ROWS_REPEATED);
                    String modRowRepeated = modRow.getAttribute(
                        OfficeConstants.ATTRIBUTE_TABLE_NUM_ROWS_REPEATED);


                    int orgRowNum = 1;
                    int modRowNum = 1;

                    if (orgRowRepeated.length() > 0) {
                        orgRowNum =
                            Integer.valueOf(orgRowRepeated).intValue();
                    }
                    if (modRowRepeated.length() > 0) {
                        modRowNum =
                            Integer.valueOf(modRowRepeated).intValue();
                    }

                    // try to find out the common number of repeated Rows
                    if (orgRowNum == modRowNum) {
                        orgSeqObject = orgSeq.next();
                        modSeqObject = modSeq.next();

                    // cut the original row into two halves, first half
                    // have the repeated attribute = modify row attr
                    } else if (orgRowNum > modRowNum) {
                        Element orgSplitRow = splitRepeatedRow(
                                    orgRow, modRowNum,
                                    orgRowNum - modRowNum);
                        // it may equal after the split!
                        if (orgSeq.equivalent(orgSplitRow, modRow)) {
                            different = false;
                        }
                        orgSplited = true;
                        modSeqObject = modSeq.next();

                    // cut the modified Row into two halves, first half
                    // have the repeated attribute = original Row attr
                    } else {
                        Element modSplitRow = splitRepeatedRow(
                                    modRow, orgRowNum,
                                    modRowNum - orgRowNum);

                        // check whether rows are equal after the split
                        if (modSeq.equivalent(orgRow, modSplitRow)) {
                            different = false;
                        }
                        modSplited = true;
                        orgSeqObject = orgSeq.next();
                    }

                    if (different) {
                        Difference diff = new Difference(Difference.CHANGE,
                                          i, j);
                        diffVector.add(diff);
                    }

                } else {
                    // Rows are equivalent, move on to next one.
                    orgSeqObject = orgSeq.next();
                    modSeqObject = modSeq.next();
                } // end if-else
                j++;
            } // end if-else
            i++;
        } // end while loop

        // any extra objects in modify sequence should consider as an add
        // to the original sequence
        for (; modSeqObject != null; modSeqObject = modSeq.next(), j++) {
            Difference diff = new Difference(Difference.ADD, i, j);
            diffVector.add(diff);
        }

        // need to refresh the iterator if we split the rows
        if (orgSplited) {
            orgSeq.refresh();
        }

        if (modSplited) {
            modSeq.refresh();
        }


        // convert the vector to array
        Difference[] diffArray = new Difference[diffVector.size()];
        diffVector.toArray(diffArray);

        return diffArray;
    }


    private Element splitRepeatedRow(Element orgRow, int splitNum, int orgNum) {
        // NOTE: should we really want to do deep clone?
        // in most the case, it is an empty Row, but the
        // specification didn't forbid any node to use multiple
        // column attributes. i.e. the node can contain text
        // nodes or other things under it.
        Element splitRow = (Element)(orgRow.cloneNode(true));

        if (splitNum > 1) {
            splitRow.setAttribute(
              OfficeConstants.ATTRIBUTE_TABLE_NUM_ROWS_REPEATED,
              String.valueOf(splitNum));
        } else if (splitNum == 1) {
            splitRow.removeAttribute(
              OfficeConstants.ATTRIBUTE_TABLE_NUM_ROWS_REPEATED);
        }
        if (orgNum > 1) {
            orgRow.setAttribute(
              OfficeConstants.ATTRIBUTE_TABLE_NUM_ROWS_REPEATED,
              String.valueOf(orgNum));
        } else if (orgNum == 1) {
            orgRow.removeAttribute(
              OfficeConstants.ATTRIBUTE_TABLE_NUM_ROWS_REPEATED);
        }

        Node parentNode = orgRow.getParentNode();
        parentNode.insertBefore(splitRow, orgRow);

        return splitRow;
    }
}

