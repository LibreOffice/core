/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package com.sun.star.comp.helper;

/** Component context entry for constructing ComponentContext objects.
    <p>
    A ComponentContextEntry is separated into a late-init and direct-value
    purpose.
    The first one is commonly used for singleton objects of the component
    context, that are raised on first-time retrieval of the key.
    You have to pass a com.sun.star.lang.XSingleComponentFactory
    or string (service name) object for this.
    </p>
*/
public class ComponentContextEntry
{
    /** if late init of service instance, set service name (String) or
        component factory (XSingleComponentFactory), null otherwise
    */
    public Object m_lateInit;
    /** set entry value
    */
    public Object m_value;

    /** Creating a late-init singleton entry component context entry.
        The second parameter will be ignored and overwritten during
        instanciation of the singleton instance.

        @param lateInit
               object factory or service string
        @param value
               pass null (dummy separating from second ctor signature)
    */
    public ComponentContextEntry( Object lateInit, Object value )
    {
        this.m_lateInit = lateInit;
        this.m_value = value;
    }
    /** Creating a direct value component context entry.

        @param value
               pass null
    */
    public ComponentContextEntry( Object value )
    {
        this.m_lateInit = null;
        this.m_value = value;
    }
}
