// JDK API
import java.io.IOException;
import java.io.PrintWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.BufferedInputStream;
import java.util.Enumeration;

// Servlet API
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.ServletOutputStream;

// Helper API
import com.oreilly.servlet.MultipartRequest;
import com.oreilly.servlet.MultipartResponse;
import com.oreilly.servlet.ServletUtils;

// UNO API
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;


/** This class implements a http servlet in order to convert an incoming document
 * with help of a running OpenOffice.org and to push the converted file back
 * to the client.
 */
public class ConverterServlet extends HttpServlet {
  /** Specifies the working directory on the web server.
   */
  private String stringWorkingDirectory = "g:/trash/";

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
        String stringConvertedFile = this.convertDocument(
        stringWorkingDirectory + stringFilename,
        multipartrequest.getParameter( "converttype" ),
        multipartrequest.getParameter( "extension" ) );

        // Constructing the multi part response to the client
        MultipartResponse multipartresponse = new MultipartResponse(
        response );

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
  String stringConvertType, String stringExtension ) {
    String stringConvertedFile = "";

    // Converting the document to the favoured type
    try {
      // Composing the URL
      String stringUrl = "file:///" + stringDocumentName;

      /* Bootstraps a servicemanager with the jurt base components
         registered */
      XMultiServiceFactory xmultiservicefactory =
      com.sun.star.comp.helper.Bootstrap.createSimpleServiceManager();

      /* Creates an instance of the component UnoUrlResolver which
         supports the services specified by the factory. */
      Object objectUrlResolver = xmultiservicefactory.createInstance(
      "com.sun.star.bridge.UnoUrlResolver" );

      // Create a new url resolver
      XUnoUrlResolver xurlresolver = ( XUnoUrlResolver )
      UnoRuntime.queryInterface( XUnoUrlResolver.class,
      objectUrlResolver );

      // Resolves an object that is specified as follow:
      // uno:<connection description>;<protocol description>;<initial object name>
      Object objectInitial = xurlresolver.resolve(
      "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager" );

      // Create a service manager from the initial object
      xmultiservicefactory = ( XMultiServiceFactory )
      UnoRuntime.queryInterface( XMultiServiceFactory.class,
      objectInitial );

      /* A desktop environment contains tasks with one or more
         frames in which components can be loaded. Desktop is the
         environment for components which can instanciate within
         frames. */
      XComponentLoader xcomponentloader = ( XComponentLoader )
      UnoRuntime.queryInterface( XComponentLoader.class,
      xmultiservicefactory.createInstance(
      "com.sun.star.frame.Desktop" ) );

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
      stringConvertedFile = stringUrl + "." + stringExtension;

      // Storing and converting the document
      xstorable.storeAsURL( stringConvertedFile, propertyvalue );

      // Getting the method dispose() for closing the document
      XComponent xcomponent =
      ( XComponent ) UnoRuntime.queryInterface( XComponent.class,
      xstorable );

      // Closing the converted document
      xcomponent.dispose();
    }
    catch( Exception exception ) {
      exception.printStackTrace();
      return( "" );
    }

    if ( stringConvertedFile.startsWith( "file:///" ) ) {
      // Truncating the beginning of the file name
      stringConvertedFile = stringConvertedFile.substring( 8 );
    }

    // Returning the name of the converted file
    return( stringConvertedFile );
  }
}
