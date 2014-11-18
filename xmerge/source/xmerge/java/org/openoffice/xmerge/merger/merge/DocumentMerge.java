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

import org.w3c.dom.Element;
import org.w3c.dom.Node;

import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.MergeException;
import org.openoffice.xmerge.merger.MergeAlgorithm;
import org.openoffice.xmerge.merger.NodeMergeAlgorithm;
import org.openoffice.xmerge.merger.Iterator;
import org.openoffice.xmerge.merger.Difference;
import org.openoffice.xmerge.util.XmlUtil;

/**
 * This is an implementation of the {@code MergeAlgorithm} interface.
 *
 * <p>This class will merge two {@code Document} classes.  It utilizes the
 * appropriate class which implements {@link
 * org.openoffice.xmerge.merger.NodeMergeAlgorithm NodeMergeAlgorithm} to
 * perform the merge.</p>
 */
public class DocumentMerge implements MergeAlgorithm {

    private final NodeMergeAlgorithm subDocumentMerge;

    /**  The capabilities of this converter. */
    protected ConverterCapabilities cc_;

    /**
     * Constructor.
     *
     * @param  cc     The {@code ConverterCapabilities}.
     * @param  merge  The {@code NodeMergeAlgorithm}.
     */
    public DocumentMerge(ConverterCapabilities cc, NodeMergeAlgorithm merge) {
        cc_ = cc;
        subDocumentMerge = merge;
    }

    public void applyDifference(Iterator orgSeq, Iterator modSeq,
                            Difference[] differences) throws MergeException {

        // a quick test whether the differences array is in ascending order
        int currentPosition = -1;
        boolean haveDeleteOperation = false;

        for (Difference difference : differences) {
            if (difference.getOrgPosition() > currentPosition) {
                currentPosition = difference.getOrgPosition();
                haveDeleteOperation = difference.getOperation() == Difference.DELETE;
            } else if (difference.getOrgPosition() == currentPosition) {
                if (difference.getOperation() == Difference.DELETE) {
                    haveDeleteOperation = true;
                } else if (difference.getOperation() == Difference.ADD &&
                            haveDeleteOperation) {
                    throw new MergeException(
                         "Differences array is not sorted. Delete before Add");
                }
            } else {
               throw new MergeException("Differences array need to be sorted.");
            }
        }

        // reset sequence counters
        orgSeq.start();
        int orgSeqCounter = 0;

        modSeq.start();
        int modSeqCounter = 0;

        // check for each diff unit in the diff array to apply the diff
        for (Difference currentDiff : differences) {

            int operation = currentDiff.getOperation();

            switch (operation) {

            case Difference.DELETE:
                // loop through the original sequence up to the expected
                // position. note that we use delta (see above comment)
                // also. we will just continue the counter without reset it.
                for (;
                     orgSeqCounter < currentDiff.getOrgPosition();
                     orgSeqCounter++, orgSeq.next()) {
                    // empty
                }

                // remove the Node. note that it will NOT affect the
                // iterator sequence as ParaNodeIterator is a static one.
                removeNode((Node)(orgSeq.currentElement()));

                break;

            // if it's an add operation, then get content from original seq
            case Difference.ADD:
                // loop through the modified sequence up to the expected
                // position to get the content. As we don't need to modify
                // the sequence. we don't need to use delta to do adjustment.
                for (;
                     modSeqCounter < currentDiff.getModPosition();
                     modSeqCounter++, modSeq.next()) {
                    // empty
                }

                for (;
                     orgSeqCounter < currentDiff.getOrgPosition();
                     orgSeqCounter++, orgSeq.next()) {
                    // empty
                }

                if (orgSeqCounter > orgSeq.elementCount() - 1) {
                    // append the element to the end of the original sequence
                    appendNode((Node)(orgSeq.currentElement()),
                               (Node)(modSeq.currentElement()));
                } else {
                    // insert the element BEFORE the current element
                    insertNode((Node)(orgSeq.currentElement()),
                               (Node)(modSeq.currentElement()));
                }

                break;

            case Difference.CHANGE:
                for (;
                     modSeqCounter < currentDiff.getModPosition();
                     modSeqCounter++, modSeq.next()) {
                    // empty
                }

                for (;
                     orgSeqCounter < currentDiff.getOrgPosition();
                     orgSeqCounter++, orgSeq.next()) {
                    // empty
                }

                if (subDocumentMerge == null) {
                    // use a simple replace if no row merge algorithm supply
                    replaceElement((Element)orgSeq.currentElement(),
                                   (Element)modSeq.currentElement());
                } else {
                    subDocumentMerge.merge((Element)orgSeq.currentElement(),
                                         (Element)modSeq.currentElement());

                }
                break;

            default:
                break;
            }
        }
    }

    /**
     * Removes the specified {@code Node}.
     *
     * @param  node  {@code Node} to remove.
     */
    protected void removeNode(Node node) {

        Node parent = node.getParentNode();
        parent.removeChild(node);
    }

    /**
     * Appends {@code Node} after the specified {@code Node}.
     *
     * @param  oldNode  {@code Node} to append after.
     * @param  newNode  {@code Node} to append.
     */
    private void appendNode(Node oldNode, Node newNode) {
        Node clonedNode = XmlUtil.deepClone(oldNode, newNode);
        Node parent = oldNode.getParentNode();
        parent.appendChild(clonedNode);
    }

    /**
     * Insert {@code Node} before the specified {@code Node}.
     *
     * @param  oldNode  {@code Node} to insert before.
     * @param  newNode  {@code Node} to insert.
     */
    private void insertNode(Node oldNode, Node newNode) {
        Node clonedNode = XmlUtil.deepClone(oldNode, newNode);
        Node parent = oldNode.getParentNode();
        parent.insertBefore(clonedNode, oldNode);
    }

    /**
     * Replace <code>Element</code>.
     *
     * @param  currElem  {@code Element} to be replaced.
     * @param  newElem   {@code Element} to replace.
     */
    private void replaceElement(Element currElem, Element newElem) {

        Node clonedNode = XmlUtil.deepClone(currElem, newElem);
        Node parent = currElem.getParentNode();
        parent.replaceChild(clonedNode, currElem);
    }
}