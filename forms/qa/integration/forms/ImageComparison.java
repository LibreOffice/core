/*
 * ImageComparison.java
 *
 * Created on 23. September 2003, 17:40
 */

package integration.forms;

import complexlib.ComplexTestCase;

/**
 *
 * @author  fs93730
 */
public final class ImageComparison implements com.sun.star.awt.XImageConsumer
{

    private byte[] m_referenceBytes;
    private int m_referencePosition;
    private java.io.FileOutputStream m_stream;
    private Object m_notifyDone;

    public boolean imagesEqual( )
    {
        return m_referencePosition == m_referenceBytes.length;
    }

    /** Creates a new instance of ImageComparison */
    public ImageComparison( byte[] referenceBytes, Object toNotify )
    {
        m_referenceBytes = referenceBytes;
        m_referencePosition = 0;
        m_notifyDone = toNotify;
    }

    public void complete(int param, com.sun.star.awt.XImageProducer xImageProducer)
    {
        synchronized( m_notifyDone )
        {
            m_notifyDone.notify();
        }
    }

    public void init(int param, int param1)
    {
    }

    public void setColorModel(short param, int[] values, int param2, int param3, int param4, int param5)
    {
    }

    public void setPixelsByBytes(int param, int param1, int param2, int param3, byte[] values, int param5, int param6)
    {
        if ( m_referencePosition == -1 )
            // already failed
            return;

        int i = 0;
        while ( ( m_referencePosition < m_referenceBytes.length ) && ( i < values.length ) )
        {
            if ( m_referenceBytes[ m_referencePosition ] != values[ i ] )
            {
                m_referencePosition = -1;
                break;
            }
            ++i;
            ++m_referencePosition;
        }
    }

    public void setPixelsByLongs(int param, int param1, int param2, int param3, int[] values, int param5, int param6)
    {
    }

}
