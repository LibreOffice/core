/*************************************************************************
 *
 *  $RCSfile: GraphicExportDemo.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-06-30 15:23:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright (c) 2003 by Sun Microsystems, Inc.
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

// base classes
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.*;

// property access
import com.sun.star.beans.*;

// name access
import com.sun.star.container.*;

// application specific classes
import com.sun.star.drawing.*;

// XExporter
import com.sun.star.document.XExporter;
import com.sun.star.document.XFilter;


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
            System.out.println( "usage: GraphicExportDemo SourceURL DestinationURL PageIndexToExport [ connection ]" );
        }
        else
        {
            XComponent xComponent = null;
            try
            {
                String sConnection;
                if ( args.length >= 4 )
                    sConnection = args[ 3 ];
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

                Object GraphicExportFilter = xServiceFactory.createInstance
                    ("com.sun.star.drawing.GraphicExportFilter");
                XExporter xExporter = (XExporter)
                    UnoRuntime.queryInterface( XExporter.class, GraphicExportFilter );

                PropertyValue aProps[] = new PropertyValue[2];
                aProps[0] = new PropertyValue();
                aProps[0].Name = "MediaType";
                aProps[0].Value = "image/gif";

/* some graphics e.g. the Windows Metafile does not have a Media Type, for this case
                aProps[0].Name = "FilterName";  // it is possible to set a FilterName
                aProps[0].Value = "WMF";
*/
                java.io.File destFile = new java.io.File(args[1]);
                StringBuffer destUrl = new StringBuffer("file:///");
                destUrl.append(destFile.getCanonicalPath().replace('\\', '/'));

                aProps[1] = new PropertyValue();
                aProps[1].Name = "URL";
                aProps[1].Value = destUrl.toString();//args[ 1 ];

                int nPageIndex = Integer.parseInt( args[ 2 ] );
                if ( nPageIndex < PageHelper.getDrawPageCount( xComponent ) &&
                   nPageIndex > 1 )
                {
                    XDrawPage xPage = PageHelper.getDrawPageByIndex( xComponent, nPageIndex );
                    XComponent xComp = (XComponent)
                        UnoRuntime.queryInterface( XComponent.class, xPage );
                    xExporter.setSourceDocument( xComp );
                    XFilter xFilter = (XFilter)
                        UnoRuntime.queryInterface( XFilter.class, xExporter );
                    xFilter.filter( aProps );
                    System.out.println( "*** graphics on page \"" + nPageIndex + "\" from file \"" +
                                        args[0] + "\" exported under the name \""
                                        + args[1] + "\" in the local directory" );
                }
                else
                {
                    System.out.println( "page index not in range" );
                }

                System.out.println( "Press return to close the document \"" + args[0]+"\" and terminate!" );
                while( System.in.read() != 10 );

                // close the document
                xComponent.dispose();

            }
            catch( Exception ex )
            {
                System.out.println( ex );
            }
        }
        System.exit( 0 );
    }
}
