package com.sun.star.script.framework.log;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;

public class LogUtils {

    private static boolean m_bDebugEnabled = false;

    static
    {
        String debugFlag =
            System.getProperties().getProperty("ScriptJavaRuntimeDebug");

        if (debugFlag != null && debugFlag.length() > 0)
        {
            m_bDebugEnabled = debugFlag.equalsIgnoreCase("true");
        }
    }

    // Ensure that instances of this class cannot be created
    private LogUtils() {
    }

    /**
    *  Print Debug Output
    *
    * @param  msg  message to be displayed
    */
    public static void DEBUG(String msg)
    {
        if (m_bDebugEnabled)
        {
            System.out.println(msg);
        }
    }

    public static String getTrace( Exception e )
    {
        ByteArrayOutputStream baos = null;
        PrintStream ps = null;
        String result = "";
        try
        {
            baos = new ByteArrayOutputStream( );
            ps = new PrintStream( baos );
            e.printStackTrace( ps );
        }
        finally
        {
            try
            {
                if ( baos != null )
                {
                    baos.close();
                }
                if ( ps != null )
                {
                    ps.close();
                }
            }
            catch ( Exception excp )
            {
            }
        }
        return result;
    }
}
