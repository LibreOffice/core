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

package org.apache.openoffice.ooxml.schema.automaton;

import org.apache.openoffice.ooxml.schema.model.complex.Any;

/** Description of optional content that can be skipped when not supported.
 *  Corresponds to the 'any' schema element.
 */
public class SkipData
{
    public SkipData (
        final Any.ProcessContents aProcessContents,
        final String[] aNamespaces)
    {
        maProcessContents = aProcessContents;
        maNamespaces = aNamespaces;
    }




    public SkipData Clone (final State aState)
    {
        return new SkipData(
            maProcessContents,
            maNamespaces);
    }




    final Any.ProcessContents maProcessContents;
    final String[] maNamespaces;
}
