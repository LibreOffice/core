/*************************************************************************
 *
 *  $RCSfile: _XModuleManager.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2004-02-25 18:10:45 $
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

package ifc.frame;

import com.sun.star.beans.PropertyValue;

import drafts.com.sun.star.frame.XModuleManager;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.lang.IllegalArgumentException;
import drafts.com.sun.star.frame.UnknownModuleException;


/**
* Testing <code>com.sun.star.frame.XModuleManager</code>
* interface methods:
* <ul>
*  <li><code> identify() </code></li>
* </ul><p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XModuleManager.XFrame'</code> (of type <code>PropertyValue[]</code>):
*  PropertyValue[n].Value : a XFrame
*  PropertyValue[n].Name : the expected return value of <code>idendify()</code></li>
*  <li> <code>'XModuleManager.XController'</code> (of type <code>PropertyValue[]</code>):
*  PropertyValue[n].Value : a XController
*  PropertyValue[n].Name : the expected return value of <code>idendify()</code></li>
*  <li> <code>'XModuleManager.XModel'</code> (of type <code>PropertyValue[]</code>):
*  PropertyValue[n].Value : a XFrame
*  PropertyValue[n].Name : the expected return value of <code>idendify()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.frame.XModuleManager
*/
public class _XModuleManager extends MultiMethodTest {
    /** Test calls the method. <p>
     * The onject relations <CODE>XModuleManager.XFrame</CODE>,
     * <CODE>XModuleManager.XController</CODE> and <CODE>XModuleManager.XModel</CODE>
     * are sequenzes of <CODE>PropertyValue</CODE>. The value of a PropertyValue
     * containes a <CODE>XFrame</CODE>, <CODE>XController</CODE> or a
     * <CODE>XModel</CODE>. The name of the PropertyValue contains the expected return
     * value of method <CODE>indetify()</CODE> if the method was called with
     * coresponding value.<p>
     * As enhancement the method <CODE>identify()</CODE> was called with incvalid
     * parameter. In this case the thrown exceptions was catched.
     */
    public XModuleManager oObj = null;
    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method returns expected values, that's equal to
    * previously obtained object relation 'Frame'.
    * The following method tests are to be completed successfully before:
    * <ul>
    *  <li> <code> attachFrame() </code> : attachs frame obtained object
    * relation 'Frame' </li>
    * </ul>
    */

    private PropertyValue[] xFrameSeq = null;
    private PropertyValue[] xControllerSeq = null;
    private PropertyValue[] xModelSeq = null;
    /** Retrieves object relations. */

    public void before() {

        xFrameSeq = (PropertyValue[]) tEnv.getObjRelation("XModuleManager.XFrame") ;

        if (xFrameSeq == null) throw new StatusException
            (Status.failed("Relation 'xFrameSeq' not found.")) ;


        xControllerSeq = (PropertyValue[]) tEnv.getObjRelation("XModuleManager.XController") ;

        if (xControllerSeq == null) throw new StatusException
            (Status.failed("Relation 'xControllerSeq' not found.")) ;


        xModelSeq = (PropertyValue[]) tEnv.getObjRelation("XModuleManager.XModel") ;

        if (xModelSeq == null) throw new StatusException
            (Status.failed("Relation 'xModelSeq' not found.")) ;
    }

    /** The method <CODE>identify()</CODE> was tesed for every entry in sequeze of
     * object relations.
     */
    public void _identify() {
        boolean ok = true;
        log.println("testing frame sequenze...");
        ok &= testSequenze(xFrameSeq);
        log.println("testing controller sequenze...");
        ok &= testSequenze(xControllerSeq);
        log.println("testing model sequenze...");
        ok &= testSequenze(xModelSeq);
        tRes.tested("identify()", ok);

        log.println("testing invalid objects...");
        try{
            oObj.identify(oObj);
        } catch (IllegalArgumentException e){
            log.println("expected exception.");
        } catch (UnknownModuleException e){
            log.println("expected exception.");
        }
    }

    private boolean testSequenze(PropertyValue[] sequenze){
        boolean ok = true;
        for (int i = 0 ; i < sequenze.length; i++){
            try{
                log.println("testing '" + sequenze[i].Name + "'");
                if (oObj.identify(sequenze[i].Value).equals(
                                                        sequenze[i].Name)){
                     ok &= ok;
                }else{
                     log.println("failure: returned value: '" +
                                 oObj.identify(sequenze[i].Value) +
                                 "' ,expected value: '" + sequenze[i].Name + "'");
                     ok = false;
                }
            } catch (IllegalArgumentException e){
                log.println("Could not get value of sequenze '" +
                            sequenze[i].Name + "'");
                return false;

            } catch (UnknownModuleException e){
                log.println("Could not indetify value of sequenze '" +
                            sequenze[i].Name + "'");
                return false;
            }
        }
        return ok;
    }

}

