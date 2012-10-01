/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

// __________ Imports __________

import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XComponent;

import com.sun.star.beans.PropertyValue;

import com.sun.star.document.XExporter;
import com.sun.star.document.XFilter;

import com.sun.star.drawing.XDrawPage;

// __________ Implementation __________

// text demo

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
            java.net.URL destUrl = destFile.toURI().toURL();

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

