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

using System;
using unoidl.com.sun.star.lang;

namespace uno.util
{

/** Helper class to conveniently auto dispose UNO objects from within
    managed code.
*/
public struct DisposeGuard : IDisposable
{
    private XComponent m_xComponent;
    
    /** ctor.
        
        @param obj target object
    */
    public DisposeGuard( XComponent obj )
    {
        m_xComponent = obj;
    }
    
    /** System.IDisposable impl
    */
    public void Dispose()
    {
        if (null != m_xComponent)
            m_xComponent.dispose();
    }
}

}
