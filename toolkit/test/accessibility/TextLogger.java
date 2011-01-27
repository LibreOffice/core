import javax.swing.JEditorPane;
import javax.swing.event.HyperlinkListener;
import javax.swing.event.HyperlinkEvent;
import javax.swing.text.Document;
import java.net.URL;

class TextLogger
    extends JEditorPane
{
    public TextLogger ()
        throws java.io.IOException
    {
//        maDocument = getEditorKit().createDefaultDocument();
            super ("http://localhost");
        try
        {
            //            setPage (new URL ("http://www.spiegel.de"));
        }
        catch (Exception e)
        {}

        setEditable (false);
        final JEditorPane finalPane = this;
        addHyperlinkListener (new HyperlinkListener()
            {
                public void hyperlinkUpdate (HyperlinkEvent e)
                {
                    try
                    {
                        if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED)
                            finalPane.setPage (e.getURL());
                    }
                    catch (java.io.IOException ex)
                    {
                        ex.printStackTrace(System.err);
                    }
                }
            });
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
