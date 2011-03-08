import javax.mail.*;
import javax.mail.internet.*;
import com.msoft.mail.provider.nntp.NNTPTransport;
import java.util.Properties;
import java.io.*;
import javax.activation.*;


public class Sender
{
    // Constructor params:
    private StatusWindow status = null;
    private OfficeAttachment attachments = null;
    private String replyto = "";
        private String subject = "";
        private String comment = "";
        private String hostname = "";
        private String newsgroup = "";
    private String statusLine = "";



    public Sender( StatusWindow sw, OfficeAttachment attach, String reply,
               String sub, String com, String host, String group )
    {
        status = sw;
        attachments = attach;
                replyto = reply;
                subject = sub;
                comment = com;
                hostname = host;
                newsgroup = group;
    }



    public boolean sendMail()
    {
        int statusPos = 5;
        try
        {
            attachments.createTempDocs();
            // Property for any information
            Properties props = new Properties();

            // Create unique session (null is unused authenticator info)
            statusLine = "Creating unique session";
            status.setStatus( statusPos, statusLine ); // 5
            Session session = Session.getInstance( props, null );

            // Create message
            statusPos++; // 6
            statusLine = "Creating message";
            status.setStatus( statusPos, statusLine );
            MimeMessage message = new MimeMessage( session );
            message.setFrom( new InternetAddress( replyto ) );
            message.setSubject( subject );
            message.setText( comment );
            message.addHeader( "Newsgroups", newsgroup );

            // Buildup bodypart with text and attachments
            Multipart multipart = new MimeMultipart();

            BodyPart messageBodyPart = new MimeBodyPart();
            messageBodyPart.setText( comment );
            multipart.addBodyPart( messageBodyPart );

            statusPos++; // 7
            statusLine = "Adding attachment(s)";
            status.setStatus( statusPos, statusLine );
            File attachs[] = attachments.getAttachments();
            for(int i=0; i < attachs.length; i++ )
            {
                //System.out.println( "Adding file: " + attachs[i].getName() );
                messageBodyPart = new MimeBodyPart();
                DataSource filesource = new FileDataSource( attachs[i] );
                messageBodyPart.setDataHandler( new DataHandler( filesource ));
                messageBodyPart.setFileName( attachs[i].getName() );
                multipart.addBodyPart( messageBodyPart );
            }

            // Add multipart to mail
            message.setContent( multipart );

            // Create and send NNTP transport
            statusPos += 2; // 9
            statusLine = "Creating NNTP transport";
            status.setStatus( statusPos, statusLine );
            Transport transport = new NNTPTransport( session, new URLName( "news:" + newsgroup ));

            // Null parameters are for user name and password
            statusPos++; // 10
            statusLine = "Connecting to mail server";
            status.setStatus( statusPos, statusLine );
            transport.connect( hostname, null, null );

            statusPos++; // 11
            statusLine = "Sending message";
            status.setStatus( statusPos, statusLine );
            transport.sendMessage( message, message.getAllRecipients() );

            statusPos++; // 12
            statusLine = "Closing transport";
            status.setStatus( statusPos, statusLine );
            transport.close();

            // Clean up when finished
            attachments.removeTempDocs();

            return true;
        }
        catch( MessagingException me )
        {
            if( statusPos == 10 )
            {
                statusLine = "Error connecting (User authentication?)";
            }
            status.setStatus( statusPos, statusLine );
            System.out.println( "Error sending message: ");
            me.printStackTrace();
            return false;
        }

    }

}
