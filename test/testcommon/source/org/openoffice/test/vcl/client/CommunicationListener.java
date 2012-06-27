/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.openoffice.test.vcl.client;

/**
 * Callback when data package is arriving.
 *
 */
public interface CommunicationListener {

    public void start();

    public void received(int headerType, byte[] header, byte[] data);

    public void stop();
}
