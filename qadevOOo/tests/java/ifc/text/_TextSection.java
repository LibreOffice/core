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

package ifc.text;

import lib.MultiPropertyTest;

import com.sun.star.text.XTextColumns;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;

/**
 * Testing <code>com.sun.star.text.TextSection</code>
 * service properties :
 * <ul>
 *  <li><code> Condition</code></li>
 *  <li><code> IsVisible</code></li>
 *  <li><code> IsProtected</code></li>
 *  <li><code> FileLink</code></li>
 *  <li><code> LinkRegion</code></li>
 *  <li><code> DDECommandType</code></li>
 *  <li><code> DDECommandFile</code></li>
 *  <li><code> DDECommandElement</code></li>
 *  <li><code> BackGraphicURL</code></li>
 *  <li><code> BackGraphicFilter</code></li>
 *  <li><code> BackGraphicLocation</code></li>
 *  <li><code> FootnoteIsCollectAtTextEnd</code></li>
 *  <li><code> FootnoteIsRestartNumbering</code></li>
 *  <li><code> FootnoteRestartNumberingAt</code></li>
 *  <li><code> FootnoteIsOwnNumbering</code></li>
 *  <li><code> FootnoteNumberingType</code></li>
 *  <li><code> FootnoteNumberingPrefix</code></li>
 *  <li><code> FootnoteNumberingSuffix</code></li>
 *  <li><code> EndnoteIsCollectAtTextEnd</code></li>
 *  <li><code> EndnoteIsRestartNumbering</code></li>
 *  <li><code> EndnoteRestartNumberingAt</code></li>
 *  <li><code> EndnoteIsOwnNumbering</code></li>
 *  <li><code> EndnoteNumberingType</code></li>
 *  <li><code> EndnoteNumberingPrefix</code></li>
 *  <li><code> EndnoteNumberingSuffix</code></li>
 *  <li><code> IsAutomaticUpdate</code></li>
 * </ul> <p>
 * The following predefined files needed to complete the test:
 * <ul>
 *  <li> <code>crazy-blue.jpg, space-metal.jpg</code> : are used for
 *  setting 'BackGraphicURL' property. </li>
 * <ul> <p>
 * Properties testing is automated by <code>lib.MultiPropertyTest</code>.
 * @see com.sun.star.text.TextSection
 */
public class _TextSection extends MultiPropertyTest {

    /**
     * Only image file URL can be used as a value.
     */
    public void _BackGraphicURL() {
        log.println("Testing with custom Property tester") ;
        testProperty("BackGraphicURL",
        util.utils.getFullTestURL("crazy-blue.jpg"),
        util.utils.getFullTestURL("space-metal.jpg")) ;
    }

    /**
     * This property can be void, so if old value is <code> null </code>
     * new value must be specified.
     */
    public void _FootnoteNumberingType() {
        log.println("Testing with custom Property tester") ;
        testProperty("FootnoteNumberingType",
        new Short(com.sun.star.text.FootnoteNumbering.PER_DOCUMENT),
        new Short(com.sun.star.text.FootnoteNumbering.PER_PAGE)) ;
    }


    /**
    * Custom property tester for property <code>TextColumns</code>
    */
    protected PropertyTester TextColumnsTester = new PropertyTester() {

        protected Object getNewValue(String propName, Object oldValue) {
        XTextColumns TC = null;
        short val2set = 25;

        TC = (XTextColumns) tEnv.getObjRelation("TC");
        try {
            val2set += ((XTextColumns) AnyConverter.toObject(
                new Type(XTextColumns.class),oldValue)).getColumnCount();
        } catch (com.sun.star.lang.IllegalArgumentException iae) {
            log.println("Couldn't change Column count");
        }
        TC.setColumnCount(val2set);

        return TC;
        };

        protected boolean compare(Object obj1, Object obj2) {
            short val1 = 0;
            short val2 = 1;
            try {
                val1 = ((XTextColumns) AnyConverter.toObject(
                    new Type(XTextColumns.class),obj1)).getColumnCount();
                val2 = ((XTextColumns) AnyConverter.toObject(
                    new Type(XTextColumns.class),obj2)).getColumnCount();
            } catch (com.sun.star.lang.IllegalArgumentException iae) {
                log.println("comparing values failed");
            }
            return val1 == val2;
        }

        protected String toString(Object obj) {
            return "XTextColumns: ColumnCount = "+
                        ((XTextColumns) obj).getColumnCount();
        }

    };

    public void _TextColumns() {
        log.println("Testing with custom Property tester");
        testProperty("TextColumns", TextColumnsTester);
    }


} //finish class _TextContent

