import com.sun.star.uno.*;
import com.sun.star.container.*;
import com.sun.star.lang.*;

/**************************************************************************/
/** an abstract interface for components doing an action on a form component
*/
interface IFormComponentAction
{
    public abstract void handle( Object aFormComponent ) throws java.lang.Exception;
};

/**************************************************************************/
/** a helper class for travelling a form component tree
*/
class ComponentTreeTraversal implements IFormComponentAction
{
    /* ------------------------------------------------------------------ */
    /** Indicator method to decide wether to step down the tree.

        <p>The default implementation checks if the container given is a grid
        control model or a <service scope="com.sun.star.form">FormComponents</service>
        instance.</p>
    */
    protected boolean shouldStepInto( XIndexContainer xContainer ) throws com.sun.star.uno.Exception
    {
        // step down the tree, if possible
        XServiceInfo xSI = UNO.queryServiceInfo( xContainer );
        if  (   null != xSI
            &&  (   xSI.supportsService( "com.sun.star.form.FormComponents" )
                ||  xSI.supportsService( "com.sun.star.form.component.GridControl" )
                )
            )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    /* ------------------------------------------------------------------ */
    public void handle( Object aFormComponent ) throws com.sun.star.uno.Exception
    {
        XIndexContainer xCont = UNO.queryIndexContainer( aFormComponent );
        if  (   ( null != xCont )
            &&  shouldStepInto( xCont )
            )
        {
            for ( int i=0; i<xCont.getCount(); ++i )
            {
                handle( xCont.getByIndex( i ) );
            }
        }
    }
}

