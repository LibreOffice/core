import javax.swing.JFrame;

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
        maFrame = new JFrame ();
        maLogger = new TextLogger ();
        maFrame.getContentPane().add (maLogger);

        maFrame.pack ();
        maFrame.setVisible (true);
    }

    private static EventLogger maInstance = null;
    private JFrame maFrame;
    private TextLogger maLogger;
}
