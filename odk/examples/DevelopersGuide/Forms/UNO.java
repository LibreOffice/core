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



import com.sun.star.uno.*;
import com.sun.star.beans.*;
import com.sun.star.awt.*;
import com.sun.star.container.*;
import com.sun.star.form.*;
import com.sun.star.lang.*;
import com.sun.star.sdb.*;
import com.sun.star.sdbcx.*;

public class UNO
{
    public static XPropertySet queryPropertySet( Object aObject )
    {
        return (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, aObject );
    }
    public static XControlModel queryControlModel( Object aObject )
    {
        return (XControlModel)UnoRuntime.queryInterface( XControlModel.class, aObject );
    }
    public static XIndexContainer queryIndexContainer( Object aObject )
    {
        return (XIndexContainer)UnoRuntime.queryInterface( XIndexContainer.class, aObject );
    }
    public static XReset queryReset( Object aObject )
    {
        return (XReset)UnoRuntime.queryInterface( XReset.class, aObject );
    }
    public static XServiceInfo queryServiceInfo( Object aObject )
    {
        return (XServiceInfo)UnoRuntime.queryInterface( XServiceInfo.class, aObject );
    }
    public static XColumn queryColumn( Object aObject )
    {
        return (XColumn)UnoRuntime.queryInterface( XColumn.class, aObject );
    }

    public static XComponent queryComponent( Object aObject )
    {
        return (XComponent)UnoRuntime.queryInterface( XComponent.class, aObject );
    }

    public static XTablesSupplier queryTablesSupplier( Object aObject )
    {
        return (XTablesSupplier)UnoRuntime.queryInterface( XTablesSupplier.class, aObject );
    }

/*  replace Foo with the identifier of your choice.

    Why does Java not have templates?

    public static XFoo queryFoo( Object aObject )
    {
        return (XFoo)UnoRuntime.queryInterface( XFoo.class, aObject );
    }
*/
};
