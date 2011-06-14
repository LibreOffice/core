package complex.embedding;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.AnyConverter;

import com.sun.star.lang.*;
import com.sun.star.embed.*;
import com.sun.star.packages.*;
import com.sun.star.io.*;
import com.sun.star.beans.*;

import share.LogWriter;

public class TestHelper  {

    LogWriter m_aLogWriter;
    String m_sTestPrefix;

    public TestHelper( LogWriter aLogWriter, String sTestPrefix )
    {
        m_aLogWriter = aLogWriter;
        m_sTestPrefix = sTestPrefix;
    }

    public void Error( String sError )
    {
        m_aLogWriter.println( m_sTestPrefix + "Error: " + sError );
    }

    public void Message( String sMessage )
    {
        m_aLogWriter.println( m_sTestPrefix + sMessage );
    }
}

