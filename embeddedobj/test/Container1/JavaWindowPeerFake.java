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

