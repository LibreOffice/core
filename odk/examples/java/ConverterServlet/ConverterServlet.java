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

// JDK API
import java.io.IOException;
import java.io.PrintWriter;
import java.io.File;
import java.util.Enumeration;

// Servlet API
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
// Helper API
import com.oreilly.servlet.MultipartRequest;
import com.oreilly.servlet.MultipartResponse;
import com.oreilly.servlet.ServletUtils;

// UNO API
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XStorable;
import com.sun.star.util.XCloseable;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;


/** This class implements a http servlet in order to convert an incoming document
 * with help of a running OpenOffice.org and to push the converted file back
 * to the client.
 */
public class ConverterServlet extends HttpServlet {
    /** Specifies the temporary directory on the web server.
     */
    private String stringWorkingDirectory =
    System.getProperty( "java.io.tmpdir" ).replace( '\\', '/' );

    /** Specifies the host for the office server.
     */
    private String stringHost = "localhost";

    /** Specifies the port for the office server.
     */
    private String stringPort = "2083";

    /** Called by the server (via the service method) to allow a servlet to handle
     * a POST request. The file from the client will be uploaded to the web server
     * and converted on the web server and after all pushed to the client.
     * @param request Object that contains the request the client has made of the servlet.
     * @param response Object that contains the response the servlet sends to the client.
     * @throws ServletException If the request for the POST could not be handled.
     * @throws IOException If an input or output error is detected when the servlet handles the request.
     */
    protected void doPost( HttpServletRequest request,
                           HttpServletResponse response) throws ServletException, java.io.IOException {
        try {
            // If necessary, add a slash to the end of the string.
            if ( !stringWorkingDirectory.endsWith( "/" ) ) {
                stringWorkingDirectory += "/";
            }

            // Construct a MultipartRequest to help read the information.
            // Pass in the request, a directory to save files to, and the
            // maximum POST size we should attempt to handle.
            MultipartRequest multipartrequest =
                new MultipartRequest( request, stringWorkingDirectory, 5 * 1024 * 1024 );

            // Getting all file names from the request
            Enumeration files = multipartrequest.getFileNames();

            // Every received file will be converted to the specified type
            while (files.hasMoreElements()) {
                // Getting the name from the element
                String stringName = (String)files.nextElement();

                // Getting the filename from the request
                String stringFilename =
                    multipartrequest.getFilesystemName( stringName );

                // Converting the given file on the server to the specified type and
                // append a special extension
                File cleanupFile = null;
                String stringSourceFile = stringWorkingDirectory + stringFilename;

                try {
                    String stringConvertedFile = convertDocument(stringSourceFile,
                        multipartrequest.getParameter( "converttype" ),
                        multipartrequest.getParameter( "extension" ));

                    String shortFileName = stringConvertedFile.substring(
                        stringConvertedFile.lastIndexOf('/') + 1);

                    // Set the response header
                    // Set the filename, is used when the file will be saved (problem with mozilla)
                    response.addHeader( "Content-Disposition",
                                        "attachment; filename=" + shortFileName);

                    // Constructing the multi part response to the client
                    MultipartResponse multipartresponse = new MultipartResponse(response);

                    // Is the convert type HTML?
                    if ( ( multipartrequest.getParameter( "converttype" ).equals(
                               "swriter: HTML (StarWriter)" ) )
                         || ( multipartrequest.getParameter( "converttype" ).equals(
                                  "scalc: HTML (StarCalc)" ) ) ) {
                        // Setting the content type of the response being sent to the client
                        // to text
                        multipartresponse.startResponse( "text/html" );
                    } else {
                        // Setting the content type of the response being sent to the client
                        // to application/octet-stream so that file will open a dialog box
                        // at the client in order to save the converted file
                        multipartresponse.startResponse( "application/octet-stream" );
                    }

                    // Pushing the converted file to the client
                    ServletUtils.returnFile( stringConvertedFile,
                                             response.getOutputStream() );

                    // Finishing the multi part response
                    multipartresponse.finish();

                    // clean up the working directory
                    cleanupFile = new File(stringConvertedFile);
                    if ( cleanupFile.exists() )
                        cleanupFile.delete();

                } catch (Exception exc) {
                    response.setContentType( "text/html;charset=8859-1" );
                    PrintWriter out = response.getWriter();

                    exc.printStackTrace();

                    out.println( "<html><head>" );
                    out.println( " <title>" + "SDK Converter Servlet" + "</title>" );
                    out.println( "</head>" );
                    out.println( "<body><br><p>");
                    out.println( "<b>Sorry, the conversion failed!</b></p>");
                    out.println( "<p><b>Error Mesage:</b><br>" + exc.getMessage() + "<br>");
                    exc.printStackTrace(out);
                    out.println( "</p></body><html>");
                }

                // clean up the working directory
                cleanupFile = new File(stringSourceFile);
                if ( cleanupFile.exists() )
                    cleanupFile.delete();
            }
        }
        catch (Exception exception) {
            System.err.println( exception.toString() );
        }
    }

