import com.sun.star.sdb.XRowSetApproveListener;
import com.sun.star.sdbc.XRowSetListener;
import com.sun.star.sdb.RowChangeEvent;
import com.sun.star.lang.EventObject;

public class RowSetEventListener implements XRowSetApproveListener,XRowSetListener
{
    // XEventListener
    public void disposing(com.sun.star.lang.EventObject event)
    {
        System.out.println("RowSet will be destroyed!");
    }
    // XRowSetApproveBroadcaster
    public boolean approveCursorMove(EventObject event)
    {
        System.out.println("Before CursorMove!");
        return true;
    }
    public boolean approveRowChange(RowChangeEvent event)
    {
        System.out.println("Before row change!");
        return true;
    }
    public boolean approveRowSetChange(EventObject event)
    {
        System.out.println("Before RowSet change!");
        return true;
    }

    // XRowSetListener
    public void cursorMoved(com.sun.star.lang.EventObject event)
    {
        System.out.println("Cursor moved!");
    }
    public void rowChanged(com.sun.star.lang.EventObject event)
    {
        System.out.println("Row changed!");
    }
    public void rowSetChanged(com.sun.star.lang.EventObject event)
    {
        System.out.println("RowSet changed!");
    }
}
