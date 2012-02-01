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
import com.sun.star.lang.XMultiServiceFactory;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.awt.XControlModel;

import com.sun.star.beans.PropertyValue;

import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;

import com.sun.star.frame.XModel;
import com.sun.star.frame.XController;

import com.sun.star.view.XSelectionSupplier;


// __________ Implementation __________

/** ControlAndSelectDemo
    @author Sven Jacobi

   A (GroupBox) ControlShape will be created.
   Finally the ControlShape will be inserted into a selection.
*/

public class ControlAndSelectDemo
{
    public static void main( String args[] )
    {
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

            xComponent = Helper.createDocument( xOfficeContext,
                "private:factory/sdraw", "_blank", 0, pPropValues );

            XMultiServiceFactory xFactory =
                (XMultiServiceFactory )UnoRuntime.queryInterface(
                    XMultiServiceFactory.class, xComponent );

            XDrawPagesSupplier xDrawPagesSupplier =
                (XDrawPagesSupplier)UnoRuntime.queryInterface(
                    XDrawPagesSupplier.class, xComponent );
            XDrawPages xDrawPages = xDrawPagesSupplier.getDrawPages();
            XDrawPage xDrawPage = (XDrawPage)UnoRuntime.queryInterface(
                XDrawPage.class, xDrawPages.getByIndex( 0 ));
            XShapes xShapes = (XShapes)UnoRuntime.queryInterface(XShapes.class,
                                                                 xDrawPage );


            // create and insert the ControlShape
            Object xObj = xFactory.createInstance(
                "com.sun.star.drawing.ControlShape" );
            XShape xShape = (XShape)UnoRuntime.queryInterface( XShape.class, xObj );
            xShape.setPosition( new Point( 1000, 1000 ) );
            xShape.setSize( new Size( 2000, 2000 ) );
            xShapes.add( xShape );

            // create and set the control
            XControlModel xControlModel = (XControlModel)UnoRuntime.queryInterface(
                XControlModel.class,
                xFactory.createInstance( "com.sun.star.form.component.GroupBox" ) );
            XControlShape xControlShape = (XControlShape)UnoRuntime.queryInterface(
                XControlShape.class, xShape );
            xControlShape.setControl( xControlModel );


            // the following code will demonstrate how to
            // make a selection that contains our new created ControlShape
            XModel xModel = (XModel)UnoRuntime.queryInterface( XModel.class,
                                                               xComponent );
            XController xController = xModel.getCurrentController();
            XSelectionSupplier xSelectionSupplier =(XSelectionSupplier)
                UnoRuntime.queryInterface( XSelectionSupplier.class, xController );
            // take care to use the global service factory only and not the one
            // that is provided by the component if you create the ShapeColletion
            XShapes xSelection = (XShapes)UnoRuntime.queryInterface( XShapes.class,
                xOfficeContext.getServiceManager().createInstanceWithContext(
                    "com.sun.star.drawing.ShapeCollection", xOfficeContext ) );
            xSelection.add( xShape );
            xSelectionSupplier.select( xSelection );
        }
        catch( java.lang.Exception ex )
        {
            System.out.println( ex );
        }
        System.exit( 0 );
    }
}
