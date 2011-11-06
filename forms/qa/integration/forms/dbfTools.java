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
    static public void disposeComponent( Object xComp ) throws java.lang.RuntimeException
    {
        XComponent xComponent = queryComponent( xComp );
        if ( null != xComponent )
            xComponent.dispose();
    }

    /* ------------------------------------------------------------------ */
    /** queries an object for the XPropertySet interface
    */
    static public XPropertySet queryPropertySet( Object aComp )
    {
        return UnoRuntime.queryInterface( XPropertySet.class, aComp );
    }

    /* ------------------------------------------------------------------ */
    /** queries an object for the XIndexContainer interface
    */
    static public XIndexContainer queryIndexContainer( Object aComp )
    {
        return UnoRuntime.queryInterface( XIndexContainer.class, aComp );
    }

    /* ------------------------------------------------------------------ */
    /** queries an object for the XComponent interface
    */
    static public XComponent queryComponent( Object aComp )
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
};
