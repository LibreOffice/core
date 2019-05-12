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



package ifc.text;

import lib.MultiPropertyTest;

import com.sun.star.text.ControlCharacter;

/**
 * Testing <code>com.sun.star.text.TextPortion</code>
 * service properties :
 * <ul>
 *  <li><code> TextPortionType</code></li>
 *  <li><code> ControlCharacter</code></li>
 * </ul> <p>
 * Properties testing is automated by <code>lib.MultiPropertyTest</code>.
 * @see com.sun.star.text.TextPortion
 */
public class _TextPortion extends MultiPropertyTest {

    /**
     * This property can be void, so if old value is <code> null </code>
     * new value must be specified.
     */
    public void _ControlCharacter() {
        testProperty("ControlCharacter",
            new Short(ControlCharacter.LINE_BREAK),
            new Short(ControlCharacter.PARAGRAPH_BREAK)) ;
    }

} //finish class _TextPortion

