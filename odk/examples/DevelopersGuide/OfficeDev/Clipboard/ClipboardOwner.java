import com.sun.star.datatransfer.*;
import com.sun.star.datatransfer.clipboard.*;

//--------------------------
//  A simple clipboard owner
//--------------------------

public class ClipboardOwner implements XClipboardOwner
{
    public void lostOwnership( XClipboard xClipboard, XTransferable xTransferable )
    {
        System.out.println("");
        System.out.println( "Lost clipboard ownership..." );
        System.out.println("");

        isowner = false;
    }

    public boolean isClipboardOwner()
    {
        return isowner;
    }

    private boolean isowner = true;
}
