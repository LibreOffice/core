/*************************************************************************
 *
 *  $RCSfile: _XSimpleMailClientSupplier.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:09:04 $
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

package ifc.system;

import lib.MultiMethodTest;
import lib.Status;

import com.sun.star.system.XSimpleMailClient;
import com.sun.star.system.XSimpleMailClientSupplier;


/**
* Testing <code>com.sun.star.system.XSimpleMailClientSupplier</code>
* interface methods :
* <ul>
*  <li><code> querySimpleMailClient()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'SystemMailExist'</code> (of type <code>Boolean</code>):
*   if <code>true</code> then <code>SimpleSystemMail</code> service
*   is available, if NOT then the service is not availabel or
*   OS doesn't has system mail.</li>
* <ul> <p>
* @see com.sun.star.system.XSimpleMailClientSupplier
*/
public class _XSimpleMailClientSupplier extends MultiMethodTest {

    public XSimpleMailClientSupplier oObj = null;

    /**
    * Test calls the method if the <code>SimpleSystemMail</code>
    * service is available on current OS. <p>
    * Has <b> OK </b> status if not null value returned or
    * has <b>SKIPPED.OK</b> status if SystemMail is not available. <p>
    */
    public void _querySimpleMailClient() {
        if (Boolean.TRUE.equals
            ((Boolean)tEnv.getObjRelation("SystemMailExist"))) {

            XSimpleMailClient aClient = oObj.querySimpleMailClient();
            tRes.tested("querySimpleMailClient()",aClient != null);
        } else {
            log.println("SystemMail doesn't exist : nothing to test") ;
            tRes.tested("querySimpleMailClient()", Status.skipped(true)) ;
        }
    }


}  // finish class _XSimpleMailClientSupplier


