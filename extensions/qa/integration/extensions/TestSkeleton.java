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

package integration.extensions;

import com.sun.star.lang.XMultiServiceFactory;

public class TestSkeleton extends complexlib.ComplexTestCase
{
    private XMultiServiceFactory    m_orb;

    /** Creates a new instance of ValueBinding */
    public TestSkeleton()
    {
    }

    /* ------------------------------------------------------------------ */
    public String[] getTestMethodNames()
    {
        return new String[] {
            "checkSomething"
        };
    }

    /* ------------------------------------------------------------------ */
    public String getTestObjectName()
    {
        return "Test Skeleton";
    }

    /* ------------------------------------------------------------------ */
    public void before() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        m_orb = (XMultiServiceFactory)param.getMSF();
    }

    /* ------------------------------------------------------------------ */
    public void after() throws com.sun.star.uno.Exception, java.lang.Exception
    {
    }

    /* ------------------------------------------------------------------ */
    public void checkSomething() throws com.sun.star.uno.Exception, java.lang.Exception
    {
    }
}
