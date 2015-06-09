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

package integration.forms;

import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.beans.*;
import com.sun.star.container.*;

/** provides global helpers
*/
class dbfTools
{
    /* ------------------------------------------------------------------ */
    /** disposes the component given
    */
    public static void disposeComponent( Object xComp ) throws java.lang.RuntimeException
    {
        XComponent xComponent = queryComponent( xComp );
        if ( null != xComponent )
            xComponent.dispose();
    }

    /* ------------------------------------------------------------------ */
    /** queries an object for the XPropertySet interface
    */
    public static XPropertySet queryPropertySet( Object aComp )
    {
        return UnoRuntime.queryInterface( XPropertySet.class, aComp );
    }

    /* ------------------------------------------------------------------ */
    /** queries an object for the XIndexContainer interface
    */
    public static XIndexContainer queryIndexContainer( Object aComp )
    {
        return UnoRuntime.queryInterface( XIndexContainer.class, aComp );
    }

    /* ------------------------------------------------------------------ */
    /** queries an object for the XComponent interface
    */
    public static XComponent queryComponent( Object aComp )
    {
        return UnoRuntime.queryInterface( XComponent.class, aComp );
    }

    /* ------------------------------------------------------------------ */
    /** retrieves the parent of the given object
    */
    @SuppressWarnings("unchecked")
    static Object getParent( Object aComponent, Class aInterfaceClass )
    {
        XChild xAsChild = UnoRuntime.queryInterface( XChild.class, aComponent );
        return UnoRuntime.queryInterface( aInterfaceClass, xAsChild.getParent() );
    }
}
