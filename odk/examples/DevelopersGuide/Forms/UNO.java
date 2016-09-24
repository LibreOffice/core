/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

import com.sun.star.uno.*;
import com.sun.star.beans.*;
import com.sun.star.awt.*;
import com.sun.star.container.*;
import com.sun.star.form.*;
import com.sun.star.lang.*;
import com.sun.star.sdb.*;

public class UNO
{
    public static XPropertySet queryPropertySet( Object aObject )
    {
        return UnoRuntime.queryInterface( XPropertySet.class, aObject );
    }
    public static XControlModel queryControlModel( Object aObject )
    {
        return UnoRuntime.queryInterface( XControlModel.class, aObject );
    }
    public static XIndexContainer queryIndexContainer( Object aObject )
    {
        return UnoRuntime.queryInterface( XIndexContainer.class, aObject );
    }
    public static XReset queryReset( Object aObject )
    {
        return UnoRuntime.queryInterface( XReset.class, aObject );
    }
    public static XServiceInfo queryServiceInfo( Object aObject )
    {
        return UnoRuntime.queryInterface( XServiceInfo.class, aObject );
    }
    public static XColumn queryColumn( Object aObject )
    {
        return UnoRuntime.queryInterface( XColumn.class, aObject );
    }

    public static XComponent queryComponent( Object aObject )
    {
        return UnoRuntime.queryInterface( XComponent.class, aObject );
    }



/*  replace Foo with the identifier of your choice.

    Why does Java not have templates?

    public static XFoo queryFoo( Object aObject )
    {
        return (XFoo)UnoRuntime.queryInterface( XFoo.class, aObject );
    }
*/
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
