/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package ifc.sheet;

import lib.MultiMethodTest;
import util.ValueChanger;

import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.sheet.FunctionArgument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.sheet.FunctionDescription</code>
* service properties:
* <ul>
*   <li><code>Arguments</code></li>
*   <li><code>Category</code></li>
*   <li><code>Description</code></li>
*   <li><code>Id</code></li>
*   <li><code>Name</code></li>
* </ul> <p>
* @see com.sun.star.sheet.FunctionDescription
*/
public class _FunctionDescription extends MultiMethodTest {

    public XPropertySet oObj = null;    // oObj filled by MultiMethodTest

    public _FunctionDescription() {
    }

    public void _Arguments() {
        // check if Service is available
        XServiceInfo xInfo = (XServiceInfo)
            UnoRuntime.queryInterface(XServiceInfo.class, oObj );

        if ( ! xInfo.supportsService
                ( "com.sun.star.sheet.FunctionDescription" ) ) {
           log.println( "Service not available !" );
           tRes.tested( "Supported", false );
        }

        try {
            XMultiServiceFactory oDocMSF = (XMultiServiceFactory)tParam.getMSF();

            XInterface FA = (XInterface)oDocMSF.
                createInstance("com.sun.star.sheet.FunctionArgument");
            FunctionArgument arg = (FunctionArgument)AnyConverter.toObject
                (FunctionArgument.class, FA);

            arg.Description = "FunctionDescription argument description" ;
            arg.Name = "FunctionDescriptiuon argument name" ;
            arg.IsOptional = true ;

            Object sValue = oObj.getPropertyValue("Arguments") ;
            oObj.setPropertyValue("Arguments", new FunctionArgument[] {arg}) ;
            Object nValue = oObj.getPropertyValue("Arguments") ;

            if (sValue.equals(nValue)) {
                log.println("Property 'Arguments' didn't change: OK") ;
                tRes.tested("Arguments", true) ;
            } else {
                log.println("Readonly property 'Arguments' changed: Failed") ;
                tRes.tested("Arguments", false) ;
            }
        } catch (Exception e) {
            log.println(
                "Exception occurred while testing property 'Arguments'" );
            e.printStackTrace( log );
            tRes.tested( "Arguments", false );
        }
    }

    public void _Category() {
        tryChangeProp("Category") ;
    }

    public void _Description() {
        tryChangeProp( "Category" );
    }

    public void _Id() {
        tryChangeProp( "Id" );
    }

    public void _Name() {
        tryChangeProp( "Name" );
    }

    public void tryChangeProp( String name ) {

        Object gValue = null;
        Object sValue = null;
        Object ValueToSet = null;


        try {
            //waitForAllThreads();
            gValue = oObj.getPropertyValue( name );

            //waitForAllThreads();
            ValueToSet = ValueChanger.changePValue( gValue );
            //waitForAllThreads();
            oObj.setPropertyValue( name, ValueToSet );
            sValue = oObj.getPropertyValue( name );

            //check get-set methods
            if( gValue.equals( sValue ) ) {
                log.println( "Value for '"+name+"' hasn't changed. OK." );
                tRes.tested( name, true );
            }
            else {
               log.println( "Property '" + name +
                    "' changes it's value : Failed !" );
               tRes.tested( name, false );
            }
        }
        catch ( Exception e ) {
            log.println(
                "Exception occurred while testing property '" + name + "'" );
            e.printStackTrace( log );
            tRes.tested( name, false );
        }
    } // end of changeProp

} //finish class _TextContent


