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
* Testing <code>com.sun.star.style.CharacterPropertiesComplex</code>
* service properties :
* <ul>
*  <li><code> CharHeightComplex</code></li>
*  <li><code> CharWeightComplex</code></li>
*  <li><code> CharFontNameComplex</code></li>
*  <li><code> CharFontStyleNameComplex</code></li>
*  <li><code> CharFontFamilyComplex</code></li>
*  <li><code> CharFontCharSetComplex</code></li>
*  <li><code> CharFontPitchComplex</code></li>
*  <li><code> CharPostureComplex</code></li>
*  <li><code> CharLocaleComplex</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.style.CharacterPropertiesComplex
*/
public class _CharacterPropertiesComplex extends MultiPropertyTest {

    public void _CharWeightComplex() {
        testProperty("CharWeightComplex", new Float(com.sun.star.awt.FontWeight.BOLD),
            new Float(com.sun.star.awt.FontWeight.THIN)) ;
    }

    public void _CharPostureComplex() {
        testProperty("CharPostureComplex", com.sun.star.awt.FontSlant.ITALIC,
            com.sun.star.awt.FontSlant.NONE) ;
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }

} //finish class _CharacterPropertiesComplex

