// base classes
import com.sun.star.io.XActiveDataSink;
import com.sun.star.io.XInputStream;

/**
 * XActiveDataSink interface implementation. Makes it possible to read
 * the corresponding object from an input stream.
 */
public class MyActiveDataSink implements XActiveDataSink {

    /**
     * Member properties
     */
    XInputStream m_aStream = null;

    /**
     * Constructor
     */
    public MyActiveDataSink() {
        super();
    }

    /**
     *   Plugs the input stream.
     *
     *@param  XInputStream
     */
    public void setInputStream( XInputStream aStream ) {
        m_aStream = aStream;
    }

    /**
     *  Get the plugged stream.
     *
     *@return  XInputStream  The plugged stream
     */
    public XInputStream getInputStream() {
        return m_aStream;
    }
}