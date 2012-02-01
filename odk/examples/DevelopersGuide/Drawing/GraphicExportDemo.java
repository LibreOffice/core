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

import com.sun.star.beans.PropertyValue;

import com.sun.star.document.XExporter;
import com.sun.star.document.XFilter;

import com.sun.star.drawing.XDrawPage;

// __________ Implementation __________

/** text demo
    @author Sven Jacobi
 */

public class GraphicExportDemo
{
    public static void main( String args[] )
    {
        if ( args.length < 3 )
        {
            System.out.println( "usage: GraphicExportDemo SourceURL DestinationURL PageIndexToExport" );
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

            Object GraphicExportFilter =
                xOfficeContext.getServiceManager().createInstanceWithContext(
                    "com.sun.star.drawing.GraphicExportFilter", xOfficeContext);
            XExporter xExporter = (XExporter)
                UnoRuntime.queryInterface( XExporter.class, GraphicExportFilter );

            PropertyValue aProps[] = new PropertyValue[2];
            aProps[0] = new PropertyValue();
            aProps[0].Name = "MediaType";
            aProps[0].Value = "image/gif";

            /* some graphics e.g. the Windows Metafile does not have a Media Type,
               for this case
               aProps[0].Name = "FilterName"; // it is possible to set a FilterName
               aProps[0].Value = "WMF";
            */
            java.io.File destFile = new java.io.File(args[1]);
            java.net.URL destUrl = destFile.toURL();

            aProps[1] = new PropertyValue();
            aProps[1].Name = "URL";
            aProps[1].Value = destUrl.toString();//args[ 1 ];

            int nPageIndex = Integer.parseInt( args[ 2 ] );
            if ( nPageIndex < PageHelper.getDrawPageCount( xComponent ) &&
                 nPageIndex > 1 )
            {
                XDrawPage xPage = PageHelper.getDrawPageByIndex( xComponent,
                                                                 nPageIndex );
                XComponent xComp = (XComponent)
                    UnoRuntime.queryInterface( XComponent.class, xPage );
                xExporter.setSourceDocument( xComp );
                XFilter xFilter = (XFilter)
                    UnoRuntime.queryInterface( XFilter.class, xExporter );
                xFilter.filter( aProps );
                System.out.println( "*** graphics on page \"" + nPageIndex
                                    + "\" from file \"" + args[0]
                                    + "\" exported under the name \""
                                    + args[1] + "\" in the local directory" );
            } else
            {
                System.out.println( "page index not in range" );
            }


            // close the document
            com.sun.star.util.XCloseable xCloseable = (com.sun.star.util.XCloseable)
                UnoRuntime.queryInterface(com.sun.star.util.XCloseable.class,
                                          xComponent);

            if (xCloseable != null )
                xCloseable.close(false);
            else
                xComponent.dispose();

            System.out.println("*** document closed!");

        }
        catch( Exception ex )
        {
            System.out.println( ex );
        }

        System.exit( 0 );
    }
}

