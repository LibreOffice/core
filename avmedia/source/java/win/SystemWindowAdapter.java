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

import sun.awt.*;
import com.sun.star.awt.*;

public class SystemWindowAdapter
{
    static public java.awt.Frame createFrame( int windowHandle )
    {
        java.awt.Frame aFrame;

        // we're initialized with the operating system window handle
        // as the parameter. We then generate a dummy Java frame with
        // that window as the parent, to fake a root window for the
        // Java implementation.

         // now, we're getting slightly system dependent here.
         String os = (String) System.getProperty( "os.name" );

         // create the embedded frame
         if( os.startsWith( "Windows" ) )
             aFrame = new sun.awt.windows.WEmbeddedFrame( windowHandle );
         else
             throw new com.sun.star.uno.RuntimeException();

        return aFrame;
    }
}
