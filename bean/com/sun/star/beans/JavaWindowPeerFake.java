/*************************************************************************
 *
 *  $RCSfile: JavaWindowPeerFake.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mi $ $Date: 2004-10-18 07:14:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.beans;

import java.awt.*;

import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.awt.*;
import com.sun.star.util.*;
import com.sun.star.beans.*;
import com.sun.star.container.*;

/** <p>Class to pass the system window handle to the OpenOffice.org toolkit.</p>
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

