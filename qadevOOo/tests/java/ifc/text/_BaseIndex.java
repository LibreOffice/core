/*************************************************************************
 *
 *  $RCSfile: _BaseIndex.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:12:48 $
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

package ifc.text;

import lib.MultiPropertyTest;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XIndexReplace;
import com.sun.star.text.XTextColumns;

/**
* Testing <code>com.sun.star.text.BaseIndex</code>
* service properties :
* <ul>
*  <li><code> Title</code></li>
*  <li><code> IsProtected</code></li>
*  <li><code> ParaStyleHeading</code></li>
*  <li><code> ParaStyleLevel1</code></li>
*  <li><code> ParaStyleLevel2</code></li>
*  <li><code> ParaStyleLevel3</code></li>
*  <li><code> ParaStyleLevel4</code></li>
*  <li><code> ParaStyleLevel5</code></li>
*  <li><code> ParaStyleLevel6</code></li>
*  <li><code> ParaStyleLevel7</code></li>
*  <li><code> ParaStyleLevel8</code></li>
*  <li><code> ParaStyleLevel9</code></li>
*  <li><code> ParaStyleLevel10</code></li>
*  <li><code> ParaStyleSeparator</code></li>
*  <li><code> TextColumns</code></li>
*  <li><code> BackGraphicURL</code></li>
*  <li><code> BackGraphicFilter</code></li>
*  <li><code> BackGraphicLocation</code></li>
*  <li><code> BackTransparent</code></li>
*  <li><code> LevelFormat</code></li>
*  <li><code> CreateFromChapter</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.text.BaseIndex
*/
public class _BaseIndex extends MultiPropertyTest {

    /**
    * Redefined method returns object, that contains changed property value.
    */
    protected PropertyTester CustomTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            XTextColumns TC = (XTextColumns) oldValue;
            TC.setColumnCount((short)(TC.getColumnCount() + (short) 1));
            return TC;
        }
    };

    /**
    * New value must be defined for this property.
    */
    public void _TextColumns() {
        log.println("Testing property 'TextColumns' with custom property tester") ;
        testProperty("TextColumns", CustomTester) ;
    }

    /**
     * The value of this property is a collection of document index
     * level formats. This property is tested in the following manner :
     * the property value (a collection) is obtained, the first element
     * of this collection is replaced with new non-empty array
     * (<code>PropertyValue[][]</code>) with some properties set.
     * After that the collection is set back as property value. <p>
     *
     * Comparing of set and got <code>PropertyValue</code> arrays
     * is difficult because values can be changed after setting
     * by service miplementation. <p>
     *
     * Has <b>OK</b> status if the collection again gotten, has a
     * new first element (i.e. lengths of the old array and the array
     * get are different or their contents differ).
     */
    public void _LevelFormat() {
        log.println("Testing property 'LevelFormat' with custom property tester") ;
        testProperty("LevelFormat", new PropertyTester() {

            PropertyValue[][] newVal = null;
            PropertyValue[][] oldVal = null ;

            protected Object getNewValue(String propName, Object oldValue) {
                XIndexReplace indProp = (XIndexReplace) oldValue;
                try {
                    oldVal = (PropertyValue[][]) indProp.getByIndex(0) ;

                    log.println("Get:") ;
                    printLevelFormatProperty(oldValue) ;

                    newVal = new PropertyValue[1][2];

                    for (int i=0; i < newVal[0].length; i++) {
                        newVal[0][i] = new PropertyValue();
                    }
                    newVal[0][1].Name = "TokenType";
                    newVal[0][1].Value = "TokenEntryText";
                    newVal[0][0].Name = "Text";
                    newVal[0][0].Value = "BaseIndex";

                    indProp.replaceByIndex(0, newVal);
                } catch ( com.sun.star.lang.WrappedTargetException e ) {
                    log.println("Exception occured while testing LevelFormat");
                    e.printStackTrace(log);
                } catch ( com.sun.star.lang.IndexOutOfBoundsException e ) {
                    log.println("Exception occured while testing LevelFormat");
                    e.printStackTrace(log);
                } catch ( com.sun.star.lang.IllegalArgumentException e ) {
                    log.println("Exception occured while testing LevelFormat");
                    e.printStackTrace(log);
                }
                return indProp;
            }

            protected void checkResult(String propName, Object oldValue,
                Object newValue, Object resValue, Exception exception)
                throws Exception {

                PropertyValue[][] res = (PropertyValue[][])
                    ((XIndexAccess) resValue ).getByIndex(0);

                log.println("Result:");
                printLevelFormatProperty(resValue) ;

                boolean result = res.length != oldVal.length ||
                    !util.ValueComparer.equalValue(res, oldVal);

                tRes.tested(propName, result) ;
            }

        });
    }

    /**
     * Outputs full description of 'LevelFormat' property
     * value into <code>log</code>.
     */
    private void printLevelFormatProperty(Object value) {
        XIndexReplace indProp = (XIndexReplace) value;
        PropertyValue[][] val = null ;
        try {
            log.println(" \u0421ollection has " + indProp.getCount() +
                " elements : ") ;

            for (int i =0 ; i < indProp.getCount(); i++) {
                val = (PropertyValue[][]) indProp.getByIndex(i) ;

                log.println("  " + i + ": has " + val.length + " levels :") ;

                for (int j = 0; j < val.length; j++) {
                    log.println("    " + j + " level :") ;

                    for (int k = 0; k < val[j].length; k++) {
                        log.println("      " + val[j][k].Name + "=" +
                            val[j][k].Value) ;
                    }
                }
            }
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            log.println("Exception occured while printing LevelFormat");
            e.printStackTrace(log);
        } catch ( com.sun.star.lang.IndexOutOfBoundsException e ) {
            log.println("Exception occured while printing LevelFormat");
            e.printStackTrace(log);
        }
    }

}  // finish class _NumberingRules


