import javax.swing.JFrame;
import javax.swing.JScrollPane;

class EventLogger
{
    public static synchronized EventLogger Instance ()
    {
        if (maInstance == null)
            maInstance = new EventLogger();
        return maInstance;
    }

    private EventLogger ()
    {
        try
        {
            maFrame = new JFrame ();
            maLogger = new TextLogger ();
            maFrame.setContentPane (new JScrollPane (maLogger));

            maFrame.setSize (400,300);
            maFrame.setVisible (true);
        }
        catch (Exception e)
        {}
    }

    private static EventLogger maInstance = null;
    private JFrame maFrame;
    private TextLogger maLogger;
}
