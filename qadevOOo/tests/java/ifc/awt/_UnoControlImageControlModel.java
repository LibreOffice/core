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



package ifc.awt;

import lib.MultiPropertyTest;
import util.utils;

/**
* Testing <code>com.sun.star.awt.UnoControlImageControlModel</code>
* service properties :
* <ul>
*  <li><code> BackgroundColor </code></li>
*  <li><code> Border </code></li>
*  <li><code> DefaultControl </code></li>
*  <li><code> Enabled </code></li>
*  <li><code> ImageURL </code></li>
*  <li><code> Printable </code></li>
* </ul>
* Almost all properties testing is automated by
* <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.awt.UnoControlImageControlModel
*/
public class _UnoControlImageControlModel extends MultiPropertyTest {

    /**
    * Custom <code>PropertyTester</code> used in <code>ImageURL</code> test.
    */
    protected PropertyTester URLTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if (oldValue.equals(util.utils.getFullTestURL("space-metal.jpg")))
                return util.utils.getFullTestURL("crazy-blue.jpg");
            else
                return util.utils.getFullTestURL("space-metal.jpg");
        }
    } ;

    /**
    * Test changes one image URL onto another.
    */
    public void _ImageURL() {
        testProperty("ImageURL", URLTester) ;
    }

    /**
    * This property can be VOID, and in case if it is so new
    * value must defined.
    */
    public void _BackgroundColor() {
        testProperty("BackgroundColor", new PropertyTester() {
            protected Object getNewValue(String propName, Object oldValue) {
                return utils.isVoid(oldValue) ? new Integer(8945) :
                    super.getNewValue(propName, oldValue) ;
            }
        }) ;
    }

   /**
    * This property can be VOID, and in case if it is so new
    * value must defined.
    */
    public void _BorderColor() {
        testProperty("BorderColor", new PropertyTester() {
            protected Object getNewValue(String p, Object old) {
                return utils.isVoid(old) ? new Integer(1234) : null ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _Tabstop() {
        testProperty("Tabstop", new PropertyTester() {
            protected Object getNewValue(String p, Object v) {
                return utils.isVoid(v) ? new Boolean(true) :
                    null ;
            }
        }) ;
    }

}

