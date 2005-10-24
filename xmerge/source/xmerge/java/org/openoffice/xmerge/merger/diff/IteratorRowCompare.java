/************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
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
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package org.openoffice.xmerge.merger.diff;

import org.w3c.dom.Node;
import org.w3c.dom.Element;

import java.util.Vector;
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

        int orgSeqlen = orgSeq.elementCount();
        int modSeqlen = modSeq.elementCount();

        Vector diffVector = new Vector();

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
        diffVector.copyInto(diffArray);

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

