import com.sun.star.frame.TerminationVetoException;
import com.sun.star.frame.XTerminateListener;
/*
 * TerminateListener.java
 *
 * Created on 11. Oktober 2002, 15:49
 */

/**
 *
 * @author  dschulten
 */
public class TerminateListener implements XTerminateListener {

    public void notifyTermination(com.sun.star.lang.EventObject eventObject) {
        System.out.println("about to terminate...");
    }

    public void queryTermination(com.sun.star.lang.EventObject eventObject)
        throws TerminationVetoException {

        // test if we can terminate now
        if (TerminationTest.isAtWork() == true) {
            System.out.println("Terminate while we are at work? Are you nuts ;-)?");
            throw new TerminationVetoException();
        }
    }

    public void disposing(com.sun.star.lang.EventObject eventObject) {
    }
}
