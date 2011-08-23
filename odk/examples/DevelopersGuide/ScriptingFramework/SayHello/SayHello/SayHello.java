import com.sun.star.script.provider.XScriptContext;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.lang.XComponent;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XText;
import com.sun.star.beans.PropertyValue;
import com.sun.star.awt.ActionEvent;

public class SayHello
{
    public static void start(XScriptContext ctxt, ActionEvent e)
        throws Exception
    {
        SayHello.start(ctxt);
    }
    
    public static void start(XScriptContext ctxt)
        throws Exception
    {
        // getting the text document object
        XTextDocument xTextDocument = createDocument(ctxt);

        XText xText = xTextDocument.getText();
        XTextRange xTextRange = xText.getEnd();
        xTextRange.setString("Hello");
    }

    private static XTextDocument createDocument(XScriptContext ctxt)
        throws Exception
    {
        XComponentLoader loader = (XComponentLoader)
            UnoRuntime.queryInterface(
                XComponentLoader.class, ctxt.getDesktop());

        XComponent comp = loader.loadComponentFromURL(
            "private:factory/swriter", "_blank", 4, new PropertyValue[0]);

        XTextDocument doc = (XTextDocument)
            UnoRuntime.queryInterface( XTextDocument.class, comp);

        return doc;
    }
}
