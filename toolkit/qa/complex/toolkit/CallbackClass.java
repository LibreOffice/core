/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CallbackClass.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-27 11:42:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

package complex.toolkit;

import com.sun.star.awt.XCallback;
import lib.MultiMethodTest;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import share.LogWriter;

/**
 * Testing <code>com.sun.star.awt.XRequestCallback</code>
 * interface methods :
 * <ul>
 *  <li><code> addCallback()</code></li>
 * </ul> <p>
 * @see com.sun.star.awt.XRequestCallback
 */
public class CallbackClass implements XCallback{

    private LogWriter log;

    private XMultiServiceFactory xMSF;


    public CallbackClass(LogWriter log, XMultiServiceFactory xMSF ) {

        this.xMSF = xMSF;
        this.log = log;
    }


    /**
     * Callback method which will be called by the asynchronous
     * service where we have added our request before.
     */
    public void notify( Object aData ) {

        log.println("callback called successfully" );
    }
}
