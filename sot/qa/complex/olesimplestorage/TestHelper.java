package complex.olesimplestorage;

import share.LogWriter;

public class TestHelper
{
    LogWriter m_aLogWriter;
    String m_sTestPrefix;

    /** Creates a new instance of TestHelper */
    public TestHelper ( LogWriter aLogWriter, String sTestPrefix )
    {
        m_aLogWriter = aLogWriter;
        m_sTestPrefix = sTestPrefix;
    }

    public void Error ( String sError )
    {
        m_aLogWriter.println ( m_sTestPrefix + "Error: " + sError );
    }

    public void Message ( String sMessage )
    {
        m_aLogWriter.println ( m_sTestPrefix + sMessage );
    }
}
