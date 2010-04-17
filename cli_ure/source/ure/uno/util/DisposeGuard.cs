/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
