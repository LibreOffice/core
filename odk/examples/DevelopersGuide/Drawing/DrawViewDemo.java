/*************************************************************************
 *
 *  $RCSfile: DrawViewDemo.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:20:40 $
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

// name access
import com.sun.star.container.*;

// application specific classes
import com.sun.star.drawing.*;

// XModel, XController
import com.sun.star.frame.*;

// XViewDataSupplier
import com.sun.star.document.*;


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
            System.out.println( "usage: DrawViewDemo SourceURL [ connection ]" );
        }
        else
        {
            XComponent xComponent = null;
            try
            {
                String sConnection;
                if ( args.length >= 2 )
                    sConnection = args[ 1 ];
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

                java.io.File sourceFile = new java.io.File(args[0]);
                StringBuffer sUrl = new StringBuffer("file:///");
                sUrl.append(sourceFile.getCanonicalPath().replace('\\', '/'));

                xComponent = Helper.createDocument( xServiceFactory,
                    sUrl.toString(), "_blank", 0, pPropValues );
                XModel xModel =
                    (XModel)UnoRuntime.queryInterface(
                        XModel.class, xComponent );


                // print all available properties of first view
                XViewDataSupplier xViewDataSupplier =
                    (XViewDataSupplier)UnoRuntime.queryInterface(
                        XViewDataSupplier.class, xModel );
                XIndexAccess xIndexAccess = xViewDataSupplier.getViewData();
                if ( xIndexAccess.getCount() != 0 )
                {
                  PropertyValue[] aPropSeq = (PropertyValue[])xIndexAccess.getByIndex( 0 );
                    for( int i = 0; i < aPropSeq.length; i++ )
                    {
                        System.out.println( aPropSeq[ i ].Name + " = " + aPropSeq[ i ].Value );
                    }
                }


                // print all properties that are supported by the controller
                // and change into masterpage mode
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
                xPropSet.setPropertyValue( "IsMasterPageMode", new Boolean( true ) );

            }
            catch( Exception ex )
            {
                System.out.println( ex );
            }
        }
        System.exit( 0 );
    }
}
