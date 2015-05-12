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

package complex.accessibility;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleRelationSet;
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XModel;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import util.AccessibilityTools;
import util.WriterTools;
import static org.junit.Assert.*;

public class AccessibleRelationSet {
    private XAccessible para1 = null;
    private XAccessible para2 = null;
    private XAccessible para3 = null;
    private XTextDocument xTextDoc = null;
    private final static String[] types = {"INVALID","CONTENT_FLOWS_FROM","CONTENT_FLOWS_TO","CONTROLLED_BY","CONTROLLER_FOR","LABEL_FOR","LABELED_BY","MEMBER_OF","SUB_WINDOW_OF"};

    @Test public void contents_flows_to() {
        XAccessibleRelationSet set = getAccessibleRelation(para1);

        short firstrelation=-1;
        XAccessibleText atarget=null;
        if (set != null) {
            assertEquals(
                "didn't gain correct count of relations", 1,
                set.getRelationCount());
            try {
                firstrelation = set.getRelation(0).RelationType;
                Object oTmp = set.getRelation(0).TargetSet[0];
                atarget = UnoRuntime.queryInterface(XAccessibleText.class, oTmp);
            } catch (IndexOutOfBoundsException e) {
                fail("Exception when getting relations "+e);
            }
        }

        assertEquals(
            "didn't gain correct relation type for paragraph 0", types[2],
            types[firstrelation]);

        XAccessibleText paraText2 =
          UnoRuntime.queryInterface(XAccessibleText.class, para2);
        assertEquals(
            "didn't gain correct target paragraph", atarget.getText(),
            paraText2.getText());
    }

    @Test public void contents_flows_from() {
        XAccessibleRelationSet set = getAccessibleRelation(para2);

        short[] relationtypes = new short[2];
        XAccessibleText[] atargets = new XAccessibleText[2];
        if (set != null) {
            assertEquals(
                "didn't gain correct count of relations", 2,
                set.getRelationCount());
            try {
                short tmprelation = set.getRelation(0).RelationType;
                if ( tmprelation == 1 )
                {
                  Object oTmp = set.getRelation(0).TargetSet[0];
                  atargets[0] = UnoRuntime.queryInterface(XAccessibleText.class, oTmp);
                  relationtypes[0] = tmprelation;
                }
                else if ( tmprelation == 2 )
                {
                  Object oTmp = set.getRelation(0).TargetSet[0];
                  atargets[1] = UnoRuntime.queryInterface(XAccessibleText.class, oTmp);
                  relationtypes[1] = tmprelation;
                }
                else
                {
                    fail("didn't gain correct relation type");
                }
                tmprelation = set.getRelation(1).RelationType;
                if ( tmprelation == 1 )
                {
                  Object oTmp = set.getRelation(1).TargetSet[0];
                  atargets[0] = UnoRuntime.queryInterface(XAccessibleText.class, oTmp);
                  relationtypes[0] = tmprelation;
                }
                else if ( tmprelation == 2 )
                {
                  Object oTmp = set.getRelation(1).TargetSet[0];
                  atargets[1] = UnoRuntime.queryInterface(XAccessibleText.class, oTmp);
                  relationtypes[1] = tmprelation;
                }
                else
                {
                    fail("didn't gain correct relation type");
                }
            } catch (IndexOutOfBoundsException e) {
                fail("Exception when getting relations "+e);
            }
        }

        assertEquals(
            "didn't gain correct relation type for paragraph 1", types[1],
            types[relationtypes[0]]);

        XAccessibleText paraText1 =
          UnoRuntime.queryInterface(XAccessibleText.class, para1);
        assertEquals(
            "didn't gain correct target paragraph", atargets[0].getText(),
            paraText1.getText());

        assertEquals(
            "didn't gain correct relation type for paragraph 3", types[2],
            types[relationtypes[1]]);

        XAccessibleText paraText3 =
          UnoRuntime.queryInterface(XAccessibleText.class, para3);
        assertEquals(
            "didn't gain correct target paragraph", atargets[1].getText(),
            paraText3.getText());
    }

    @Before public void before()
        throws com.sun.star.lang.IllegalArgumentException,
        IndexOutOfBoundsException
    {
        XMultiServiceFactory factory = UnoRuntime.queryInterface(
            XMultiServiceFactory.class,
            connection.getComponentContext().getServiceManager());

        xTextDoc = WriterTools.createTextDoc(factory);

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        for (int i=0; i<5; i++){
            oText.insertString( oCursor,"Paragraph Number: " + i, false);
            oText.insertControlCharacter(
                oCursor, ControlCharacter.PARAGRAPH_BREAK, false );
        }

        XModel aModel = UnoRuntime.queryInterface(XModel.class, xTextDoc);

        XAccessibleContext ctx;
        for (int i = 0;; ++i) {
            XWindow xWindow = AccessibilityTools.getCurrentWindow(aModel);
            XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);
            ctx = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.DOCUMENT_TEXT);
            if (ctx != null) {
                break;
            }
            if (i == 20) { // give up after 10 sec
                throw new RuntimeException(
                    "Couldn't get AccessibleRole.DOCUMENT_TEXT object");
            }
            System.out.println("No DOCUMENT_TEXT found yet, retrying");
            try {
                Thread.sleep(500);
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
        }

        para1 = ctx.getAccessibleChild(0);
        para2 = ctx.getAccessibleChild(1);
        para3 = ctx.getAccessibleChild(2);
    }

    @After public void after() {
        util.DesktopTools.closeDoc(xTextDoc);
    }

    public XAccessibleRelationSet getAccessibleRelation(XAccessible xAcc) {
        XAccessibleContext oObj = UnoRuntime.queryInterface(XAccessibleContext.class, xAcc);

        XAccessibleRelationSet set = oObj.getAccessibleRelationSet();
        return set;
    }

    @BeforeClass public static void setUpConnection() throws Exception {
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();
}
