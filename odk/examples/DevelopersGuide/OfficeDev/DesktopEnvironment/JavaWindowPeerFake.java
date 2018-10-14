/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



import java.awt.*;


/** <p>Class to pass the system window handle to the OpenOffice toolkit.</p>
 */
class JavaWindowPeerFake implements com.sun.star.awt.XSystemDependentWindowPeer,
                                    com.sun.star.awt.XWindowPeer
{
    NativeView maView;

    public JavaWindowPeerFake(NativeView aNative)
    {
            maView = aNative;
    }

    // ____________________
    /**
     * Implementation of XSystemDependentWindowPeer (that's all we really need).
     * This method is called back from the OpenOffice toolkit to retrieve the system data.
     */
    public java.lang.Object getWindowHandle(byte[] aProcessId, short aSystem)
        throws com.sun.star.uno.RuntimeException
    {
        Object aReturn = null;
        if(aSystem==maView.maSystem)
                aReturn = (Object)maView.maHandle;
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

