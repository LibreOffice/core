/*
 * Created on 07.11.2003
 *
 * To change the template for this generated file go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
package com.sun.star.wizards.ui;

import com.sun.star.awt.*;
import com.sun.star.uno.UnoRuntime;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class PeerConfig implements Runnable {
    protected String[] propNames;
    protected Object[] propValues;
    protected Object control;

    /**
     * constructs a PeeerConfiguration that
     * sets the given properties.
     * @param props an Object array, which contains
     * pairs of property names and values.
     */
    public PeerConfig(Object control_, String[] propNames_, Object[] propValues_) {
        propNames = propNames_;
        propValues = propValues_;
        control = control_;
    }

    private static String[] APN = new String[] { "AccessibilityName" };

    public PeerConfig(Object control, String accessibilityName) {
        this(control, APN, new Object[] { accessibilityName });
    }

    /**
     * Is called by the PeerConfig when a control's
     * peer should configure.
     * @param peer the peer to configure.
     */
    private void configure(XWindowPeer peer) {
        setPeerProperties(peer);
        configPeer(peer);
    }

    /**
     * sets the properties defined when instanciating this
     * Configuration object to the peers's XVclWindowPeer
     * interface.
     * @param peer
     */
    private void setPeerProperties(XWindowPeer peer) {
        XVclWindowPeer xVclWindowPeer = (XVclWindowPeer) UnoRuntime.queryInterface(XVclWindowPeer.class, peer);
        for (int i = 0; i < propNames.length; i++)
            xVclWindowPeer.setProperty(propNames[i], propValues[i]);
    }

    /**
     * Performs arbitrary configuration of the peer.
     * This method is empty, and should be overriden by classes who
     * wish to perform custom implementation of the peer.
     * @param peer the peer to be configered.
     */
    protected void configPeer(XWindowPeer peer) {
    }

    /* (non-Javadoc)
     * @see java.lang.Runnable#run()
     */
    public void run() {
        configure(getPeer());
    }

    private XWindowPeer getPeer() {
        return ((XControl) UnoRuntime.queryInterface(XControl.class, control)).getPeer();
    }

}
