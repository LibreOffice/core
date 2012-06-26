/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.openoffice.test.vcl.client;

import java.util.logging.Level;
import java.util.logging.Logger;


/**
 *
 * The class is used to handle handshake package
 *
 */
public class Handshaker implements CommunicationListener, Constant {

    private static Logger logger = Logger.getLogger("com.ibm.vclhook");

    private CommunicationManager communicationManager = null;

    public Handshaker(CommunicationManager communicationManager) {
        this.communicationManager = communicationManager;
        this.communicationManager.addListener(this);
    }

    public void received(int headerType, byte[] header, byte[] data) {
        if (headerType == CH_Handshake) {
            int handshakeType = data[1] + ((data[0] & 255) << 8);
            switch (handshakeType) {
            case CH_REQUEST_HandshakeAlive:
                logger.log(Level.CONFIG, "Receive Handshake - CH_REQUEST_HandshakeAlive");
                sendHandshake(CH_RESPONSE_HandshakeAlive);
                break;
            case CH_REQUEST_ShutdownLink:
                logger.log(Level.CONFIG, "Receive Handshake - CH_REQUEST_ShutdownLink");
                sendHandshake(CH_ShutdownLink);
                break;
            case CH_ShutdownLink:
                logger.log(Level.CONFIG, "Receive Handshake - CH_ShutdownLink");
                communicationManager.stop();
                break;
            case CH_SetApplication:
                //String len
//              int len = data[2] + ((data[3] & 255) << 8);
//              String app = new String(data, 4, data.length - 4);
                logger.log(Level.CONFIG, "Receive Handshake - CH_SetApplication - app");
                //sendHandshake(CH_SetApplication);
                break;
            default:
            }
        }
    }
    public void sendHandshake(int handshakeType) {
        sendHandshake(handshakeType, new byte[0]);
    }
    public void sendHandshake(int handshakeType, byte[] data) {
        byte[] realData = new byte[data.length + 2];
        realData[0] = (byte) ((handshakeType >>> 8) & 0xFF);
        realData[1] = (byte) ((handshakeType >>> 0) & 0xFF);
        System.arraycopy(data, 0, realData, 2, data.length);
        communicationManager.sendPackage(CH_Handshake, null, realData);
    }

    public void start() {
    }

    public void stop() {
    }
}
