import java.awt.Font;
import java.awt.Rectangle;
import java.awt.Color;
import java.awt.Graphics;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JScrollBar;



/** A message area displays text in a scrollable text widget.  It is a
    singleton.  Other objects can access it directly to display messages.
*/
public class MessageArea
    extends JScrollPane
{
    public static synchronized MessageArea Instance ()
    {
        if (saInstance == null)
            saInstance = new MessageArea ();
        return saInstance;
    }




    /** Create a new message area.  This method is private because the class is
        a singleton and may therefore not be instanciated from the outside.
    */
    private MessageArea ()
    {
        maText = new JTextArea();
        maText.setBackground (new Color (255,250,240));
        maText.setFont (new Font ("Helvetica", Font.PLAIN, 9));
        setViewportView (maText);
        setVerticalScrollBarPolicy (JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        setHorizontalScrollBarPolicy (JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);

        printMessage (
            "class path is " + System.getProperty ("java.class.path") + "\n");
    }




    /** Show the given string at the end of the message area and scroll to make
        it visible.
    */
    public static synchronized void print (String aMessage)
    {
        print (0, aMessage);
    }




    /** Show the given string at the end of the message area and scroll to make
        it visible.  Indent the string as requested.
    */
    public static synchronized void print (int nIndentation, String aMessage)
    {
        while (nIndentation-- > 0)
            aMessage = " " + aMessage;
        Instance().printMessage(aMessage);
    }




    /** Show the given string at the end of the message area and scroll to make
        it visible.
    */
    public static void println (String aMessage)
    {
        println (0, aMessage);
    }




    /** Show the given string at the end of the message area and scroll to make
        it visible.
    */
    public static void println (int nIndentation, String aMessage)
    {
        print (nIndentation, aMessage+"\n");
    }




    public void paintComponent (Graphics g)
    {
        JScrollBar sb = getVerticalScrollBar();
        if (sb != null)
        {
            int nScrollBarValue = sb.getMaximum() - sb.getVisibleAmount() - 1;
            sb.setValue (nScrollBarValue);
        }
        super.paintComponent (g);
    }




    /** Append the given string to the end of the text and scroll so that it
        becomes visible.  This is an internal method.  Use one of the static
        and public ones.
    */
    private synchronized void printMessage (String aMessage)
    {
        maText.append (aMessage);
    }




    private static MessageArea saInstance = null;
    private JTextArea maText;
}
