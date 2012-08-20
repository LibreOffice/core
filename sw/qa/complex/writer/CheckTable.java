/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package complex.writer;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XEnumeration;
import com.sun.star.util.XCloseable;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextTable;
import com.sun.star.table.TableBorder;
import com.sun.star.table.TableBorder2;
import com.sun.star.table.BorderLine;
import com.sun.star.table.BorderLine2;
import static com.sun.star.table.BorderLineStyle.*;

import org.openoffice.test.OfficeConnection;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.*;

import java.util.Set;
import java.util.HashSet;


public class CheckTable
{
    private static final OfficeConnection connection = new OfficeConnection();

    @BeforeClass public static void setUpConnection() throws Exception {
        connection.setUp();
//Thread.sleep(5000);
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        connection.tearDown();
    }

    private XMultiServiceFactory m_xMSF = null;
    private XComponentContext m_xContext = null;
    private XTextDocument m_xDoc = null;

    @Before public void before() throws Exception
    {
        m_xMSF = UnoRuntime.queryInterface(
            XMultiServiceFactory.class,
            connection.getComponentContext().getServiceManager());
        m_xContext = connection.getComponentContext();
        assertNotNull("could not get component context.", m_xContext);
        m_xDoc = util.WriterTools.createTextDoc(m_xMSF);
    }

    @After public void after()
    {
        util.DesktopTools.closeDoc(m_xDoc);
    }

