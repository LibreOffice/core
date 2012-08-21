package testlib.uno;

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

// __________ Imports __________

import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;

import com.sun.star.beans.XPropertySet;

import com.sun.star.container.NoSuchElementException;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;

import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;

import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextRange;

public class ShapeUtil {
    // __________ static helper methods __________
    //
    public static XPropertySet createAndInsertShape(XComponent xDrawDoc,
            XShapes xShapes, Point aPos, Size aSize, String sShapeType)
            throws java.lang.Exception {
        XShape xShape = createShape(xDrawDoc, aPos, aSize, sShapeType);
        xShapes.add(xShape);
        XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, xShape);
        return xPropSet;
    }

    /**
     * create a Shape
     */
    public static XShape createShape(XComponent xDrawDoc, Point aPos,
            Size aSize, String sShapeType) throws java.lang.Exception {
        XShape xShape = null;
        XMultiServiceFactory xFactory = (XMultiServiceFactory) UnoRuntime
                .queryInterface(XMultiServiceFactory.class, xDrawDoc);
        Object xObj = xFactory.createInstance(sShapeType);
        xShape = (XShape) UnoRuntime.queryInterface(XShape.class, xObj);
        xShape.setPosition(aPos);
        xShape.setSize(aSize);
        return xShape;
    }

    /**
     * try to get shape according position
     *
     * @param aPos
     * @return
     */
    public static XShape getShape(XComponent xDrawDoc, Point aPos,
            String sShapeType) {
        XShape xShape = null;
        try {
            XMultiServiceFactory xFactory = (XMultiServiceFactory) UnoRuntime
                    .queryInterface(XMultiServiceFactory.class, xDrawDoc);
            Object xObj = xFactory.createInstance(sShapeType);
            xShape = (XShape) UnoRuntime.queryInterface(XShape.class, xObj);
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return xShape;
    }

    /**
     * add text to a shape. the return value is the PropertySet of the text
     * range that has been added
     */
    public static XPropertySet addPortion(XShape xShape, String sText,
            boolean bNewParagraph)
            throws com.sun.star.lang.IllegalArgumentException {
        XText xText = (XText) UnoRuntime.queryInterface(XText.class, xShape);

        XTextCursor xTextCursor = xText.createTextCursor();
        xTextCursor.gotoEnd(false);
        if (bNewParagraph == true) {
            xText.insertControlCharacter(xTextCursor,
                    ControlCharacter.PARAGRAPH_BREAK, false);
            xTextCursor.gotoEnd(false);
        }
        XTextRange xTextRange = (XTextRange) UnoRuntime.queryInterface(
                XTextRange.class, xTextCursor);
        xTextRange.setString(sText);
        xTextCursor.gotoEnd(true);
        XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, xTextRange);
        return xPropSet;
    }

    /**
     * get a paragraph in a shape. the return value is the PropertySet of the text
     * range that specified by the index
     */
    public static XPropertySet getPortion(XShape xShape, int index) throws NoSuchElementException, WrappedTargetException {
        XEnumerationAccess m_paraAccess = (XEnumerationAccess)UnoRuntime.queryInterface(XEnumerationAccess.class, xShape);
        XEnumeration xParaEnum = m_paraAccess.createEnumeration();
        XPropertySet xPropSet = null;
        int i=0;
        while(xParaEnum.hasMoreElements())
        {
            if(i == index)
            {
                Object aPortionObj = xParaEnum.nextElement();
                XTextRange xTextRange = (XTextRange)UnoRuntime.queryInterface(XTextRange.class, aPortionObj);
//              System.out.println(xTextRange.getText().getString());
                xPropSet = (XPropertySet) UnoRuntime.queryInterface(
                        XPropertySet.class, xTextRange);
                break;
            }
            else i++;
        }
        return xPropSet;
    }


    /**
     * try to get text of a shape
     *
     * @return
     */
    public static String getPortion(XShape xShape) {
        String text = null;
        XText xText = (XText) UnoRuntime.queryInterface(XText.class, xShape);

        XTextCursor xTextCursor = xText.createTextCursor();
        XTextRange xTextRange = (XTextRange) UnoRuntime.queryInterface(
                XTextRange.class, xTextCursor);
        text = xTextRange.getString();
        return text;

    }

    public static void setPropertyForLastParagraph(XShape xText,
            String sPropName, Object aValue)
            throws com.sun.star.beans.UnknownPropertyException,
            com.sun.star.beans.PropertyVetoException,
            com.sun.star.lang.IllegalArgumentException,
            com.sun.star.lang.WrappedTargetException,
            com.sun.star.container.NoSuchElementException {
        XEnumerationAccess xEnumerationAccess = (XEnumerationAccess) UnoRuntime
                .queryInterface(XEnumerationAccess.class, xText);
        if (xEnumerationAccess.hasElements()) {
            XEnumeration xEnumeration = xEnumerationAccess.createEnumeration();
            while (xEnumeration.hasMoreElements()) {
                Object xObj = xEnumeration.nextElement();
                if (xEnumeration.hasMoreElements() == false) {
                    XTextContent xTextContent = (XTextContent) UnoRuntime
                            .queryInterface(XTextContent.class, xObj);
                    XPropertySet xParaPropSet = (XPropertySet) UnoRuntime
                            .queryInterface(XPropertySet.class, xTextContent);
                    xParaPropSet.setPropertyValue(sPropName, aValue);
                }
            }
        }
    }
}
