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

package test.java_uno.any;

//"any" is a reserved word in IDL, so we need to use a different packagename
import test.java_uno.anytest.*;

public class TestJni
{
    static { System.loadLibrary( "test_javauno_any" ); }
    private static native XTransport create_jni_transport(ClassLoader loader);

    public static void main( String args [] )
    {
        if (TestAny.test(
                create_jni_transport(TestJni.class.getClassLoader()), false ))
        {
            System.out.println( "jni any test succeeded." );
        }
        else
        {
            System.err.println( "jni any test failed!" );
            System.exit( 1 );
        }
    }
}
