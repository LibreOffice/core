/*************************************************************************
 *
 *  $RCSfile: ControlAndSelectDemo.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:20:24 $
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

// __________ Imports __________

// base classes
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.*;

// property access
import com.sun.star.beans.*;

// application specific classes
import com.sun.star.drawing.*;

// XModel, XController
import com.sun.star.frame.*;

// Point, Size
import com.sun.star.awt.*;

//
import com.sun.star.view.*;


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
            String sConnection;
            if ( args.length >= 1 )
                sConnection = args[ 0 ];
            else
                sConnection = "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager";
            XMultiServiceFactory xServiceFactory =
                Helper.connect( sConnection );

            // suppress Presentation Autopilot when opening the document
            // properties are the same as described for com.sun.star.document.MediaDescriptor
            PropertyValue[] pPropValues = new PropertyValue[ 1 ];
            pPropValues[ 0 ] = new PropertyValue();
            pPropValues[ 0 ].Name = "Silent";
            pPropValues[ 0 ].Value = new Boolean( true );

            xComponent = Helper.createDocument( xServiceFactory,
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
            XShapes xShapes = (XShapes)UnoRuntime.queryInterface( XShapes.class, xDrawPage );


            // create and insert the ControlShape
            Object xObj = xFactory.createInstance( "com.sun.star.drawing.ControlShape" );
            XShape xShape = (XShape)UnoRuntime.queryInterface( XShape.class, xObj );
            xShape.setPosition( new Point( 1000, 1000 ) );
            xShape.setSize( new Size( 2000, 2000 ) );
            xShapes.add( xShape );

            // create and set the control
            XControlModel xControlModel = (XControlModel)UnoRuntime.queryInterface( XControlModel.class,
                xFactory.createInstance( "com.sun.star.form.component.GroupBox" ) );
            XControlShape xControlShape = (XControlShape)UnoRuntime.queryInterface( XControlShape.class, xShape );
            xControlShape.setControl( xControlModel );


            // the following code will demonstrate how to
            // make a selection that contains our new created ControlShape
            XModel xModel = (XModel)UnoRuntime.queryInterface( XModel.class, xComponent );
            XController xController = xModel.getCurrentController();
            XSelectionSupplier xSelectionSupplier =(XSelectionSupplier)
                UnoRuntime.queryInterface( XSelectionSupplier.class, xController );
            // take care to use the global service factory only and not the one that is
            // provided by the component if you create the ShapeColletion
            XShapes xSelection = (XShapes)UnoRuntime.queryInterface( XShapes.class,
                xServiceFactory.createInstance( "com.sun.star.drawing.ShapeCollection" ) );
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
