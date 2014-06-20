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

package org.apache.openoffice.ooxml.parser.action;

import javax.xml.stream.Location;

import org.apache.openoffice.ooxml.parser.ElementContext;

/** Interface for actions that are bound to states and triggered by XML events.
 */
public interface IAction
{
    /** Callback for a single XML event.
     *  @param eTrigger
     *      Equivalent to the XML event type.
     *  @param aContext
     *      The context of the element that was just entered (element start),
     *      is about to be left (element end) or is currently active (all other
     *      events).
     * @param sText
     *      Contains text for ActionTrigger.Text.  Is null for all other
     *      triggers.
     * @param aStartLocation
     *      The location in the source file where the triggering element starts.
     * @param aEndLocation
     *      The location in the source file where the triggering element ends.
     */
    void Run (
        final ActionTrigger eTrigger,
        final ElementContext aContext,
        final String sText,
        final Location aStartLocation,
        final Location aEndLocation);
}
