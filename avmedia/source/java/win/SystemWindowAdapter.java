/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SystemWindowAdapter.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:41:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
