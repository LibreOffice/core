/*************************************************************************
 *
 *  $RCSfile: GraphicExportDemo.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:21:13 $
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
