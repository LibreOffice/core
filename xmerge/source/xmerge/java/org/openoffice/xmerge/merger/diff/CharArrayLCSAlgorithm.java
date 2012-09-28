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

import java.util.ArrayList;
import org.openoffice.xmerge.merger.Difference;

/**
 *  <p>This is an implementations of <code>DiffAlgorithm</code> interface
 *  which will difference char arrays.</p>
 *
 *  <p>It also use Longest Common Subsequence (LCS).  The algorithm is based
 *  on the book "Introduction to Algorithms" by Thomas H.Cormen,
 *  Charles E.Leiserson, and Ronald L.Riverst (MIT Press 1990) page 314.</p>
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

        ArrayList<Difference> diffResult = new ArrayList<Difference>();

        generateResult(diffTable, orgSeqlen, modSeqlen, diffResult);

        Difference[] diffArray = new Difference[0];

        // convert the vector to array, it has to do in here as
        // generateResult is called recursively
        if (diffResult.size() > 0) {
            diffArray = new Difference[diffResult.size()];
            diffResult.toArray(diffArray);
        }

        diffTable = null;
        diffResult = null;

        return diffArray;
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
                                int i, int j, ArrayList<Difference> diffVector) {

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

