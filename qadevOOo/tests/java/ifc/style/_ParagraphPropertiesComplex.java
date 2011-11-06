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



package ifc.style;

import lib.MultiPropertyTest;

/**
* Testing <code>com.sun.star.style.ParagraphPropertiesComplex</code>
*
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.style.ParagraphPropertiesComplex
*/
public class _ParagraphPropertiesComplex extends MultiPropertyTest {


    protected PropertyTester WritingModeTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            if ((oldValue != null) && (oldValue.equals(new Short(com.sun.star.text.WritingMode2.LR_TB))))
                return new Short(com.sun.star.text.WritingMode2.PAGE); else
                return new Short(com.sun.star.text.WritingMode2.LR_TB);
        }
    } ;

    public void _WritingMode() {
        log.println("Testing with custom Property tester") ;
        testProperty("WritingMode", WritingModeTester) ;
    }

}  // finish class _ParagraphPropertiesComplex

