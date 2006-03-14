/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XObjectInspector.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2006-03-14 11:49:31 $
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

package ifc.inspection;

import com.sun.star.inspection.XObjectInspector;
import com.sun.star.inspection.XObjectInspectorModel;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;


/**
 * Testing <code>com.sun.star.inspection.XObjectInspector</code>
 * interface methods :
 * <ul>
 *  <li><code> inspect()</code></li>
 *  <li><code> InspectorModel()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 *
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'XObjectInspector.toInspect'</code>
 *  (of type <code>Object []</code>):
 *   acceptable collection of one or more objects which can be inspected by <code>inspect()</code> </li>
 * <ul> <p>
 *
 */
public class _XObjectInspector extends MultiMethodTest {

    /**
     * the test object
     */
    public XObjectInspector oObj = null;
    /**
     * This variable was filled with the object relation
     * <CODE>XObjectInspector.toInspect</CODE> and was used to
     * test the method <CODE>inspect()</CODE>
     */
    public Object[] oInspect = null;
    /**
     * In the test of method <CODE>getInspectorModel()</CODE> this variable
     * was assigned to the current <CODE>XModel</CODE>.
     */
    public XObjectInspectorModel xGetModel = null;
    /**
     * This variable was filled with the object relation
     * <CODE>XObjectInspector.InspectorModelToSet</CODE> and was used to
     * test the method <CODE>setInspectorModel()</CODE>
     */
    public XObjectInspectorModel xSetModel = null;

    /**
     * get object relations
     * <ul>
     *   <li>XObjectInspector.toInspect</li>
     * </ul>
     */
    public void before() {

        oInspect = (Object[]) tEnv.getObjRelation("XObjectInspector.toInspect");

        if (oInspect == null) throw new StatusException
                (Status.failed("Relation 'XObjectInspector.toInspect' not found.")) ;

        xSetModel = (XObjectInspectorModel) tEnv.getObjRelation("XObjectInspector.InspectorModelToSet");

        if (xSetModel == null) throw new StatusException
                (Status.failed("Relation 'XObjectInspector.InspectorModelToSet' not found.")) ;
    }

    /**
     * Inspects a new collection of one or more objects given by object realtion
     * <CODE>XObjectInspector.toInspect</CODE><br>
     * Has <b>OK</b> status if no runtime exceptions occured.
     */
    public void _inspect() {

        boolean result = true;

        try {
            oObj.inspect(oInspect);

        } catch (com.sun.star.util.VetoException e){
            log.println("ERROR:" + e.toString());
            result = false;
        }

        tRes.tested("inspect()", result) ;
    }

    /**
     * Call the method <CODE>getInspectorModel()</CODE> and assign the value to the
     * module variable <CODE>xGetModel</CODE>.<br>
     * Has <CODE>OK</CODE> status if the returned value is not null.
     */
    public void _getInspectorModel() {

        xGetModel = oObj.getInspectorModel() ;
        tRes.tested("getInspectorModel()", xGetModel != null) ;
    }

    /**
     * Call the method <CODE>setInspectorModel()</CODE> with the module variable
     * <CODE>xSetModel</CODE> as parameter.<br> Then <CODE>getInspectorModel()</CODE>
     * was called and the returned valued was compared to the module variable
     * <CODE>xSetModel</CODE><br>
     * Has <CODE>OK</CODE> status if the returned value is equal to
     * <CODE>xSetModel</CODE>.
     * The following method test has to be completed successfully before :
     * <ul>
     *    <li> <code> getInspectorModel() </code></li>
     * </ul>
     */
    public void _setInspectorModel() {
        requiredMethod("getInspectorModel()");

        oObj.setInspectorModel(xSetModel);

        XObjectInspectorModel xNewModel = oObj.getInspectorModel();

        oObj.setInspectorModel(xGetModel);

        tRes.tested("setInspectorModel()", xSetModel.equals(xNewModel)) ;
    }

}


