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

    /** not really needed.
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

