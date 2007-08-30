/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JavaWindowPeerFake.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-08-30 13:57:57 $
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

package com.sun.star.comp.beans;

import java.awt.*;

import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.awt.*;
import com.sun.star.util.*;
import com.sun.star.beans.*;
import com.sun.star.container.*;

/** <p>Class to pass the system window handle to the OpenOffice.org toolkit.</p>
 *
 * @since OOo 2.0.0
 */
/* package */ class JavaWindowPeerFake
    implements XSystemDependentWindowPeer, XWindowPeer
{
     protected int localSystemType;
    protected Any wrappedHandle;

    /** Create the faked window peer.
     * @param _hWindow the system handle to the window.
     * @param _systemType specifies the system type.
     */
    public JavaWindowPeerFake(Any _hWindow, int _systemType)
    {
        localSystemType = _systemType;
        wrappedHandle = _hWindow;
    }

    /** <p>Implementation of XSystemDependentWindowPeer (that's all we really need)</p>
     *  This method is called back from the OpenOffice.org toolkit to retrieve the system data.
     */
    public Object getWindowHandle(/*IN*/byte[] ProcessId, /*IN*/short SystemType)
        throws com.sun.star.uno.RuntimeException
    {
        if (SystemType == localSystemType) {
            return wrappedHandle;
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

