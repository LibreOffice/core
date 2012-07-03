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

package org.openoffice.xmerge.merger.merge;

import java.util.List;
import org.w3c.dom.Node;
import org.openoffice.xmerge.merger.Difference;
import org.openoffice.xmerge.merger.NodeMergeAlgorithm;
import org.openoffice.xmerge.merger.diff.CharacterParser;
import org.openoffice.xmerge.merger.diff.CharArrayLCSAlgorithm;
import org.openoffice.xmerge.merger.diff.TextNodeEntry;
import org.openoffice.xmerge.util.Debug;

/**
 *  This is an implementation of the <code>NodeMergeAlgorithm</code>
 *  interface.  It is used to merge two paragraph <code>Node</code>
 *  objects based on character comparisons.
 *
 * @author smak
 */
public final class CharacterBaseParagraphMerge
                   implements NodeMergeAlgorithm {



    /**
     *  Merge two paragraph <code>Node</code> by using Longest Common
     *  Subsequence (LCS) character algorithm defined in {@link
     *  org.openoffice.xmerge.merger.diff.CharArrayLCSAlgorithm
     *  CharArrayLCSAlgorithm}
     *
     *  @param  orgPara  The original paragraph <code>Node</code>.
     *  @param  modPara  The modified paragraph <code>Node</code>.
     */
    public void merge(Node orgPara, Node modPara) {
        CharacterParser orgParser = new CharacterParser(orgPara);
        CharacterParser modParser = new CharacterParser(modPara);

        char[] orgCharArray = orgParser.getCharArray();
        char[] modCharArray = modParser.getCharArray();

        CharArrayLCSAlgorithm diffAlgo = new CharArrayLCSAlgorithm();

        Difference[] diffResult = diffAlgo.computeDiffs(orgCharArray,
                                                        modCharArray);
        // debug use
        System.out.println("Diff Result: ");
        for (int i = 0; i < diffResult.length; i++) {
            Debug.log(Debug.INFO, diffResult[i].debug());
        }

        applyDifference(orgParser, modParser, diffResult);
    }


    private void applyDifference(CharacterParser orgParser,
                                 CharacterParser modParser,
                                 Difference[] diffs) {

        List<TextNodeEntry> orgNodeList = orgParser.getNodeList();
        int diffCount = 0;
        int numNode = orgNodeList.size();

        for (int i = 0; i < numNode; i++) {

            int extraChar = 0;
            int orgDiffCount = diffCount;
            TextNodeEntry orgTextNode = (orgNodeList.get(i));

            Debug.log(Debug.INFO, "checking node " + (i + 1) + " of " + numNode);

            // check any difference in this node and estimate the new char num
            for (; diffCount < diffs.length; diffCount++) {

                Debug.log(Debug.INFO, "  checking diff " + (diffCount + 1) +
                  " of " + diffs.length);
                Debug.log(Debug.INFO, "    OrgPosision <" +
                  diffs[diffCount].getOrgPosition() + "> diffCount <" +
                  diffCount + "> orgDiffCount <" + orgDiffCount + ">");

                // don't need to check and diffs beyond the current node text
                // range except the last node
                if (diffs[diffCount].getOrgPosition() > orgTextNode.endChar() &&
                    i < numNode - 1) {
                    Debug.log(Debug.INFO, "    breaking!");
                    break;
                }

                if (diffs[diffCount].getOrgPosition()
                                                  >= orgTextNode.startChar()) {
                    if (diffs[diffCount].getOperation() == Difference.DELETE) {
                        extraChar--;
                    } else if (diffs[diffCount].getOperation()
                                                        == Difference.ADD) {
                        extraChar++;
                    }

                }
            }

            Debug.log(Debug.INFO, "  final diffCount <" + diffCount +
              "> final orgDiffCount <" + orgDiffCount + ">");

            // will only try to merge if there is a difference in this node
            if (diffCount > orgDiffCount) {

                Debug.log(Debug.INFO, "  There is a difference, doing merge");
                Debug.log(Debug.INFO, "  TextNode name <" +
                  orgTextNode.node().getNodeName() + ">");
                Debug.log(Debug.INFO, "  TextNode value <" +
                  orgTextNode.node().getNodeValue() + ">");
                Debug.log(Debug.INFO, "  TextNode start char <" +
                  orgTextNode.startChar() + "> TextNode end char <" +
                  orgTextNode.endChar() + ">");
                Debug.log(Debug.INFO, "  extraChar value <" + extraChar + ">");

                coreMerge(orgDiffCount, diffCount, diffs, orgParser,
                          modParser, orgTextNode, extraChar);
            }
        }
    }

    private void coreMerge(int startDiffNum, int endDiffNum, Difference[] diffs,
                        CharacterParser orgParser, CharacterParser modParser,
                        TextNodeEntry orgTextNode, int extraChar) {

        Node orgNode = orgTextNode.node();
        char[] modTextArray = modParser.getCharArray();
        String tmpString;

        // Handle situation where getNodeValue returns null
        //
        if (orgNode.getNodeValue() != null)
           tmpString = orgNode.getNodeValue();
        else
           tmpString = "";

        char[] orgNodeText = tmpString.toCharArray();
        char[] newNodeText;

        if (orgNodeText.length + extraChar > 0)
           newNodeText = new char[orgNodeText.length + extraChar];
        else
           newNodeText = new char[0];

        int orgTextPosition = orgTextNode.startChar();   // used for block copy
        int newTextPosition = 0;                         // used for block copy
        int unChangedTextLength = 0;

        char[] cacheCharArray = new char[endDiffNum - startDiffNum];
        int cacheLength = 0;
        int lastDiffOperation = Difference.UNCHANGE;
        int lastDiffPosition = -1;

        // starting to diff
        //
        for (int j = startDiffNum; j < endDiffNum; j++) {

            // copy any contents before the diff
            //
            if (diffs[j].getOrgPosition() > orgTextPosition) {
                // need to flush first
                if (cacheLength > 0) {
                    System.arraycopy(cacheCharArray, 0,
                                     newNodeText, newTextPosition, cacheLength);
                    newTextPosition += cacheLength;

                    // reset the markers
                    lastDiffPosition = -1;
                    lastDiffOperation = Difference.UNCHANGE;
                    cacheLength = 0;
                }

                // find out the length how many characters are
                // untouched by the diff
                unChangedTextLength = diffs[j].getOrgPosition() -
                                      orgTextPosition;
                System.arraycopy(orgNodeText,
                                 orgTextPosition - orgTextNode.startChar(),
                                 newNodeText, newTextPosition,
                                 unChangedTextLength);
                orgTextPosition += unChangedTextLength;
                newTextPosition += unChangedTextLength;
            }

            // for any deleted characters, just skip without copy
            // but still need to take care the cached characters
            //
            if (diffs[j].getOperation() == Difference.DELETE) {
                orgTextPosition++;

                // flush out the cache and copy the content to new Text
                if (cacheLength > 0) {
                    System.arraycopy(cacheCharArray, 0,
                                     newNodeText, newTextPosition, cacheLength);
                    newTextPosition += cacheLength;

                    // reset the markers
                    lastDiffPosition = -1;
                    lastDiffOperation = Difference.UNCHANGE;
                    cacheLength = 0;
                }

                continue;


            // check whether we should flush the cache.
            // For changed diffs, only continuous changes can be cached
            // For Add diffs, only same insertion point can be cached
            // and for both changed/add diffs, need to have same operation
            // as last cached diffs.

            } else {
                if (lastDiffOperation != diffs[j].getOperation()  ||
                    (diffs[j].getOperation() == Difference.CHANGE &&
                     diffs[j].getOrgPosition() != lastDiffPosition + 1) ||
                    (diffs[j].getOperation() == Difference.ADD &&
                     diffs[j].getOrgPosition() != lastDiffPosition)) {

                    // flush the cache
                    if (cacheLength > 0) {
                        System.arraycopy(cacheCharArray, 0, newNodeText,
                                         newTextPosition, cacheLength);
                        newTextPosition += cacheLength;

                        // reset the markers
                        lastDiffPosition = -1;
                        lastDiffOperation = Difference.UNCHANGE;
                        cacheLength = 0;
                    }
                }

                // add the diffs to the cache, now the diffs will be either
                // a new 'changed' char or is an adjacent following change of
                // last difference
                cacheCharArray[cacheLength] =
                                    modTextArray[diffs[j].getModPosition()];
                cacheLength++;
                lastDiffOperation = diffs[j].getOperation();
                lastDiffPosition = diffs[j].getOrgPosition();

                // need to increment the original text position
                // after we cached it
                if (lastDiffOperation == Difference.CHANGE) {
                    orgTextPosition++;
                }
            }
        }

        // flush any contents remaining in the cache
        if (cacheLength > 0) {
            System.arraycopy(cacheCharArray, 0, newNodeText,
                             newTextPosition, cacheLength);
            newTextPosition += cacheLength;
            // no need to reset any cache-related info as this is a last flush
        }

        // copy any contents after all the diffs
        int orgStartPosition = orgTextNode.startChar();
        if (orgNodeText.length + orgStartPosition > orgTextPosition) {
            unChangedTextLength = orgNodeText.length + orgStartPosition
                                  - orgTextPosition;
            System.arraycopy(orgNodeText, orgTextPosition - orgStartPosition,
                             newNodeText, newTextPosition,
                             unChangedTextLength);
        }

        // set the text to the original node if there are any diffs processed.
        // can't use newNodeText.length to check as even it is empty, we may
        // process a whole bunch of deletion already (i.e. the whole
        // orgNodeText deleted).
        if (endDiffNum > startDiffNum) {
            String newString = new String(newNodeText);
            orgNode.setNodeValue(newString);
        }
    }
}

