/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package com.sun.star.uno;

/**
 * The interface implemented by UNO environments.
 *
 * <p>With this interface, objects can be registered at and revoked from an
 * environment.</p>
 *
 * @see com.sun.star.uno.IBridge
 * @see com.sun.star.uno.IQueryInterface
 * @see com.sun.star.uno.UnoRuntime
 *
 * @deprecated As of UDK 3.2, this interface is deprecated, without offering a
 * replacement.
 */
public interface IEnvironment {
    /**
     * Gets the context of this environment.
     *
     * @return the context of this environment
     */
    Object getContext();

    /**
     * Gets the name of this environment.
     *
     * @return the name of this environment
     */
    String getName();

    /**
     * Registers one UNO interface facet of an object.
     *
     * <p>Such an object will typically be one of three things:
     * <ul>
     * <li>A local Java object, to be mapped out of this environment via a given
     *     bridge.</li>
     * <li>A proxy object, mapped into this environment via some bridge
     *     <var>B1</var>, and now to be mapped out of this environment via a
     *     given bridge <var>B2</var>.</li>
     * <li>A proxy object, created as a remote object is mapped into this
     *     environment via a given bridge.</li>
     * </ul>
     *
     * <p>The object actually registered may differ from the specified
     * <code>object</code> that is passed as an argument.  This enables an
     * environment to work in a multi-threaded scenario, where two threads can
     * call <code>registerInterface</code> for the same combination of
     * <code>oid</code> and <code>type</code> at the same time; the race
     * condition is solved by letting one of the calls register its argument
     * <code>object</code>, ignoring the argument <code>object</code> of the
     * other call, and letting both calls return the same
     * <code>object</code>.</p>
     *
     * <p>The registered object is held only weakly by the environment.  After a
     * call to <code>registerInterface</code>, a call to
     * <code>getRegisteredInterface</code> only succeeds as long as the
     * registered object is still strongly reachable, and the registered object
     * has not been explicitly revoked by calling
     * <code>revokeInterface</code>.</p>
     *
     * @param object the object to register; must be non-null
     * @param oid in-out parameter containing the OID of <code>object</code>.
     *    This must be a non-null reference to an array of length at least one;
     *    the zeroth element is used to pass the argument in and out.  If the
     *    zeroth element is null on input, the OID will be computed and passed
     *    out (that is, the zeroth element will never be null upon normal
     *    return).
     * @param type the UNO interface type to register.  This argument must be
     *    non-null, and must denote a UNO interface type.  The given
     *    <code>object</code> should implement this <code>type</code>.
     * @return the registered object (may differ from the <code>object</code>
     *     passed in); will never be null
     */
    Object registerInterface(Object object, String[] oid, Type type);

    /**
     * Explicitly revokes a UNO interface facet.
     *
     * <p>Calls to <code>registerInterface</code> and
     * <code>revokeInterface</code> must be paired.  A facet is only removed
     * from the environment when it has been revoked as often as it has been
     * registered.  This may change in the future, so that a facet would be
     * removed upon the first call to <code>revokeInterface</code> (and calls to
     * <code>revokeInterface</code> would no longer be necessary if the calling
     * code does not want to control the temporal extent of the
     * registration).</p>
     *
     * <p>It is not an error if the specified facet is not registered at this
     * environment (either because no corresponding object has ever been
     * registered, or it has been explicitly revoked, or it is no longer
     * strongly reachable).  In such a case, this method simply does
     * nothing.</p>
     *
     * @param oid the OID of the object to revoke; must be non-null
     * @param type the UNO interface type of the object to revoke.  This
     *     argument must be non-null, and must denote a UNO interface type.
     */
    void revokeInterface(String oid, Type type);

    /**
     * Retrieves a registered object, specified by OID and UNO interface type.
     *
     * @param oid the OID of the object to retrieve; must be non-null
     * @param type the UNO interface type of the object to retrieve.  This
     *     argument must be non-null, and must denote a UNO interface type.
     * @return the registered object, or null if none is found
     */
    Object getRegisteredInterface(String oid, Type type);

    /**
     * Retrieves the OID for a registered object.
     *
     * @param object a registered object; must be non-null
     * @return the OID of the <code>object</code>; will never be null
     */
    String getRegisteredObjectIdentifier(Object object);

    /**
     * Lists the registered objects to <code>System.out</code>.
     *
     * <p>This is for debug purposes.</p>
     */
    void list();
}
