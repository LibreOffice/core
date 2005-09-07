/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConnectionDescriptor.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:54:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package com.sun.star.lib.connections.socket;

/**
 * Helper class for <code>socketAcceptor</code> and
 * <code>socketConnector</code>.
 *
 * <p>FIXME:  Once those classes have been moved from <code>jurt</code> to
 * <code>javaunohelper</code>, they should use
 * <code>com.sun.star.lib.uno.helper.UnoUrl</code> either instead of this class
 * or underneath this class.</p>
 */
final class ConnectionDescriptor {
    public ConnectionDescriptor(String description)
        throws com.sun.star.lang.IllegalArgumentException {
        for (int i = description.indexOf(','); i >= 0;) {
            int j = description.indexOf(',', i + 1);
            int k = j < 0 ? description.length() : j;
            int l = description.indexOf('=', i + 1);
            if (l < 0 || l >= k) {
                throw new com.sun.star.lang.IllegalArgumentException(
                    "parameter lacks '='");
            }
            String key = description.substring(i + 1, l);
            String value = description.substring(l + 1, k);
            if (key.equalsIgnoreCase("host")) {
                host = value;
            } else if (key.equalsIgnoreCase("port")) {
                try {
                    port = Integer.valueOf(value).intValue();
                } catch (NumberFormatException e) {
                    throw new com.sun.star.lang.IllegalArgumentException(
                        e.toString());
                }
                if (port < 0 || port > 65535) {
                    throw new com.sun.star.lang.IllegalArgumentException(
                        "port parameter must have value between 0 and 65535,"
                        + " inclusive");
                }
            } else if (key.equalsIgnoreCase("backlog")) {
                try {
                    backlog = Integer.valueOf(value).intValue();
                } catch (NumberFormatException e) {
                    throw new com.sun.star.lang.IllegalArgumentException(
                        e.toString());
                }
            } else if (key.equalsIgnoreCase("tcpnodelay")) {
                if (value.equals("0")) {
                    tcpNoDelay = Boolean.FALSE;
                } else if (value.equals("1")) {
                    tcpNoDelay = Boolean.TRUE;
                } else {
                    throw new com.sun.star.lang.IllegalArgumentException(
                        "tcpnodelay parameter must have 0/1 value");
                }
            }
            i = j;
        }
    }

    public String getHost() {
        return host;
    }

    public int getPort() {
        return port;
    }

    public int getBacklog() {
        return backlog;
    }

    public Boolean getTcpNoDelay() {
        return tcpNoDelay;
    }

    private String host = null;
    private int port = 6001;
    private int backlog = 50;
    private Boolean tcpNoDelay = null;
}
