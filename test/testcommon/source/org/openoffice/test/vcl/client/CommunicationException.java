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
 * Exception that occurs when socket communication has a problem.
 *
 */
public class CommunicationException extends RuntimeException {

    /**
     *
     */
    private static final long serialVersionUID = 1L;

    public CommunicationException() {
        super();
    }

    public CommunicationException(String message, Throwable cause) {
        super(message, cause);
    }

    public CommunicationException(String message) {
        super(message);
    }

    public CommunicationException(Throwable cause) {
        super(cause);
    }

}
