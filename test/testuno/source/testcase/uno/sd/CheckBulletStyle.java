/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/
/**
 *
 */
package testcase.uno.sd;
import static org.junit.Assert.*;
import static testlib.uno.PageUtil.getDrawPageByIndex;
import static testlib.uno.ShapeUtil.*;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexReplace;
import com.sun.star.drawing.XShape;

import com.sun.star.lang.XComponent;

import com.sun.star.style.NumberingType;
import com.sun.star.uno.UnoRuntime;
import testlib.uno.SDUtil;

/**
 * 1. New a SD
2. Insert some text
3. Set bullet on
4. Change the bullet color and bullet size
5. save/close/reopen and then check the bullet color and size
 *
 */
public class CheckBulletStyle {

    private static final UnoApp app = new UnoApp();

    private XComponent m_xSDComponent = null;
    private String m_filePath = null;
//  private XShape m_xsecondTextBox = null;
    Object m_numberingRules = null;
    XPropertySet m_textProperty = null;
    XIndexReplace m_xReplace = null;

    @Before
    public void setUpDocument() throws Exception {
        m_filePath = Testspace.getPath("temp/CheckBulletStyle.odt");
        if(FileUtil.fileExists(m_filePath))
        {   //load
            m_xReplace = load();
        }
        else{
            //create a sd
            m_xSDComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, app.newDocument("simpress"));
            Object firstPage = getDrawPageByIndex(m_xSDComponent, 0);
            Object secondTextBox = SDUtil.getShapeOfPageByIndex(firstPage, 1);
            XShape xsecondTextBox = (XShape)UnoRuntime.queryInterface(XShape.class, secondTextBox);
            m_textProperty = addPortion(xsecondTextBox, "Test Bullet Style", false);

            //get numberingRules
            m_numberingRules = m_textProperty.getPropertyValue("NumberingRules");

            m_xReplace = (XIndexReplace) UnoRuntime.queryInterface(
                     XIndexReplace.class, m_numberingRules);

            PropertyValue[] props = new PropertyValue[1];
            props[0] = new PropertyValue();
            props[0].Name = "NumberingType";
            props[0].Value = new Short(NumberingType.CHAR_SPECIAL );

            //set numberingType
            m_xReplace.replaceByIndex(0, props);
            m_textProperty.setPropertyValue("NumberingRules", m_numberingRules);
            //set numbering level to 0
            m_textProperty.setPropertyValue("NumberingLevel", new Short((short)0));
        }
    }
    private XIndexReplace load() throws Exception{
        m_xSDComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class,
                app.loadDocument(m_filePath));
        Object firstPage = getDrawPageByIndex(m_xSDComponent, 0);
        Object secondTextBox = SDUtil.getShapeOfPageByIndex(firstPage, 1);
        XShape xsecondTextBox = (XShape)UnoRuntime.queryInterface(XShape.class, secondTextBox);
        m_textProperty = getPortion(xsecondTextBox, 0);

        m_numberingRules = m_textProperty.getPropertyValue("NumberingRules");

        XIndexReplace xReplace = (XIndexReplace) UnoRuntime.queryInterface(
                 XIndexReplace.class, m_numberingRules);
        return xReplace;
    }

    @After
    public void tearDownDocument() {
        app.closeDocument(m_xSDComponent);
    }

    @BeforeClass
    public static void setUpConnection() throws Exception {
        app.start();
    }

    @AfterClass
    public static void tearDownConnection() throws InterruptedException,
            Exception {
        app.close();
        //remove the temp file
        FileUtil.deleteFile(Testspace.getPath("temp"));
    }

    @Test
    public void testBulletColor() throws Exception {
        //BulletColor, Integer
        PropertyValue[] props = new PropertyValue[1];
        props[0] = new PropertyValue();
        props[0].Name = "BulletColor";
        props[0].Value = new Integer(255);

        m_xReplace.replaceByIndex(0, props);
        m_textProperty.setPropertyValue("NumberingRules", m_numberingRules);

        app.saveDocument(m_xSDComponent, m_filePath);
        app.closeDocument(m_xSDComponent);

        XIndexReplace xReplace = load();
        PropertyValue[] proValues = (PropertyValue[])xReplace.getByIndex(0);
        assertEquals("name should be BulletColor", "BulletColor", proValues[11].Name);
        assertEquals("BulletColor should be 255(Blue)", new Integer(255), proValues[11].Value);
    }

    @Test
    public void testBulletSize() throws Exception {
        //BulletRelSize, default 45
        PropertyValue[] props = new PropertyValue[1];
        props[0] = new PropertyValue();
        props[0].Name = "BulletRelSize";
        props[0].Value = new Short((short)200);

        m_xReplace.replaceByIndex(0, props);
        m_textProperty.setPropertyValue("NumberingRules", m_numberingRules);

        app.saveDocument(m_xSDComponent, m_filePath);
        app.closeDocument(m_xSDComponent);

        XIndexReplace xReplace = load();
        PropertyValue[] proValues = (PropertyValue[])xReplace.getByIndex(0);
        assertEquals("name should be BulletRelSize", "BulletRelSize", proValues[12].Name);
        assertEquals("BulletRelSize should be 200%", new Short((short)200), proValues[12].Value);
    }
}
