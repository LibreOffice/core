import com.sun.star.frame.XFrameActionListener;
import com.sun.star.frame.FrameActionEvent;

import com.sun.star.lang.EventObject;

public class   FrameActionListener
    implements XFrameActionListener
{
    public FrameActionListener ()
    {
    }

    public void frameAction (com.sun.star.frame.FrameActionEvent aEvent)
    {
        System.out.println ("frame action");
    }

    public void disposing (com.sun.star.lang.EventObject aEvent)
    {
    }
}
