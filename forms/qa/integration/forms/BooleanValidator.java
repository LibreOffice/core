/*
 * BooleanValidator.java
 *
 * Created on 10. März 2004, 16:27
 */

package integration.forms;

/**
 *
 * @author  fs93730
 */
public class BooleanValidator extends integration.forms.ControlValidator
{
    private boolean m_preventChecked;

    /** Creates a new instance of BooleanValidator */
    public BooleanValidator( boolean preventChecked )
    {
        m_preventChecked = preventChecked;
    }

    public String explainInvalid( Object Value )
    {
        try
        {
            short value = ((Short)Value).shortValue();
            if ( value == 2 )
                return "'indetermined' is not an allowed state";
            if ( m_preventChecked && ( value == 1 ) )
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
            short value = ((Short)Value).shortValue();
            if ( value == 2 )
                return false;
            if ( m_preventChecked && ( value == 1 ) )
                return false;
            return true;
        }
        catch( java.lang.Exception e )
        {
        }
        return false;
    }
}
