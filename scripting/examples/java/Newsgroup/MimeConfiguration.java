import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.script.framework.runtime.XScriptContext;
import com.sun.star.util.XStringSubstitution;

import javax.mail.*;
import javax.activation.*;

import java.io.*;


public class MimeConfiguration
{

    // Office Installation path
    private static String instPath = "";


    public static boolean createFiles( XScriptContext xsc )
    {
        try
        {
            XComponentContext xcc = xsc.getComponentContext();
            XMultiComponentFactory xmf = xcc.getServiceManager();

            Object pathSub = xmf.createInstanceWithContext( "com.sun.star.comp.framework.PathSubstitution", xcc );
            XStringSubstitution stringSub = ( XStringSubstitution ) UnoRuntime.queryInterface( XStringSubstitution.class, pathSub );
            instPath = stringSub.getSubstituteVariableValue( "$(inst)" );

        }
        catch( com.sun.star.beans.UnknownPropertyException upe )
        {
            System.out.println( "com.sun.star.beans.UnknownPropertyException" );
            upe.printStackTrace();
        }
        catch( com.sun.star.uno.Exception e )
        {
            System.out.println( "com.sun.star.uno.Exception" );
            e.printStackTrace();
        }

        writeMailCap();
        writeMimeTypes();

        // ToDo: include status feedback to StatusWindow
        return true;
    }




    private static void writeMailCap()
    {
        String mailcapPath = getConfigDir() + System.getProperty( "file.separator" ) + "mailcap";

        try
        {
                    if( ! new File( java.net.URLDecoder.decode( mailcapPath )  ).exists() )
                           {
                //System.out.println( "URLDecoder: " + java.net.URLDecoder.decode( mailcapPath ) );
                File mailcapFile = new File( mailcapPath );
                FileWriter out = new FileWriter( mailcapFile );
                String[] lines = getMailcapText();
                for( int i=0; i<lines.length; i++ )
                {
                    out.write( lines[i], 0, lines[i].length() );
                }
                out.close();
                    }
                    else
                    {
                //System.out.println( "URLDecoder: " + java.net.URLDecoder.decode( mailcapPath ) );
                    }



            // use prog dir, if not there then java.io to create/write new file
            MailcapCommandMap map = new MailcapCommandMap( mailcapPath );
            CommandMap.setDefaultCommandMap ( map );
        }
        catch( IOException ioe )
        {
            ioe.printStackTrace();
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }
    }


    private static String[] getMailcapText()
    {
        String[] mailcapText = {
            "#\n",
            "# Default mailcap file for the JavaMail System.\n",
            "#\n",
            "# JavaMail content-handlers:\n",
            "#\n",
            "text/plain;;            x-java-content-handler=com.sun.mail.handlers.text_plain\n",
            "text/html;;             x-java-content-handler=com.sun.mail.handlers.text_html\n",
            "text/xml;;              x-java-content-handler=com.sun.mail.handlers.text_xml\n",
            "image/gif;;             x-java-content-handler=com.sun.mail.handlers.image_gif\n",
            "image/jpeg;;            x-java-content-handler=com.sun.mail.handlers.image_jpeg\n",
            "multipart/*;;           x-java-content-handler=com.sun.mail.handlers.multipart_mixed\n",
            "message/rfc822;;        x-java-content-handler=com.sun.mail.handlers.message_rfc822\n"
        };

        return mailcapText;
    }



    private static void writeMimeTypes()
    {
        String mimetypesPath = getConfigDir() + System.getProperty( "file.separator" ) + "mimetypes.default";

        try
        {
                    if( ! new File( java.net.URLDecoder.decode( mimetypesPath )  ).exists() )
                           {
                //System.out.println( "URLDecoder: " + java.net.URLDecoder.decode( mimetypesPath ) );
                File mimetypesFile = new File( mimetypesPath );
                FileWriter out = new FileWriter( mimetypesFile );
                String[] lines = getMimeTypesText();
                for( int i=0; i<lines.length; i++ )
                {
                    out.write( lines[i], 0, lines[i].length() );
                }
                out.close();
                    }
                    else
                    {
                //System.out.println( "URLDecoder: " + java.net.URLDecoder.decode( mimetypesPath ) );
                    }

            MimetypesFileTypeMap mimeTypes = new MimetypesFileTypeMap( mimetypesPath );
                        FileTypeMap.setDefaultFileTypeMap( mimeTypes );
        }
        catch( IOException ioe )
        {
            ioe.printStackTrace();
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }
    }


    private static String[] getMimeTypesText()
    {
        String[] mimesText = {
            "#\n",
            "# A simple, old format, mime.types file\n",
            "#\n",
            "text/html               html htm HTML HTM\n",
            "text/plain              txt text TXT TEXT\n",
            "image/gif               gif GIF\n",
            "image/ief               ief\n",
            "image/jpeg              jpeg jpg jpe JPG\n",
            "image/tiff              tiff tif\n",
            "image/x-xwindowdump     xwd\n",
            "application/postscript  ai eps ps\n",
            "application/rtf         rtf\n",
            "application/x-tex       tex\n",
            "application/x-texinfo   texinfo texi\n",
            "application/x-troff     t tr roff\n",
            "audio/basic             au\n",
            "audio/midi              midi mid\n",
            "audio/x-aifc            aifc\n",
            "audio/x-aiff            aif aiff\n",
            "audio/x-mpeg            mpeg mpg\n",
            "audio/x-wav             wav\n",
            "video/mpeg              mpeg mpg mpe\n",
            "video/quicktime         qt mov\n",
            "video/x-msvideo         avi\n"
        };

        return mimesText;
    }


    private static String getConfigDir()
    {
        // mailcap file must be written to the Office user/config  directory

                // instPath is a URL, needs to be converted to a system pathname
                String config = instPath + "/user/config";
                String configNonURL = "";

                if( System.getProperty( "os.name" ).indexOf( "Windows" ) != -1 )
                {
                        // Windows
                        // removes "file:///"
                        int start = 8;
                        configNonURL = config.substring( start, config.length() );
                        // Convert forward to back-slashes
                        while( configNonURL.indexOf( "/" ) != -1 )
                        {
                                int fSlash = configNonURL.indexOf( "/" );
                                String firstPart = configNonURL.substring( 0, fSlash );
                                String secondPart = configNonURL.substring( fSlash + 1, configNonURL.length() );
                                configNonURL = firstPart + "\\" + secondPart;
                        }
                }
                else
                {
                        // Unix/Linux
                        // removes "file://"
                        int start = 7;
                        configNonURL = config.substring( start, config.length() );
                }

        return configNonURL;
    }

}