    @Test
    public void test_tableborder() throws Exception
    {
        // insert table
        XMultiServiceFactory xDocF =
            UnoRuntime.queryInterface(XMultiServiceFactory.class, m_xDoc);
        XTextTable xTable = UnoRuntime.queryInterface(XTextTable.class,
            xDocF.createInstance("com.sun.star.text.TextTable"));
        xTable.initialize(3, 3);
        XText xText = m_xDoc.getText();
        XTextCursor xCursor = xText.createTextCursor();
        xText.insertTextContent(xCursor, xTable, false);
        // read orig border
        XPropertySet xProps = UnoRuntime.queryInterface(XPropertySet.class,
                xTable);
        TableBorder border = (TableBorder) xProps.getPropertyValue("TableBorder");
        assertTrue(border.IsTopLineValid);
        assertEquals(0, border.TopLine.InnerLineWidth);
        assertEquals(2, border.TopLine.OuterLineWidth);
        assertEquals(0, border.TopLine.LineDistance);
        assertEquals(0, border.TopLine.Color);
        assertTrue(border.IsBottomLineValid);
        assertEquals(0, border.BottomLine.InnerLineWidth);
        assertEquals(2, border.BottomLine.OuterLineWidth);
        assertEquals(0, border.BottomLine.LineDistance);
        assertEquals(0, border.BottomLine.Color);
        assertTrue(border.IsLeftLineValid);
        assertEquals(0, border.LeftLine.InnerLineWidth);
        assertEquals(2, border.LeftLine.OuterLineWidth);
        assertEquals(0, border.LeftLine.LineDistance);
        assertEquals(0, border.LeftLine.Color);
        assertTrue(border.IsRightLineValid);
        assertEquals(0, border.RightLine.InnerLineWidth);
        assertEquals(2, border.RightLine.OuterLineWidth);
        assertEquals(0, border.RightLine.LineDistance);
        assertEquals(0, border.RightLine.Color);
        assertTrue(border.IsHorizontalLineValid);
        assertEquals(0, border.HorizontalLine.InnerLineWidth);
        assertEquals(2, border.HorizontalLine.OuterLineWidth);
        assertEquals(0, border.HorizontalLine.LineDistance);
        assertEquals(0, border.HorizontalLine.Color);
        assertTrue(border.IsVerticalLineValid);
        assertEquals(0, border.VerticalLine.InnerLineWidth);
        assertEquals(2, border.VerticalLine.OuterLineWidth);
        assertEquals(0, border.VerticalLine.LineDistance);
        assertEquals(0, border.VerticalLine.Color);
        assertTrue(border.IsDistanceValid);
        assertEquals(97, border.Distance);
        // set border
        border.TopLine = new BorderLine(0, (short)11, (short)19, (short)19);
        border.BottomLine = new BorderLine(0xFF, (short)0, (short)11, (short)0);
        border.HorizontalLine = new BorderLine(0xFF00, (short)0, (short)90, (short)0);
        xProps.setPropertyValue("TableBorder", border);
        // read set border
        border = (TableBorder) xProps.getPropertyValue("TableBorder");
        assertTrue(border.IsTopLineValid);
        assertEquals(11, border.TopLine.InnerLineWidth);
        assertEquals(19, border.TopLine.OuterLineWidth);
        assertEquals(19, border.TopLine.LineDistance);
        assertEquals(0, border.TopLine.Color);
        assertTrue(border.IsBottomLineValid);
        assertEquals(0, border.BottomLine.InnerLineWidth);
        assertEquals(11, border.BottomLine.OuterLineWidth);
        assertEquals(0, border.BottomLine.LineDistance);
        assertEquals(0xFF, border.BottomLine.Color);
        assertTrue(border.IsLeftLineValid);
        assertEquals(0, border.LeftLine.InnerLineWidth);
        assertEquals(2, border.LeftLine.OuterLineWidth);
        assertEquals(0, border.LeftLine.LineDistance);
        assertEquals(0, border.LeftLine.Color);
        assertTrue(border.IsRightLineValid);
        assertEquals(0, border.RightLine.InnerLineWidth);
        assertEquals(2, border.RightLine.OuterLineWidth);
        assertEquals(0, border.RightLine.LineDistance);
        assertEquals(0, border.RightLine.Color);
        assertTrue(border.IsHorizontalLineValid);
        assertEquals(0, border.HorizontalLine.InnerLineWidth);
        assertEquals(90, border.HorizontalLine.OuterLineWidth);
        assertEquals(0, border.HorizontalLine.LineDistance);
        assertEquals(0xFF00, border.HorizontalLine.Color);
        assertTrue(border.IsVerticalLineValid);
        assertEquals(0, border.VerticalLine.InnerLineWidth);
        assertEquals(2, border.VerticalLine.OuterLineWidth);
        assertEquals(0, border.VerticalLine.LineDistance);
        assertEquals(0, border.VerticalLine.Color);
        assertTrue(border.IsDistanceValid);
        assertEquals(97, border.Distance);
        TableBorder2 border2 = (TableBorder2) xProps.getPropertyValue("TableBorder2");
        assertTrue(border2.IsTopLineValid);
        assertEquals(11, border2.TopLine.InnerLineWidth);
        assertEquals(19, border2.TopLine.OuterLineWidth);
        assertEquals(19, border2.TopLine.LineDistance);
        assertEquals(0, border2.TopLine.Color);
        assertEquals(DOUBLE, border2.TopLine.LineStyle);
        assertEquals(49, border2.TopLine.LineWidth);
        assertTrue(border2.IsBottomLineValid);
        assertEquals(0, border2.BottomLine.InnerLineWidth);
        assertEquals(11, border2.BottomLine.OuterLineWidth);
        assertEquals(0, border2.BottomLine.LineDistance);
        assertEquals(0xFF, border2.BottomLine.Color);
        assertEquals(SOLID, border2.BottomLine.LineStyle);
        assertEquals(11, border2.BottomLine.LineWidth);
        assertTrue(border2.IsLeftLineValid);
        assertEquals(0, border2.LeftLine.InnerLineWidth);
        assertEquals(2, border2.LeftLine.OuterLineWidth);
        assertEquals(0, border2.LeftLine.LineDistance);
        assertEquals(0, border2.LeftLine.Color);
        assertEquals(SOLID, border2.LeftLine.LineStyle);
        assertEquals(2, border2.LeftLine.LineWidth);
        assertTrue(border2.IsRightLineValid);
        assertEquals(0, border2.RightLine.InnerLineWidth);
        assertEquals(2, border2.RightLine.OuterLineWidth);
        assertEquals(0, border2.RightLine.LineDistance);
        assertEquals(0, border2.RightLine.Color);
        assertEquals(SOLID, border2.RightLine.LineStyle);
        assertEquals(2, border2.RightLine.LineWidth);
        assertTrue(border2.IsHorizontalLineValid);
        assertEquals(0, border2.HorizontalLine.InnerLineWidth);
        assertEquals(90, border2.HorizontalLine.OuterLineWidth);
        assertEquals(0, border2.HorizontalLine.LineDistance);
        assertEquals(0xFF00, border2.HorizontalLine.Color);
        assertEquals(SOLID, border2.HorizontalLine.LineStyle);
        assertEquals(90, border2.HorizontalLine.LineWidth);
        assertTrue(border2.IsVerticalLineValid);
        assertEquals(0, border2.VerticalLine.InnerLineWidth);
        assertEquals(2, border2.VerticalLine.OuterLineWidth);
        assertEquals(0, border2.VerticalLine.LineDistance);
        assertEquals(0, border2.VerticalLine.Color);
        assertEquals(SOLID, border2.VerticalLine.LineStyle);
        assertEquals(2, border2.VerticalLine.LineWidth);
        assertTrue(border2.IsDistanceValid);
        assertEquals(97, border2.Distance);

        // set border2
        border2.RightLine =
            new BorderLine2(0, (short)0, (short)0, (short)0, THICKTHIN_LARGEGAP, 120);
        border2.LeftLine =
            new BorderLine2(0, (short)0, (short)0, (short)0, EMBOSSED, 90);
        border2.VerticalLine =
            new BorderLine2(0xFF, (short)0, (short)90, (short)0, DOTTED, 0);
        border2.HorizontalLine =
            new BorderLine2(0xFF00, (short)0, (short)0, (short)0, DASHED, 11);
        xProps.setPropertyValue("TableBorder2", border2);

        // read set border2
        border2 = (TableBorder2) xProps.getPropertyValue("TableBorder2");
        assertTrue(border2.IsTopLineValid);
        assertEquals(11, border2.TopLine.InnerLineWidth);
        assertEquals(19, border2.TopLine.OuterLineWidth);
        assertEquals(19, border2.TopLine.LineDistance);
        assertEquals(0, border2.TopLine.Color);
        assertEquals(DOUBLE, border2.TopLine.LineStyle);
        assertEquals(49, border2.TopLine.LineWidth);
        assertTrue(border2.IsBottomLineValid);
        assertEquals(0, border2.BottomLine.InnerLineWidth);
        assertEquals(11, border2.BottomLine.OuterLineWidth);
        assertEquals(0, border2.BottomLine.LineDistance);
        assertEquals(0xFF, border2.BottomLine.Color);
        assertEquals(SOLID, border2.BottomLine.LineStyle);
        assertEquals(11, border2.BottomLine.LineWidth);
        assertTrue(border2.IsLeftLineValid);
        assertEquals(23, border2.LeftLine.InnerLineWidth);
        assertEquals(23, border2.LeftLine.OuterLineWidth);
        assertEquals(46, border2.LeftLine.LineDistance);
        assertEquals(0, border2.LeftLine.Color);
        assertEquals(EMBOSSED, border2.LeftLine.LineStyle);
        assertEquals(90, border2.LeftLine.LineWidth);
        assertTrue(border2.IsRightLineValid);
        assertEquals(53, border2.RightLine.InnerLineWidth);
        assertEquals(26, border2.RightLine.OuterLineWidth);
        assertEquals(41, border2.RightLine.LineDistance);
        assertEquals(0, border2.RightLine.Color);
        assertEquals(THICKTHIN_LARGEGAP, border2.RightLine.LineStyle);
        assertEquals(120, border2.RightLine.LineWidth);
        assertTrue(border2.IsHorizontalLineValid);
        assertEquals(0, border2.HorizontalLine.InnerLineWidth);
        assertEquals(11, border2.HorizontalLine.OuterLineWidth);
        assertEquals(0, border2.HorizontalLine.LineDistance);
        assertEquals(0xFF00, border2.HorizontalLine.Color);
        assertEquals(DASHED, border2.HorizontalLine.LineStyle);
        assertEquals(11, border2.HorizontalLine.LineWidth);
        assertTrue(border2.IsVerticalLineValid);
        assertEquals(0, border2.VerticalLine.InnerLineWidth);
        assertEquals(90, border2.VerticalLine.OuterLineWidth);
        assertEquals(0, border2.VerticalLine.LineDistance);
        assertEquals(0xFF, border2.VerticalLine.Color);
        assertEquals(DOTTED, border2.VerticalLine.LineStyle);
        assertEquals(90, border2.VerticalLine.LineWidth);
        assertTrue(border2.IsDistanceValid);
        assertEquals(97, border2.Distance);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
