/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JavaWindowPeerFake.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:46:34 $
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
 *************************************************************************/

package embeddedobj.test;

import java.awt.*;

import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.awt.*;
import com.sun.star.util.*;
import com.sun.star.beans.*;
import com.sun.star.container.*;

/** <p>Class to pass the system window handle to the OpenOffice.org toolkit.</p>
 */
class JavaWindowPeerFake implements XSystemDependentWindowPeer,
                                    XWindowPeer
{
    NativeView maView;

    public JavaWindowPeerFake( NativeView aNative )
    {
        maView = aNative;
    }

    // ____________________
    /**
     * Implementation of XSystemDependentWindowPeer ( that's all we really need ).
     * This method is called back from the Office toolkit to retrieve the system data.
     */
    public java.lang.Object getWindowHandle( byte[] aProcessId, short aSystem )
        throws com.sun.star.uno.RuntimeException
    {
        Object aReturn = null;
        if( aSystem == maView.maSystem )
            aReturn = ( Object )maView.maHandle;

        return aReturn;
    }

    /** not really neaded.
     */
    public XToolkit getToolkit()
        throws com.sun.star.uno.RuntimeException
    {
        return null;
    }

    public void setPointer( XPointer xPointer )
        throws com.sun.star.uno.RuntimeException
    {
    }

    public void setBackground( int nColor )
        throws com.sun.star.uno.RuntimeException
    {
    }

    public void invalidate( short nFlags )
        throws com.sun.star.uno.RuntimeException
    {
    }

    public void invalidateRect( com.sun.star.awt.Rectangle aRect,short nFlags )
        throws com.sun.star.uno.RuntimeException
    {
    }

    public void dispose()
        throws com.sun.star.uno.RuntimeException
    {
    }

    public void addEventListener( XEventListener xListener )
        throws com.sun.star.uno.RuntimeException
    {
    }

    public void removeEventListener( XEventListener xListener )
        throws com.sun.star.uno.RuntimeException
    {
    }
}