    /** This method converts a document to a given type by using a running
     * OpenOffice.org and saves the converted document to the specified
     * working directory.
     * @param stringDocumentName The full path name of the file on the server to be converted.
     * @param stringConvertType Type to convert to.
     * @param stringExtension This string will be appended to the file name of the converted file.
     * @return The full path name of the converted file will be returned.
     * @see stringWorkingDirectory
     */
    private String convertDocument( String stringDocumentName,
                                    String stringConvertType,
                                    String stringExtension)
        throws Exception
    {
        String stringConvertedFile = "";

        // Converting the document to the favoured type
//         try {
            // Composing the URL
            String stringUrl = "file:///" + stringDocumentName;

            /* Bootstraps a component context with the jurt base components
               registered. Component context to be granted to a component for running.
               Arbitrary values can be retrieved from the context. */
            XComponentContext xcomponentcontext =
                com.sun.star.comp.helper.Bootstrap.createInitialComponentContext( null );

            /* Gets the service manager instance to be used (or null). This method has
               been added for convenience, because the service manager is a often used
               object. */
            XMultiComponentFactory xmulticomponentfactory =
                xcomponentcontext.getServiceManager();

            /* Creates an instance of the component UnoUrlResolver which
               supports the services specified by the factory. */
            Object objectUrlResolver =
                xmulticomponentfactory.createInstanceWithContext(
                    "com.sun.star.bridge.UnoUrlResolver", xcomponentcontext );

            // Create a new url resolver
            XUnoUrlResolver xurlresolver = ( XUnoUrlResolver )
                UnoRuntime.queryInterface( XUnoUrlResolver.class,
                                           objectUrlResolver );

            // Resolves an object that is specified as follow:
            // uno:<connection description>;<protocol description>;<initial object name>
            Object objectInitial = xurlresolver.resolve(
                "uno:socket,host=" + stringHost + ",port=" + stringPort +
                ";urp;StarOffice.ServiceManager" );

            // Create a service manager from the initial object
            xmulticomponentfactory = ( XMultiComponentFactory )
                UnoRuntime.queryInterface( XMultiComponentFactory.class, objectInitial );

            // Query for the XPropertySet interface.
            XPropertySet xpropertysetMultiComponentFactory = ( XPropertySet )
                UnoRuntime.queryInterface( XPropertySet.class, xmulticomponentfactory );

            // Get the default context from the office server.
            Object objectDefaultContext =
                xpropertysetMultiComponentFactory.getPropertyValue( "DefaultContext" );

            // Query for the interface XComponentContext.
            xcomponentcontext = ( XComponentContext ) UnoRuntime.queryInterface(
                XComponentContext.class, objectDefaultContext );

            /* A desktop environment contains tasks with one or more
               frames in which components can be loaded. Desktop is the
               environment for components which can instanciate within
               frames. */
            XComponentLoader xcomponentloader = ( XComponentLoader )
                UnoRuntime.queryInterface( XComponentLoader.class,
                                           xmulticomponentfactory.createInstanceWithContext(
                                               "com.sun.star.frame.Desktop", xcomponentcontext ) );

            // Preparing properties for loading the document
            PropertyValue propertyvalue[] = new PropertyValue[ 1 ];
            // Setting the flag for hidding the open document
            propertyvalue[ 0 ] = new PropertyValue();
            propertyvalue[ 0 ].Name = "Hidden";
            propertyvalue[ 0 ].Value = new Boolean(true);

            // Loading the wanted document
            Object objectDocumentToStore =
                xcomponentloader.loadComponentFromURL(
                    stringUrl, "_blank", 0, propertyvalue );

            // Getting an object that will offer a simple way to store a document to a URL.
            XStorable xstorable =
                ( XStorable ) UnoRuntime.queryInterface( XStorable.class,
                                                         objectDocumentToStore );

            // Preparing properties for converting the document
            propertyvalue = new PropertyValue[ 2 ];
            // Setting the flag for overwriting
            propertyvalue[ 0 ] = new PropertyValue();
            propertyvalue[ 0 ].Name = "Overwrite";
            propertyvalue[ 0 ].Value = new Boolean(true);
            // Setting the filter name
            propertyvalue[ 1 ] = new PropertyValue();
            propertyvalue[ 1 ].Name = "FilterName";
            propertyvalue[ 1 ].Value = stringConvertType;

            // Appending the favoured extension to the origin document name
            int index = stringUrl.lastIndexOf('.');
            if ( index >= 0 ) {
                stringConvertedFile = stringUrl.substring(0, index) + "." + stringExtension;
            } else {
                stringConvertedFile = stringUrl + "." + stringExtension;
            }

            // Storing and converting the document
            xstorable.storeAsURL( stringConvertedFile, propertyvalue );

            XCloseable xcloseable = (XCloseable)UnoRuntime.queryInterface( XCloseable.class,xstorable );

            // Closing the converted document
            if ( xcloseable != null )
                xcloseable.close(false);
            else {
                // If Xcloseable is not supported (older versions,
                // use dispose() for closing the document
                XComponent xComponent = ( XComponent ) UnoRuntime.queryInterface(
                    XComponent.class, xstorable );
                xComponent.dispose();
            }

//         }
//         catch( Exception exception ) {
//             exception.printStackTrace();
//             return( "" );
//         }

        if ( stringConvertedFile.startsWith( "file:///" ) ) {
            // Truncating the beginning of the file name
            stringConvertedFile = stringConvertedFile.substring( 8 );
        }

        // Returning the name of the converted file
        return stringConvertedFile;
    }
}
