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

package installer;
import java.io.ByteArrayOutputStream;
import java.io.PrintStream;

// class for propagating the exception stack traces across the Java/UNO bridge
public class ExceptionTraceHelper
{
    public static String getTrace( Exception e )
    {
        ByteArrayOutputStream baos = null;
        PrintStream ps = null;
        String result = "";
        try
        {
            baos = new ByteArrayOutputStream( 128 );
            ps = new PrintStream( baos );
            e.printStackTrace( ps );
        }
        finally
        {
            try
            {
                if ( baos != null )
                {
                    baos.close();
                }
                if ( ps != null )
                {
                    ps.close();
                }
            }
            catch ( Exception excp )
            {
            }
        }
        return result;
    }
}
