/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/




/** <p>Class to pass the system window handle to the OpenOffice.org toolkit.</p>
 */
class JavaWindowPeerFake implements com.sun.star.awt.XSystemDependentWindowPeer,
                                    com.sun.star.awt.XWindowPeer
{
    private final NativeView maView;

    public JavaWindowPeerFake(NativeView aNative)
    {
            maView = aNative;
    }


    /**
     * Implementation of XSystemDependentWindowPeer (that's all we really need).
     * This method is called back from the Office toolkit to retrieve the system data.
     */
    public java.lang.Object getWindowHandle(byte[] aProcessId, short aSystem)
        throws com.sun.star.uno.RuntimeException
    {
        Object aReturn = null;
        if(aSystem==maView.maSystem)
                aReturn = maView.maHandle;
        return aReturn;
    }

    /** not really needed.
     */
    public com.sun.star.awt.XToolkit getToolkit()
        throws com.sun.star.uno.RuntimeException
    {
        return null;
    }

    public void setPointer(com.sun.star.awt.XPointer xPointer)
        throws com.sun.star.uno.RuntimeException
    {
    }

    public void setBackground(int nColor)
        throws com.sun.star.uno.RuntimeException
    {
    }

    public void invalidate(short nFlags)
        throws com.sun.star.uno.RuntimeException
    {
    }

    public void invalidateRect(com.sun.star.awt.Rectangle aRect,short nFlags)
        throws com.sun.star.uno.RuntimeException
    {
    }

    public void dispose()
        throws com.sun.star.uno.RuntimeException
    {
    }

    public void addEventListener(com.sun.star.lang.XEventListener xListener)
        throws com.sun.star.uno.RuntimeException
    {
    }

    public void removeEventListener(com.sun.star.lang.XEventListener xListener)
        throws com.sun.star.uno.RuntimeException
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
