import com.sun.star.lang.*;
import com.sun.star.datatransfer.*;
import com.sun.star.datatransfer.clipboard.*;

//-----------------------------
// A simple clipboard listener
//-----------------------------

public class ClipboardListener implements XClipboardListener
{
    public void disposing(EventObject event)
    {
    }

    public void changedContents(ClipboardEvent event)
    {
        System.out.println("");
        System.out.println("Clipboard content has changed!");
        System.out.println("");
    }
}
