/*
 * WindowListener.java
 *
 * Created on 30. Juli 2002, 12:36
 */

package helper;

/**
 * An own implementation of a XWindowListener
 *
 */
public class WindowListener implements com.sun.star.awt.XWindowListener {

    // hidden called
    public boolean hiddenTrigger;
    // move called
    public boolean movedTrigger;
    // resize called
    public boolean resizedTrigger;
    // show called
    public boolean shownTrigger;
    // dispose called
    public boolean disposeTrigger;

    /**
     * Creates a new WindowListener
     */
    public WindowListener() {
        resetTrigger();
    }

    /**
     * The window hidden event
     */
    public void windowHidden(com.sun.star.lang.EventObject eventObject) {
        hiddenTrigger = true;
    }

    /**
     * The window move event
     */
    public void windowMoved(com.sun.star.awt.WindowEvent windowEvent) {
        movedTrigger = true;
    }

    /**
     * The window resize event
     */
    public void windowResized(com.sun.star.awt.WindowEvent windowEvent) {
        resizedTrigger = true;
    }

    /**
     * The window show event
     */
    public void windowShown(com.sun.star.lang.EventObject eventObject) {
        shownTrigger = true;
    }

    /**
     * The dispose event
     */
    public void disposing(com.sun.star.lang.EventObject eventObject) {
        disposeTrigger = true;
    }

    /**
     * Reset all triggers to "not fired".
     */
    public void resetTrigger() {
        hiddenTrigger = false;
        movedTrigger = false;
        resizedTrigger = false;
        shownTrigger = false;
        disposeTrigger = false;
    }
}
