package installer;
import java.io.PrintStream;
import java.io.FileOutputStream;

import java.util.Date;
import java.text.DateFormat;
import java.text.SimpleDateFormat;


//import java.io.PrintWriter;
public class LogStream extends PrintStream
{
    static final private DateFormat formatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss z: ");

    private String getTimeStamp()
    {
         String timeStamp = formatter.format( new Date() );
         return timeStamp;
    }
    public LogStream( String logFileName ) throws java.io.FileNotFoundException
    {
        super( new FileOutputStream( logFileName ) );
    }
    public  void println(String x)
    {
        super.println( getTimeStamp() + x );
    }
    public static void main(String[] args)
    {
        if ( args.length > 0 )
        {
            try
            {
                LogStream log = new LogStream( args[0] );
                System.setErr(log);
                System.setOut(log);
                System.out.println("Test from logger from out");
                System.err.println("Test from logger from err");
                System.out.println("finised test from out");
                System.err.println("finised test from err");
            }
            catch( java.io.FileNotFoundException fe )
            {
                System.err.println("Error creating logStream: " + fe );
                fe.printStackTrace();
            }
        }
        else
        {
            System.err.println("specify log file java LogStream [logfile]");
            System.exit(1);
        }
    }
}
