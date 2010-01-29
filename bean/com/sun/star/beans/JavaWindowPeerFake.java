/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: JavaWindowPeerFake.java,v $
 * $Revision: 1.4 $
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

package com.sun.star.beans;

import com.sun.star.awt.*;

/** <p>Class to pass the system window handle to the OpenOffice.org toolkit.</p>
 *  @deprecated
 */
/* package */ class JavaWindowPeerFake
    implements XSystemDependentWindowPeer, XWindowPeer
{

    protected long hWindow;
    protected int localSystemType;

    /** Create the faked window peer.
     * @param _hWindow the system handle to the window.
     * @param _systemType specifies the system type.
     */
    public JavaWindowPeerFake(long _hWindow, int _systemType)
    {
        hWindow = _hWindow;
        localSystemType = _systemType;
    }

    /** <p>Implementation of XSystemDependentWindowPeer (that's all we really need)</p>
     *  This method is called back from the OpenOffice.org toolkit to retrieve the system data.
     */
    public Object getWindowHandle(/*IN*/byte[] ProcessId, /*IN*/short SystemType)
        throws com.sun.star.uno.RuntimeException
    {

        if (SystemType == localSystemType) {
            return new Integer((int)hWindow);
        }
        else return null;
    }

    /** not really neaded.
     */
    public XToolkit getToolkit()
        throws com.sun.star.uno.RuntimeException
    {
        return null;
    }

    /** not really neaded.
     */
    public void setPointer(/*IN*/XPointer Pointer)
        throws com.sun.star.uno.RuntimeException
    {
    }

    /** not really neaded.
     */
    public void setBackground(/*IN*/int Color)
        throws com.sun.star.uno.RuntimeException
    {
    }

    /** not really neaded.
     */
    public void invalidate(/*IN*/short Flags)
        throws com.sun.star.uno.RuntimeException
    {
    }

    /** not really neaded.
     */
    public void invalidateRect(/*IN*/com.sun.star.awt.Rectangle Rect, /*IN*/short Flags)
        throws com.sun.star.uno.RuntimeException
    {
    }

    /** not really neaded.
     */
    public void dispose()
        throws com.sun.star.uno.RuntimeException
    {
    }

    /** not really neaded.
     */
    public void addEventListener(/*IN*/com.sun.star.lang.XEventListener xListener)
        throws com.sun.star.uno.RuntimeException
    {
    }

    /** not really neaded.
     */
    public void removeEventListener(/*IN*/com.sun.star.lang.XEventListener aListener)
        throws com.sun.star.uno.RuntimeException
    {
    }
}

