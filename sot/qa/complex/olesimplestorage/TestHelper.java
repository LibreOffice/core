package complex.olesimplestorage;


public class TestHelper
{

    String m_sTestPrefix;

    /** Creates a new instance of TestHelper
     * @param sTestPrefix
     */
    public TestHelper (  String sTestPrefix )
    {

        m_sTestPrefix = sTestPrefix;
    }

    public void Error ( String sError )
    {
        System.out.println ( m_sTestPrefix + "Error: " + sError );
    }

    public void Message ( String sMessage )
    {
        System.out.println ( m_sTestPrefix + sMessage );
    }
}
