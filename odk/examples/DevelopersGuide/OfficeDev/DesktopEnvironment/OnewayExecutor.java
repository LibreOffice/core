/*************************************************************************
 *
 *  $RCSfile: OnewayExecutor.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:31:15 $
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

package OfficeDev.samples.DesktopEnvironment;

import java.util.Vector;

// __________ Implementation __________

/**
 * It's not allowed to call synchronoues back inside an oneway interface call.
 * (see IOnewayLink too). So we start a thread (implemented by this class), which
 * gets all neccessary parameters from the original called object and
 * call it back later inside his run() method. So the execution of such oneway call
 * will be asynchronous. It works in a generic way and can be used or any type
 * of oneway request. Because the source and the target of this call-link knows,
 * which method was used and which parameters must be handled.
 *
 * @author     Andreas Schl&uuml;ns
 * @created    17.07.2002 08:18
 */
class OnewayExecutor extends Thread
{
    // _______________________________

    /**
     * const
     * We define some request for some well known oneway interface
     * calls here too. So they mustn't be declared more then ones.
     * Of course it's not necessary to use it ... but why not :-)
     */

    public static final int     REQUEST_FRAMEACTION             = 1     ;
    public static final int     REQUEST_STATUSCHANGED           = 2     ;
    public static final int     REQUEST_ADDSTATUSLISTENER       = 3     ;
    public static final int     REQUEST_REMOVESTATUSLISTENER    = 4     ;
    public static final int     REQUEST_DISPATCH                = 5     ;

    public static final boolean ENCODE_PARAMS                   = true  ;
    public static final boolean DECODE_PARAMS                   = false ;

    // _______________________________

    /**
     * @member  m_rLink     the object, which wish to be called back by this thread
     * @member  m_nRequest  describes the type of the original request (means the called oneyway method)
     * @member  m_lParams   list of parameters of the original request
     */
    private IOnewayLink m_rLink     ;
    private int         m_nRequest  ;
    private Vector      m_lParams   ;

    // _______________________________

    /**
     * ctor
     * It's initialize this thread with all neccessary parameters.
     * It gets the object, which wish to be called back and the type
     * and parameters of the original request.
     *
     * @param nRequest
     *          The two user of this callback can define an unique number,
     *          which identify the type of original interface method.
     *          So the called interface object can decide, which action will be neccessary.
     *
     * @param lParams
     *          If the original method used parameters, they will be coded here
     *          in a generic way. Only the called interface object know (it depends from
     *          the original request - see nRequest too), how this list must be interpreted.
     *          Note: Atomic types (e.g. int, long) will be transported as objects too (Integer, Long)!
     */
    public OnewayExecutor( IOnewayLink rLink    ,
                           int         nRequest ,
                           Vector      lParams  )
    {
        m_rLink    = rLink   ;
        m_nRequest = nRequest;
        m_lParams  = lParams ;

        if (m_rLink==null)
            System.out.println("ctor ... m_rLink == null");
        if (m_lParams==null)
            System.out.println("ctor ... m_lParams == null");
    }

    // _______________________________

    /**
     * implements the thread function
     * Here we call the internal set link object back and
     * give him all neccessary parameters.
     * After that we die by ouerself ...
     */
    public void run()
    {
        if (m_rLink==null)
            System.out.println("run ... m_rLink == null");
        if (m_lParams==null)
            System.out.println("run ... m_lParams == null");

        if (m_rLink!=null)
            m_rLink.execOneway( m_nRequest, m_lParams );
    }

    // _______________________________

    /**
     * static helper!
     * To make convertion of the generic parameter list to the original
     * one easier - you can use this helper methods. They know how suchlist
     * must be coded. It's not a must to use it - but you can ...
     */
    public static void codeFrameAction(boolean bEncode, /*INOUT*/Vector[] lParams, /*INOUT*/com.sun.star.frame.FrameActionEvent[] aAction)
    {
        if (bEncode)
        {
            lParams[0] = new Vector(1);
            lParams[0].add( (Object)(aAction[0]) );
        }
        else
        {
            aAction[0] = (com.sun.star.frame.FrameActionEvent)(lParams[0].elementAt(0));
        }
    }

    // _______________________________

    public static void codeStatusChanged(boolean bEncode, /*INOUT*/Vector[] lParams, /*INOUT*/com.sun.star.frame.FeatureStateEvent[] aStatus)
    {
        if (bEncode)
        {
            lParams[0] = new Vector(1);
            lParams[0].add( (Object)aStatus[0] );
        }
        else
        {
            aStatus[0] = (com.sun.star.frame.FeatureStateEvent)(lParams[0].elementAt(0));
        }
    }

    // _______________________________

    public static void codeAddOrRemoveStatusListener(boolean bEncode, /*INOUT*/Vector[] lParams, /*INOUT*/com.sun.star.frame.XStatusListener[] xListener, /*INOUT*/com.sun.star.util.URL[] aURL)
    {
        if (bEncode)
        {
            lParams[0] = new Vector(2);
            lParams[0].add( (Object)xListener[0] );
            lParams[0].add( (Object)aURL[0]      );
        }
        else
        {
            xListener[0] = (com.sun.star.frame.XStatusListener)(lParams[0].elementAt(0));
            aURL[0]      = (com.sun.star.util.URL)(lParams[0].elementAt(1));
        }
    }

    // _______________________________

    public static void codeDispatch(boolean bEncode, /*INOUT*/Vector[] lParams, /*INOUT*/com.sun.star.util.URL[] aURL, /*INOUT*/com.sun.star.beans.PropertyValue[][] lArgs)
    {
        if (bEncode)
        {
            int nLength = lArgs.length+1;
            int nPos    = 0;
            lParams[0] = new Vector(nLength);

            lParams[0].add( (Object)aURL[0] );
            --nLength;

            while (nLength>0)
            {
                lParams[0].add( (Object)lArgs[0][nPos] );
                --nLength;
                ++nPos   ;
            }
        }
        else
        {
            int nLength = lParams[0].size()-1;
            int nPos    = 0;

            lArgs[0] = new com.sun.star.beans.PropertyValue[nLength];
            aURL[0]  = (com.sun.star.util.URL)(lParams[0].elementAt(0));

            while (nPos<nLength)
            {
                lArgs[0][nPos] = (com.sun.star.beans.PropertyValue)(lParams[0].elementAt(nPos+1));
                ++nPos;
            }
        }
    }
}
