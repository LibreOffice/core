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

package org.openoffice.xmerge.converter.xml.sxw.pocketword;

import org.w3c.dom.Document;

import org.openoffice.xmerge.DocumentMerger;
import org.openoffice.xmerge.MergeException;
import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.converter.xml.sxw.SxwDocument;
import org.openoffice.xmerge.merger.DiffAlgorithm;
import org.openoffice.xmerge.merger.Difference;
import org.openoffice.xmerge.merger.NodeMergeAlgorithm;
import org.openoffice.xmerge.merger.Iterator;
import org.openoffice.xmerge.merger.diff.ParaNodeIterator;
import org.openoffice.xmerge.merger.diff.IteratorLCSAlgorithm;
import org.openoffice.xmerge.merger.merge.DocumentMerge;
import org.openoffice.xmerge.merger.merge.CharacterBaseParagraphMerge;
import org.openoffice.xmerge.util.Debug;


/**
 *  PocketWord implementation of <code>DocumentMerger</code>
 *  for the {@link
 *  org.openoffice.xmerge.converter.xml.sxw.pocketword.PluginFactoryImpl
 *  PluginFactoryImpl}.</p>
 */
public class DocumentMergerImpl implements DocumentMerger {

    private ConverterCapabilities cc_;
    private org.openoffice.xmerge.Document orig = null;

    public DocumentMergerImpl(org.openoffice.xmerge.Document doc, ConverterCapabilities cc) {
        cc_ = cc;
        this.orig = doc;
    }

    public void merge(org.openoffice.xmerge.Document modifiedDoc) throws MergeException {

        SxwDocument wdoc1 = (SxwDocument) orig;
        SxwDocument wdoc2 = (SxwDocument) modifiedDoc;

        Document doc1 = wdoc1.getContentDOM();
        Document doc2 = wdoc2.getContentDOM();

        Iterator i1 = new ParaNodeIterator(cc_, doc1.getDocumentElement());
        Iterator i2 = new ParaNodeIterator(cc_, doc2.getDocumentElement());

        DiffAlgorithm diffAlgo = new IteratorLCSAlgorithm();

        // find out the paragrah level diffs
        Difference[] diffTable = diffAlgo.computeDiffs(i1, i2);

        if (Debug.isFlagSet(Debug.INFO)) {
            Debug.log(Debug.INFO, "Diff Result: ");

            for (int i = 0; i < diffTable.length; i++) {
                Debug.log(Debug.INFO, diffTable[i].debug());
            }
        }

        // merge the paragraphs
        NodeMergeAlgorithm charMerge = new CharacterBaseParagraphMerge();
        DocumentMerge docMerge = new DocumentMerge(cc_, charMerge);

        docMerge.applyDifference(i1, i2, diffTable);
    }
}


