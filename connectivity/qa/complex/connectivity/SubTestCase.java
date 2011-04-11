package complex.connectivity;

import share.LogWriter;

public class SubTestCase implements TestCase
{
    protected SubTestCase( final TestCase i_parentTestCase )
    {
        m_parentTestCase = i_parentTestCase;
    }

    public void assure( String i_message, boolean i_condition )
    {
        m_parentTestCase.assure( i_message, i_condition );
    }

    public LogWriter getLog()
    {
        return m_parentTestCase.getLog();
    }

    private final TestCase m_parentTestCase;
}
