/*************************************************************************
 *
 *  $RCSfile: _MailMerge.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2004-07-23 10:46:21 $
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

package ifc.text;

import lib.MultiPropertyTest;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.uno.UnoRuntime;

public class _MailMerge extends MultiPropertyTest {

    /**
     * Custom tester for properties which contains URLs.
     * Switches between two valid folders
     */
    protected PropertyTester URLTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            if (oldValue.equals(util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF())))
                return util.utils.getFullTestURL(""); else
                return util.utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF());
        }
    } ;

    /**
     * Custom tester for properties which contains document URLs.
     * Switches between two document URLs.
     */
    protected PropertyTester DocumentURLTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            if (oldValue.equals(util.utils.getFullTestURL("MailMerge.sxw")))
                return util.utils.getFullTestURL("sForm.sxw"); else
                return util.utils.getFullTestURL("MailMerge.sxw");
        }
    } ;
    /**
     * Tested with custom property tester.
     */
    public void _ResultSet() {
        String propName = "ResultSet";
        try{

            log.println("try to get value from property...");
            XResultSet oldValue = (XResultSet) UnoRuntime.queryInterface(XResultSet.class,oObj.getPropertyValue(propName));

            log.println("try to get value from object relation...");
            XResultSet newValue = (XResultSet) UnoRuntime.queryInterface(XResultSet.class,tEnv.getObjRelation("MailMerge.XResultSet"));

            log.println("set property to a new value...");
            oObj.setPropertyValue(propName, newValue);

            log.println("get the new value...");
            XResultSet getValue = (XResultSet) UnoRuntime.queryInterface(XResultSet.class,oObj.getPropertyValue(propName));

            tRes.tested(propName, this.compare(newValue, getValue));
        } catch (com.sun.star.beans.PropertyVetoException e){
            log.println("could not set property '"+ propName +"' to a new value!");
            tRes.tested(propName, false);
        } catch (com.sun.star.lang.IllegalArgumentException e){
            log.println("could not set property '"+ propName +"' to a new value!");
            tRes.tested(propName, false);
        } catch (com.sun.star.beans.UnknownPropertyException e){
            if (this.isOptional(propName)){
                    // skipping optional property test
                    log.println("Property '" + propName
                            + "' is optional and not supported");
                    tRes.tested(propName,true);

            } else {
                log.println("could not get property '"+ propName +"' from XPropertySet!");
                tRes.tested(propName, false);
            }
        } catch (com.sun.star.lang.WrappedTargetException e){
            log.println("could not get property '"+ propName +"' from XPropertySet!");
            tRes.tested(propName, false);
        }
    }

    /**
     * Tested with custom property tester.
     */

    public void _ActiveConnection() {
        String propName = "ActiveConnection";
        try{

            log.println("try to get value from property...");
            XConnection oldValue = (XConnection) UnoRuntime.queryInterface(XConnection.class,oObj.getPropertyValue(propName));

            log.println("try to get value from object relation...");
            XConnection newValue = (XConnection) UnoRuntime.queryInterface(XConnection.class,tEnv.getObjRelation("MailMerge.XConnection"));

            log.println("set property to a new value...");
            oObj.setPropertyValue(propName, newValue);

            log.println("get the new value...");
            XConnection getValue = (XConnection) UnoRuntime.queryInterface(XConnection.class,oObj.getPropertyValue(propName));

            tRes.tested(propName, this.compare(newValue, getValue));
        } catch (com.sun.star.beans.PropertyVetoException e){
            log.println("could not set property '"+ propName +"' to a new value! " + e.toString());
            tRes.tested(propName, false);
        } catch (com.sun.star.lang.IllegalArgumentException e){
            log.println("could not set property '"+ propName +"' to a new value! " + e.toString());
            tRes.tested(propName, false);
        } catch (com.sun.star.beans.UnknownPropertyException e){
            if (this.isOptional(propName)){
                    // skipping optional property test
                    log.println("Property '" + propName
                            + "' is optional and not supported");
                    tRes.tested(propName,true);

            } else {
                log.println("could not get property '"+ propName +"' from XPropertySet!");
                tRes.tested(propName, false);
            }
        } catch (com.sun.star.lang.WrappedTargetException e){
            log.println("could not get property '"+ propName +"' from XPropertySet!");
            tRes.tested(propName, false);
        }
    }

    /**
     * Tested with custom property tester.
     */
    public void _DocumentURL() {
        log.println("Testing with custom Property tester") ;
        testProperty("DocumentURL", DocumentURLTester) ;
    }

    /**
     * Tested with custom property tester.
     */
    public void _OutputURL() {
        log.println("Testing with custom Property tester") ;
        testProperty("OutputURL", URLTester) ;
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }


} //finish class _MailMerge

