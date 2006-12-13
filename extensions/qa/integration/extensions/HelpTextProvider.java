/*
 * HelpTextProvider.java
 *
 * Created on 16. November 2006, 09:44
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package integration.extensions;

import com.sun.star.inspection.XObjectInspectorUI;
import com.sun.star.inspection.XPropertyControl;
import com.sun.star.inspection.XPropertyControlObserver;
import com.sun.star.lang.NoSupportException;

/** displays help text for the currently selected method
 */
public class HelpTextProvider implements XPropertyControlObserver
{
    private XObjectInspectorUI  m_inspectorUI;

    /**
     * Creates a new instance of HelpTextProvider
     */
    public HelpTextProvider( XObjectInspectorUI _inspectorUI )
    {
        m_inspectorUI = _inspectorUI;
        m_inspectorUI.registerControlObserver( this );
    }

    public void focusGained( XPropertyControl _propertyControl )
    {
        try
        {
            String helpText = "here could be the help for:\n";
            helpText += _propertyControl.getValue().toString();
            m_inspectorUI.setHelpSectionText( helpText );
        }
        catch (NoSupportException ex)
        {
            ex.printStackTrace();
        }
    }

    public void valueChanged( XPropertyControl _propertyControl )
    {
        // not interested in
    }
}
