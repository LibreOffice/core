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

import java.util.Vector;
import org.openoffice.xmerge.merger.DiffAlgorithm;
import org.openoffice.xmerge.merger.Difference;

/**
 *  <p>This is an implementations of <code>DiffAlgorithm</code> interface
 *  which will difference char arrays.</p>
 *
 *  <p>It also use Longest Common Subsequence (LCS).  The algorithm is based
 *  on the book "Introduction to Algorithms" by Thomas H.Cormen,
 *  Charles E.Leiserson, and Ronald L.Riverst (MIT Press 1990) page 314.</p>
 *
 *  @author smak
 */
public class CharArrayLCSAlgorithm {

    /**
     *  Return an <code>Difference</code> array.  This method finds out
     *  the difference between two sequences.
     *
     *  @param  orgSeq  The original sequence.
     *  @param  modSeq  The modified (or changed) sequence to
     *                  compare against the origial.
     *
     *  @return  A <code>Difference</code> array.
     */
    public Difference[] computeDiffs(char[] orgSeq, char[] modSeq) {

        int orgSeqlen = orgSeq.length;
        int modSeqlen = modSeq.length;

        int[][] diffTable;

        // Diff table is used to keep track which element is the same or not
        // in those 2 sequences
        diffTable = createDiffTable(orgSeq, modSeq);

        // debug purpose...
        // printDiffTable(diffTable);

        Vector diffResult = new Vector();

        generateResult(diffTable, orgSeqlen, modSeqlen, diffResult);

        // don't need anymore if Difference do not contain content information
        /* fillInDiffContent(diffResult, orgSeq, modSeq); */

        Difference[] diffArray = new Difference[0];

        // convert the vector to array, it has to do in here as
        // generateResult is called recursively
        if (diffResult.size() > 0) {
            diffArray = new Difference[diffResult.size()];
            diffResult.copyInto(diffArray);
        }

        diffTable = null;
        diffResult = null;

        return diffArray;
    }


    /**
     *  Debug function Used to print out the nicely formatted
     *  difference table.
     *
     *  @param  diffTable  The difference table to display.
     */
    private void printDiffTable(int[][] diffTable) {

        for (int i = 0; i < diffTable.length; i++) {
            for (int j = 0; j < diffTable[i].length; j++) {
                System.out.print(" " + diffTable[i][j] +  " ");
            }
            System.out.println();
        }
    }


    /**
     *  Create the difference table.
     *  The difference table is used internal to keep track what
     *  elements are common or different in the two sequences.
     *
     *  @param  orgSeq  The original sequence to be used as a base.
     *  @param  modSeq  The modified sequence to compare.
     *
     *  @return  A difference table as a two-dimensional array of
     *           integers.
     */
    private int[][] createDiffTable(char[] orgSeq, char[] modSeq) {
        int orgSeqlen = orgSeq.length + 1;
        int modSeqlen = modSeq.length + 1;
        int[][] diffTable;

        // initialize the diffTable (it need to be 1 row/col bigger
        // than the original str)
        diffTable = new int[orgSeqlen][];
        for (int i = 0; i < orgSeqlen; i++) {
            diffTable[i] = new int[modSeqlen];
        }

        // compute the diff Table using LCS algorithm, refer to the book
        // mentioned at the top of the program
        for (int i = 1; i < orgSeqlen; i++) {
            for (int j = 1; j < modSeqlen; j++) {

                if (orgSeq[i-1] == modSeq[j-1]) {
                    diffTable[i][j] = diffTable[i-1][j-1]+1;
                } else {
                    if (diffTable[i-1][j] >= diffTable[i][j-1]) {
                        diffTable[i][j] = diffTable[i-1][j];
                    } else {
                        diffTable[i][j] = diffTable[i][j-1];
                    }
                }
            }
        }

        return diffTable;
    }


    /**
     *  Generate the <code>Difference</code> result vector.
     *  This method will be called recursively to backtrack the difference
     *  table to get the difference result (and also the LCS).
     *
     *  @param   diffTable   The difference table containing the
     *                       <code>Difference</code> result.
     *  @param   i           The nth element in original sequence to
     *                       compare.  This method is called recursively
     *                       with i and j decreased until 0.
     *  @param   j           The nth element in modified sequence to
     *                       compare.
     *  @param   diffVector  A vector to output the <code>Difference</code>
     *                       result. Can not use a return variable as it
     *                       is a recursive method.  The vector will contain
     *                       <code>Difference</code> objects with operation
     *                       and positions filled in.
     */
    private void generateResult(int[][] diffTable,
                                int i, int j, Vector diffVector) {

        // handle the first element
        if (i == 0 || j == 0) {
            if (i == 0 && j == 0) {
                // return
            } else if (j == 0) {
                for (int cnt = 0; cnt < i; cnt++) {
                    Difference diff =
                        new Difference(Difference.DELETE, cnt, j);
                    diffVector.add(diff);
                }
            } else {
                for (int cnt = 0; cnt < j; cnt++) {
                    Difference diff =
                        new Difference(Difference.ADD, i, cnt);
                    diffVector.add(diff);
                }
            }
            return;
        }

        // for the detail of this algorithm, refer to the book mentioned on
        // the top and page 317 and 318.
        if ((diffTable[i-1][j-1] == diffTable[i][j] -1) &&
            (diffTable[i-1][j-1] == diffTable[i-1][j]) &&
            (diffTable[i-1][j-1] == diffTable[i][j-1])) {

            // the element of ith and jth in org and mod sequence is the same
            generateResult(diffTable, i-1, j-1, diffVector);
        } else {
            if (diffTable[i-1][j] > diffTable[i][j-1]) {

                // recursively call first, then add the result so that
                // the beginning of the diffs will be stored first
                generateResult(diffTable, i-1, j, diffVector);

                Difference diff =
                    new Difference(Difference.DELETE, i-1, j);
                diffVector.add(diff);
            } else if (diffTable[i-1][j] < diffTable[i][j-1]) {

                // recursively call first, then add the result so that
                // the beginning of the diffs will be stored first
                generateResult(diffTable, i, j-1, diffVector);

                Difference diff =
                    new Difference(Difference.ADD, i, j-1);
                diffVector.add(diff);
            } else { // diffTable[i-1][j] == diffTable[i][j-1]
                // recursively call first, then add the result so that
                // the beginning of the diffs will be stored first
                generateResult(diffTable, i-1, j-1, diffVector);

                Difference diff =
                    new Difference(Difference.CHANGE, i-1, j-1);
                diffVector.add(diff);

            }
        }
    }
}

