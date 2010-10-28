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
                atarget = (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class, oTmp);
            } catch (IndexOutOfBoundsException e) {
                fail("Exception when getting relations "+e);
            }
        }

        assertEquals(
            "didn't gain correct relation type for paragraph 0", types[2],
            types[firstrelation]);

        XAccessibleText paraTxt2 =
          (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class, para2);
        assertEquals(
            "didn't gain correct target paragraph", atarget.getText(),
            paraTxt2.getText());
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
                  atargets[0] = (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class, oTmp);
                  relationtypes[0] = tmprelation;
                }
                else if ( tmprelation == 2 )
                {
                  Object oTmp = set.getRelation(0).TargetSet[0];
                  atargets[1] = (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class, oTmp);
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
                  atargets[0] = (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class, oTmp);
                  relationtypes[0] = tmprelation;
                }
                else if ( tmprelation == 2 )
                {
                  Object oTmp = set.getRelation(1).TargetSet[0];
                  atargets[1] = (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class, oTmp);
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

        XAccessibleText paraTxt1 =
          (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class, para1);
        assertEquals(
            "didn't gain correct target paragraph", atargets[0].getText(),
            paraTxt1.getText());

        assertEquals(
            "didn't gain correct relation type for paragraph 3", types[2],
            types[relationtypes[1]]);

        XAccessibleText paraTxt3 =
          (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class, para3);
        assertEquals(
            "didn't gain correct target paragraph", atargets[1].getText(),
            paraTxt3.getText());
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

        XModel aModel = (XModel)
        UnoRuntime.queryInterface(XModel.class, xTextDoc);

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow(factory, aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

        at.getAccessibleObjectForRole(xRoot, AccessibleRole.DOCUMENT);

        para1 = at.SearchedContext.getAccessibleChild(0);
        para2 = at.SearchedContext.getAccessibleChild(1);
        para3 = at.SearchedContext.getAccessibleChild(2);
    }

    @After public void after() {
        util.DesktopTools.closeDoc(xTextDoc);
    }

    public XAccessibleRelationSet getAccessibleRelation(XAccessible xAcc) {
        XAccessibleContext oObj = (XAccessibleContext)
        UnoRuntime.queryInterface(XAccessibleContext.class, xAcc);

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
