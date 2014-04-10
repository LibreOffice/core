/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
        XServiceInfo xInfo = UnoRuntime.queryInterface(XServiceInfo.class, oObj );

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
                    "' changes its value : Failed !" );
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


