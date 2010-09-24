/*
 * BooleanValidator.java
 *
 * Created on 10. Maerz 2004, 16:27
 */

package integration.forms;

/**
 *
 * @author  fs93730
 */
public class BooleanValidator extends integration.forms.ControlValidator
{
    private boolean                         m_preventChecked;
    private com.sun.star.uno.AnyConverter   m_converter;

    /** Creates a new instance of BooleanValidator */
    public BooleanValidator( boolean preventChecked )
    {
        m_preventChecked = preventChecked;
        m_converter = new com.sun.star.uno.AnyConverter();
    }

    public String explainInvalid( Object Value )
    {
        try
        {
            if ( m_converter.isVoid( Value ) )
                return "'indetermined' is not an allowed state";
            boolean value = ((Boolean)Value).booleanValue();
            if ( m_preventChecked && ( value == true ) )
                return "no no no. Don't check it.";
        }
        catch( java.lang.Exception e )
        {
            return "ooops. Unknown error";
        }
        return "";
    }

    public boolean isValid( Object Value )
    {
        try
        {
            if ( m_converter.isVoid( Value ) )
                return false;

            boolean value = ((Boolean)Value).booleanValue();
            if ( m_preventChecked && ( value == true ) )
                return false;
            return true;
        }
        catch( java.lang.Exception e )
        {
        }
        return false;
    }
}
