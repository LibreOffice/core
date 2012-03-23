import com.sun.star.script.provider.XScriptContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XText;
/**
 *  HelloWorld class
 *
 */
public class HelloWorld {
  public static void printHW(XScriptContext xSc) {

    // getting the text document object
    XTextDocument xtextdocument = (XTextDocument) UnoRuntime.queryInterface(
XTextDocument.class, xSc.getDocument());
    XText xText = xtextdocument.getText();
    XTextRange xTextRange = xText.getEnd();
    xTextRange.setString( "Hello World (in Java)" );

  }// printHW

}
