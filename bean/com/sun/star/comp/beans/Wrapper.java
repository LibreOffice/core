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
        xComponent = UnoRuntime.queryInterface(
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


