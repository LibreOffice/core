/*************************************************************************
 *
 *  $RCSfile: Implementation.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sb $ $Date: 2002-10-07 13:16:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.comp.connections;

import com.sun.star.connection.ConnectionSetupException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

/**
 * Helper class for <code>Acceptor</code> and <code>Connector</code>.
 */
final class Implementation {
    /**
     * Instantiate a service for a given connection type.
     *
     * @param factory the service factory used to instantiate the requested
     *     service.
     * @param description has the following format:
     *     <code><var>type</var></code><!--
     *         -->*(<code><var>key</var>=<var>value</var></code>).
     *     The specific service implementation is instantiated through the
     *     service factory as
     *     <code>com.sun.star.connection.<var>type</var>service<var></var><!--
     *         --></code>
     *     (with <code><var>type</var></code> in lower case, and
     *     <code><var>service</var></code> either <code>Acceptor</code> or
     *     <code>Connector</code>).</p>
     * @param serviceClass the IDL interface type for which to query the
     *     requested service.
     * @param serviceType must be either <code>Acceptor</code> or
     *     <code>Connector</code>.
     * @return an instance of the requested service.  Never returns
     *     <code>null</code>.
     * @throws ConnectionSetupException if the requested service can not be
     *     found, or cannot be instantiated.
     */
    public static Object getConnectionService(XMultiServiceFactory factory,
                                              String description,
                                              Class serviceClass,
                                              String serviceType)
        throws ConnectionSetupException
    {
        int i = description.indexOf(',');
        String type
            = (i < 0 ? description : description.substring(0, i)).toLowerCase();
        Object service = null;
        try {
            service = UnoRuntime.queryInterface(
                serviceClass,
                factory.createInstance("com.sun.star.connection." + type
                                       + serviceType));
        } catch (RuntimeException e) {
            throw e;
        } catch (com.sun.star.uno.Exception e) {
        }
        if (service == null) {
            // As a fallback, also try to instantiate the service from the
            // com.sun.star.lib.connections package structure:
            try {
                service
                    = Class.forName("com.sun.star.lib.connections." + type
                                    + "." + type + serviceType).newInstance();
            } catch (ClassNotFoundException e) {
            } catch (IllegalAccessException e) {
            } catch (InstantiationException e) {
            }
        }
        if (service == null) {
            throw new ConnectionSetupException("no " + serviceType + " for "
                                               + type);
        }
        return service;
    }

    private Implementation() {} // do not instantiate
}
