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

package org.openoffice.xmerge.converter.xml.sxw.aportisdoc;

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
 *  AportisDoc implementation of <code>DocumentMerger</code>
 *  for the {@link
 *  org.openoffice.xmerge.converter.xml.sxw.aportisdoc.PluginFactoryImpl
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

        Iterator result = null;

        docMerge.applyDifference(i1, i2, diffTable);
    }
}


