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



// __________ Imports __________

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XComponent;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;

import com.sun.star.container.XIndexAccess;

import com.sun.star.document.XViewDataSupplier;

import com.sun.star.frame.XModel;
import com.sun.star.frame.XController;



// __________ Implementation __________

/** text demo
    @author Sven Jacobi
 */

public class DrawViewDemo
{
    public static void main( String args[] )
    {
        if ( args.length < 1 )
        {
            System.out.println( "usage: DrawViewDemo SourceURL" );
            System.exit(1);
        }

        XComponent xComponent = null;
        try
        {
            // get the remote office context of a running office (a new office
            // instance is started if necessary)
            com.sun.star.uno.XComponentContext xOfficeContext = Helper.connect();

            // suppress Presentation Autopilot when opening the document
            // properties are the same as described for
            // com.sun.star.document.MediaDescriptor
            PropertyValue[] pPropValues = new PropertyValue[ 1 ];
            pPropValues[ 0 ] = new PropertyValue();
            pPropValues[ 0 ].Name = "Silent";
            pPropValues[ 0 ].Value = new Boolean( true );

            java.io.File sourceFile = new java.io.File(args[0]);
            StringBuffer sUrl = new StringBuffer("file:///");
            sUrl.append(sourceFile.getCanonicalPath().replace('\\', '/'));

            xComponent = Helper.createDocument( xOfficeContext,
                                                sUrl.toString(), "_blank", 0,
                                                pPropValues );
            XModel xModel =
                (XModel)UnoRuntime.queryInterface(
                    XModel.class, xComponent );


            // print all available properties of first view
            System.out.println("*** print all available properties of first view");
            XViewDataSupplier xViewDataSupplier =
                (XViewDataSupplier)UnoRuntime.queryInterface(
                    XViewDataSupplier.class, xModel );
            XIndexAccess xIndexAccess = xViewDataSupplier.getViewData();
            if ( xIndexAccess.getCount() != 0 )
            {
                PropertyValue[] aPropSeq = (PropertyValue[])
                    xIndexAccess.getByIndex( 0 );

                for( int i = 0; i < aPropSeq.length; i++ )
                {
                    System.out.println( aPropSeq[ i ].Name + " = " +
                                        aPropSeq[ i ].Value );
                }
            }


            // print all properties that are supported by the controller
            // and change into masterpage mode
            System.out.println("*** print all properties that are supported by the controller");
            XController xController = xModel.getCurrentController();
            XPropertySet xPropSet =
                (XPropertySet)UnoRuntime.queryInterface(
                    XPropertySet.class, xController );
            XPropertySetInfo xPropSetInfo = xPropSet.getPropertySetInfo();
            Property[] aPropSeq = xPropSetInfo.getProperties();
            for( int i = 0; i < aPropSeq.length; i++ )
            {
                System.out.println( aPropSeq[ i ].Name );
            }
            System.out.println("*** change into masterpage mode");
            xPropSet.setPropertyValue( "IsMasterPageMode", new Boolean( true ) );

        }
        catch( Exception ex )
        {
            System.out.println( ex.getMessage() );
            ex.printStackTrace(System.out);
        }

        System.exit( 0 );
    }
}
