import javax.swing.JEditorPane;
import javax.swing.text.Document;
import java.net.URL;

class TextLogger
    extends JEditorPane
{
    public TextLogger ()
    {
//        maDocument = getEditorKit().createDefaultDocument();
        try
        {
            setPage (new URL ("http://www.spiegel.de"));
        }
        catch (Exception e)
        {}
    }

    public void appendText (String sText)
    {
        try
        {
            maDocument.insertString (maDocument.getLength(), sText, null);
        }
        catch (javax.swing.text.BadLocationException e)
        {}
    }

    private Document maDocument;
}
