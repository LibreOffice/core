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

package com.sun.star.comp.beans;

import com.sun.star.uno.UnoRuntime;

//==========================================================================
/** Wrapper base class for UNO services which emulates the upcoming
     mode of automatic runtime Java classes to get rid of the need for
    queryInterface.

    Because its not worth the efford to create a runtime generated wrapper
    for this purpose, as it might be for OOo 2.0, you still have to use
    UnoRuntime.queryInterface() for interfaces which are optional or come
    from a subclass.  But for non optional interfaces you can already
    directly call their methods.

    This wrapper will only work for UNO objects via a bridge, not for
    direct Java objects.

    @since OOo 2.0.0
 */
class Wrapper
    implements
        com.sun.star.lib.uno.Proxy,
            // see the comment in com.sun.star.lib.uno.bridges.java_remote
            // .java_remote_bridge.mapInterfaceTo for the consequences of this
            // hack
        com.sun.star.uno.IQueryInterface,
        com.sun.star.lang.XComponent
{
    private com.sun.star.uno.IQueryInterface xQueryInterface;
    private com.sun.star.lang.XComponent xComponent;

    public Wrapper( com.sun.star.uno.XInterface xProxy )
    {
        xQueryInterface = (com.sun.star.uno.IQueryInterface) xProxy;
        xComponent = (com.sun.star.lang.XComponent)
            UnoRuntime.queryInterface(
                com.sun.star.lang.XComponent.class, xProxy );
    }

    //==============================================================
    // com.sun.star.uno.IQueryInterface
    //--------------------------------------------------------------

    public String getOid()
    {
        return xQueryInterface.getOid();
    }

    public boolean isSame( Object aObject )
    {
        return xQueryInterface.isSame( aObject );
    }

    public Object queryInterface( com.sun.star.uno.Type aType )
    {
//System.err.println( "Wrapper::queryInterface(" + aType + ")" );
        return xQueryInterface.queryInterface( aType );
    }

    //==============================================================
    // com.sun.star.lang.XComponent
    //--------------------------------------------------------------

    public void dispose(  )
    {
        xComponent.dispose();
    }

    public void addEventListener( /*IN*/ com.sun.star.lang.XEventListener xListener )
    {
        xComponent.addEventListener( xListener );
    }

    public void removeEventListener( /*IN*/ com.sun.star.lang.XEventListener xListener )
    {
        xComponent.removeEventListener( xListener );
    }
};


