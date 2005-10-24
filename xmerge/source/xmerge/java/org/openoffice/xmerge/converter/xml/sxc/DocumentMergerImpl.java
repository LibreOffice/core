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

package org.openoffice.xmerge.converter.xml.sxc;

import org.w3c.dom.Node;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import org.openoffice.xmerge.Document;
import org.openoffice.xmerge.DocumentMerger;
import org.openoffice.xmerge.MergeException;
import org.openoffice.xmerge.ConverterCapabilities;
import org.openoffice.xmerge.converter.xml.OfficeConstants;
import org.openoffice.xmerge.merger.DiffAlgorithm;
import org.openoffice.xmerge.merger.Difference;
import org.openoffice.xmerge.merger.Iterator;
import org.openoffice.xmerge.merger.DiffAlgorithm;
import org.openoffice.xmerge.merger.NodeMergeAlgorithm;
import org.openoffice.xmerge.merger.diff.IteratorRowCompare;
import org.openoffice.xmerge.merger.diff.RowIterator;
import org.openoffice.xmerge.merger.merge.SheetMerge;
import org.openoffice.xmerge.merger.merge.PositionBaseRowMerge;
import org.openoffice.xmerge.merger.MergeAlgorithm;
import org.openoffice.xmerge.util.XmlUtil;
import org.openoffice.xmerge.util.Debug;


/**
 *  Generic small device implementation of <code>DocumentMerger</code> for
 *  the {@link
 *  org.openoffice.xmerge.converter.xml.sxc.SxcPluginFactory
 *  SxcPluginFactory}.  Used with SXC <code>Document</code> objects.</p>
 */
public class DocumentMergerImpl implements DocumentMerger {

    private ConverterCapabilities cc_;
    private org.openoffice.xmerge.Document orig = null;

    /**
     *  Constructor
     *
     *  @param  doc  The original &quot;Office&quot; <code>Document</code>
     *               to merge.
     *  @param  cc   The <code>ConverterCapabilities</code>.
     */
    public DocumentMergerImpl(org.openoffice.xmerge.Document doc, ConverterCapabilities cc) {
        cc_ = cc;
        this.orig = doc;
    }

    public void merge(Document modifiedDoc) throws MergeException {

        SxcDocument sdoc1 = (SxcDocument)orig;
        SxcDocument sdoc2 = (SxcDocument)modifiedDoc;

        org.w3c.dom.Document doc1 = sdoc1.getContentDOM();
        org.w3c.dom.Document doc2 = sdoc2.getContentDOM();

        Element elem1 = doc1.getDocumentElement();
        Element elem2 = doc2.getDocumentElement();

        // get table name
        NodeList workSheetList1 =
            elem1.getElementsByTagName(OfficeConstants.TAG_TABLE);
        NodeList workSheetList2 =
            elem2.getElementsByTagName(OfficeConstants.TAG_TABLE);

        int numOfWorkSheet = workSheetList1.getLength();

        for (int i=0; i < numOfWorkSheet; i++) {
            Node workSheet = workSheetList1.item(i);

            // try to match the workSheet
            Node matchingWorkSheet = matchWorkSheet(workSheet, workSheetList2);

            if (matchingWorkSheet != null) {

                // need to put it into a row Iterator
                // use a straight comparsion algorithm then do a merge on it
                Iterator i1 = new RowIterator(cc_, workSheet);
                Iterator i2 = new RowIterator(cc_, matchingWorkSheet);

                // find out the diff
                DiffAlgorithm diffAlgo = new IteratorRowCompare();

                // find out the paragrah level diffs
                Difference[] diffResult = diffAlgo.computeDiffs(i1, i2);

                if (Debug.isFlagSet(Debug.INFO)) {
                    Debug.log(Debug.INFO, "Diff Result: ");

                    for (int j = 0; j < diffResult.length; j++) {
                        Debug.log(Debug.INFO, diffResult[j].debug());
                    }
                }

                // merge back the result
                NodeMergeAlgorithm rowMerger = new PositionBaseRowMerge(cc_);
                MergeAlgorithm merger = new SheetMerge(cc_, rowMerger);

                Iterator result = null;

                merger.applyDifference(i1, i2, diffResult);
            }
        }

        numOfWorkSheet = workSheetList2.getLength();

        // for those workSheet from target don't have a matching one
        // in the original workSheet list, we add it

        // find out the office body node first
        NodeList officeBodyList =
            elem1.getElementsByTagName(OfficeConstants.TAG_OFFICE_BODY);

        Node officeBody = officeBodyList.item(0);

        // for each WorkSheets, try to see whether we have it or not
        for (int j=0; j < numOfWorkSheet; j++) {
            Node workSheet= workSheetList2.item(j);

            // try to match the workSheet
            //
            Node matchingWorkSheet = matchWorkSheet(workSheet, workSheetList1);

            // add the new WorkSheet to the original document iff match not
            // found
            //
            if (matchingWorkSheet == null) {
                Node cloneNode = XmlUtil.deepClone(officeBody, workSheet);
                officeBody.appendChild(cloneNode);
            }
        }
    }

    /**
     *  Try to find a WorkSheet from the modified WorkSheetList that
     *  has a matching table name from the original WorkSheet.
     *
     *  @param  orgSheet      The original WorkSheet.
     *  @param  modSheetList  The modified WorkSheet.
     *
     *  @return  The Node in modSheetList that matches the orgSheet.
     */
    private Node matchWorkSheet(Node orgSheet, NodeList modSheetList) {

        Node matchSheet = null;

        String orgTableName = ((Element)orgSheet).getAttribute(
                            OfficeConstants.ATTRIBUTE_TABLE_NAME);

        if (orgTableName == null)
            return null;

        int numOfWorkSheet = modSheetList.getLength();

        String modTableName;

        for (int i=0; i < numOfWorkSheet; i++) {
            modTableName = ((Element)modSheetList.item(i)).getAttribute(
                            OfficeConstants.ATTRIBUTE_TABLE_NAME);
            if (modTableName == null)
                continue;

            if (orgTableName.equals(modTableName)) {
                matchSheet = modSheetList.item(i);
                break;
            }
        }

        return matchSheet;
    }
}

