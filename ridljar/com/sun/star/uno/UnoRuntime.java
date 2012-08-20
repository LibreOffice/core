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

import java.io.IOException;
import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.util.ArrayList;
import java.util.Iterator;
import com.sun.star.lib.uno.typedesc.TypeDescription;
import com.sun.star.lib.util.WeakMap;

/**
 * The central class needed for implementing or using UNO components in Java.
 *
 * <p>The methods <code>queryInterface</code> and <code>areSame</code> delegate
 * calls to the implementing objects and are used instead of casts,
 * <code>instanceof</code>, <code>==</code>, and <code>equals</code>.<p>
 *
 * <p>For historic reasons, this class is not <code>final</code>, and has a
 * <code>public</code> constructor.  These artifacts are considered mistakes,
 * which might be corrected in a future version of this class, so client code
 * should not rely on them.</p>
 *
 * @see com.sun.star.uno.IBridge
 * @see com.sun.star.uno.IEnvironment
 * @see com.sun.star.uno.IQueryInterface
 */
public class UnoRuntime {
    /**
     * @deprecated As of UDK&nbsp;3.2.0, do not create instances of this class.
     * It is considered a historic mistake to have a <code>public</code>
     * constructor for this class, which only has <code>static</code> members.
     * Also, this class might be changed to become <code>final</code> in a
     * future version.
     */
    public UnoRuntime() {}

    /**
     * Generates a world wide unique identifier string.
     *
     * <p>It is guaranteed that every invocation of this method generates a new
     * ID, which is unique within the VM.  The quality of &ldquo;world wide
     * unique&rdquo; will depend on the actual implementation, you should look
     * at the source to determine if it meets your requirements.</p>
     *
     * @return a unique <code>String</code>
     */
    public static String getUniqueKey() {
        synchronized (uniqueKeyLock) {
            if (uniqueKeyCount == Long.MAX_VALUE) {
                long time;
                for (time = System.currentTimeMillis(); time == uniqueKeyTime;)
                {
                    // Conservatively sleep for 100 millisecond to wait for
                    // System.currentTimeMillis() to change:
                    try {
                        Thread.sleep(100);
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                }
                uniqueKeyTime = time;
                uniqueKeyCount = Long.MIN_VALUE;
            }
            return uniqueKeyHostPrefix + Long.toString(uniqueKeyTime, 16) + ":"
                + Long.toString(uniqueKeyCount++, 16);
        }
    }

    /**
     * Generates a world wide unique object identifier (OID) for the given
     * Java object.
     *
     * <p>It is guaranteed that subsequent calls to this method with the same
     * Java object will give the same ID.</p>
     *
     * <p>This method is generally of little use for client code.  It should be
     * considered a mistake that this method is published at all.</p>
     *
     * @param object any object for which a OID shall be generated; must not be
     * <code>null</code>
     * @return the generated OID
     * @see com.sun.star.uno.IQueryInterface#getOid
     */
    public static String generateOid(Object object) {
        String oid = null;
        if (object instanceof IQueryInterface) {
            oid = ((IQueryInterface) object).getOid();
        }
        return oid == null ? object.hashCode() + oidSuffix : oid;
    }

    /**
     * Queries the given UNO object for the given UNO interface type.
     *
     * <p>This method returns <code>null</code> in case the given UNO object
     * does not support the given UNO interface type (or is itself
     * <code>null</code>).  Otherwise, a reference to a Java object implementing
     * the Java interface type corresponding to the given UNO interface is
     * returned.  In the latter case, it is unspecified whether the returned
     * Java object is the same as the given object, or is another facet of that
     * UNO object.</p>
     *
     * @param type the requested UNO interface type; must be a <code>Type</code>
     * object representing a UNO interface type
     * @param object a reference to any Java object representing (a facet of) a
     * UNO object; may be <code>null</code>
     * @return a reference to the requested UNO interface type if available,
     * otherwise <code>null</code>
     * @see com.sun.star.uno.IQueryInterface#queryInterface
     */
    public static Object queryInterface(Type type, Object object) {
        // Gracefully handle those situations where the passed in UNO object is
        // wrapped in an Any.  Strictly speaking, such a situation constitutes a
        // bug, but it is anticipated that such situations will arise quite
        // often in practice (especially since UNO Anys containing an XInterface
        // reference are not wrapped in a Java Any, but UNO Anys containing any
        // other interface reference are wrapped in a Java Any, which can lead
        // to confusion).
        if (object instanceof Any) {
            Any a = (Any) object;
            if (a.getType().getTypeClass() == TypeClass.INTERFACE) {
                object = a.getObject();
            }
        }
        if (object instanceof IQueryInterface) {
            object = ((IQueryInterface) object).queryInterface(type);
            if (object instanceof Any) {
                Any a = (Any) object;
                object = a.getType().getTypeClass() == TypeClass.INTERFACE
                    ? a.getObject() : null;
            }
        }
        // Ensure that the object implements the requested interface type:
        Class<?> c = type.getZClass();
        if (c == null || !c.isInstance(object)) {
            object = null;
        }
        return object;
    }

    /**
     * Queries the given UNO object for the given Java class (which must
     * represent a UNO interface type).
     *
     * @param zInterface a Java class representing a UNO interface type
     * @param object a reference to any Java object representing (a facet of) a
     * UNO object; may be <code>null</code>
     * @return a reference to the requested UNO interface type if available,
     * otherwise <code>null</code>
     * @see #queryInterface(Type, Object)
     */
    @SuppressWarnings("unchecked")
    public static <T> T queryInterface(Class<T> zInterface, Object object) {
        return (T) queryInterface(new Type(zInterface), object);
    }

    /**
       Tests two UNO <code>ANY</code> values for equality.

       <p>Two UNO values are <dfn>equal</dfn> if and only if they are of the
       same UNO type&nbsp;<var>t</var>, and they meet the following condition,
       depending on&nbsp;<var>t</var>:</p>
       <ul>
         <li>If <var>t</var> is a primitive type, then both values must denote
         the same element of the set of values of&nbsp;<var>t</var>.</li>

         <li>If <var>t</var> is a structured type, then both values must
         recursively contain corresponding values that are equal.</li>

         <li>If <var>t</var> is an interface type, then the two values must be
         either both null references, or both references to the same UNO
         object.</li>
       </ul>

       @param any1 a Java value representing a UNO <code>ANY</code> value.

       @param any2 a Java value representing a UNO <code>ANY</code> value.

       @return <code>true</code> if and only if the two arguments represent
       equal UNO values.
    */
    public static boolean areSame(Object any1, Object any2) {
        Any a1 = Any.complete(any1);
        Any a2 = Any.complete(any2);
        Type t = a1.getType();
        if (!a2.getType().equals(t)) {
            return false;
        }
        Object v1 = a1.getObject();
        Object v2 = a2.getObject();
        switch (t.getTypeClass().getValue()) {
        case TypeClass.VOID_value:
            return true;
        case TypeClass.BOOLEAN_value:
        case TypeClass.BYTE_value:
        case TypeClass.SHORT_value:
        case TypeClass.UNSIGNED_SHORT_value:
        case TypeClass.LONG_value:
        case TypeClass.UNSIGNED_LONG_value:
        case TypeClass.HYPER_value:
        case TypeClass.UNSIGNED_HYPER_value:
        case TypeClass.FLOAT_value:
        case TypeClass.DOUBLE_value:
        case TypeClass.CHAR_value:
        case TypeClass.STRING_value:
        case TypeClass.TYPE_value:
            return v1.equals(v2);
        case TypeClass.SEQUENCE_value:
            int n = Array.getLength(v1);
            if (n != Array.getLength(v2)) {
                return false;
            }
            for (int i = 0; i < n; ++i) {
                // Recursively using areSame on Java values that are (boxed)
                // elements of Java arrays representing UNO sequence values,
                // instead of on Java values that are representations of UNO ANY
                // values, works by chance:
                if (!areSame(Array.get(v1, i), Array.get(v2, i))) {
                    return false;
                }
            }
            return true;
        case TypeClass.ENUM_value:
            return v1 == v2;
        case TypeClass.STRUCT_value:
        case TypeClass.EXCEPTION_value:
            IFieldDescription[] fs;
            try {
                fs = TypeDescription.getTypeDescription(t).
                    getFieldDescriptions();
            } catch (ClassNotFoundException e) {
                throw new java.lang.RuntimeException(e.toString());
            }
            for (int i = 0; i< fs.length; ++i) {
                Type ft = new Type(fs[i].getTypeDescription());
                try {
                    // Recursively using areSame on Java values that are (boxed)
                    // fields of Java classes representing UNO struct or
                    // exception values, instead of on Java values that are
                    // representations of UNO ANY values, works by chance:
                    if (!areSame(
                            completeValue(ft, fs[i].getField().get(v1)),
                            completeValue(ft, fs[i].getField().get(v2))))
                    {
                        return false;
                    }
                } catch (IllegalAccessException e) {
                    throw new java.lang.RuntimeException(e.toString());
                }
            }
            return true;
        case TypeClass.INTERFACE_value:
            return v1 == v2
                || (v1 instanceof IQueryInterface
                    && ((IQueryInterface) v1).isSame(v2))
                || (v2 instanceof IQueryInterface
                    && ((IQueryInterface) v2).isSame(v1));
        default:
            throw new java.lang.RuntimeException(
                "com.sun.star.uno.Any has bad com.sun.star.uno.TypeClass");
        }
    }

    /**
       Complete a UNO value (make sure it is no invalid <code>null</code>
       value).

       <p>This is useful for members of parameterized type of instantiated
       polymorphic struct types, as <code>null</code> is a valid value there
       (and only there, for all types except <code>ANY</code> and interface
       types).</p>

       @param type a non-void, non-exception UNO type.

       @param value a Java value representing a UNO value of the given UNO type,
       or <code>null</code>.

       @return the given value, or the neutral value of the given type, if the
       given value was an invalid <code>null</code> value.

       @since UDK 3.2.3
    */
    public static final Object completeValue(Type type, Object value) {
        if (value != null) {
            return value;
        }
        switch (type.getTypeClass().getValue()) {
        case TypeClass.BOOLEAN_value:
            return Boolean.FALSE;
        case TypeClass.BYTE_value:
            return new Byte((byte) 0);
        case TypeClass.SHORT_value:
        case TypeClass.UNSIGNED_SHORT_value:
            return new Short((short) 0);
        case TypeClass.LONG_value:
        case TypeClass.UNSIGNED_LONG_value:
            return new Integer(0);
        case TypeClass.HYPER_value:
        case TypeClass.UNSIGNED_HYPER_value:
            return new Long(0L);
        case TypeClass.FLOAT_value:
            return new Float(0.0f);
        case TypeClass.DOUBLE_value:
            return new Double(0.0);
        case TypeClass.CHAR_value:
            return new Character('\u0000');
        case TypeClass.STRING_value:
            return "";
        case TypeClass.TYPE_value:
            return Type.VOID;
        case TypeClass.ANY_value:
        case TypeClass.INTERFACE_value:
            return null;
        case TypeClass.SEQUENCE_value:
            return Array.newInstance(type.getZClass().getComponentType(), 0);
        case TypeClass.STRUCT_value:
            try {
                return type.getZClass().getConstructor((Class[]) null).
                    newInstance((Object[]) null);
            } catch (java.lang.RuntimeException e) {
                throw e;
            } catch (java.lang.Exception e) {
                throw new java.lang.RuntimeException(e.toString());
            }
        case TypeClass.ENUM_value:
            try {
                return type.getZClass().getMethod("getDefault", (Class[]) null).
                    invoke(null, (Object[]) null);
            } catch (java.lang.RuntimeException e) {
                throw e;
            } catch (java.lang.Exception e) {
                throw new java.lang.RuntimeException(e.toString());
            }
        default:
            throw new IllegalArgumentException(
                "com.sun.star.uno.UnoRuntime.completeValue called with bad"
                + " com.sun.star.uno.Type");
        }
    }

    /**
     * Gets the current context of the current thread, or <code>null</code> if
     * no context has been set for the current thread.
     *
     * <p>The current context is thread local, which means that this method
     * returns the context that was last set for this thread.</p>
     *
     * @return the current context of the current thread, or <code>null</code>
     * if no context has been set for the current thread
     */
    public static XCurrentContext getCurrentContext() {
        return currentContext.get();
    }

    /**
     * Sets the current context for the current thread.
     *
     * <p>The current context is thread local.  To support a stacking behaviour,
     * every function that sets the current context should reset it to the
     * original value when exiting (for example, within a <code>finally</code>
     * block).</p>
     *
     * @param context the context to be set; if <code>null</code>, any
     * previously set context will be removed
    */
    public static void setCurrentContext(XCurrentContext context) {
        // optimize this by using Java 1.5 ThreadLocal.remove if context == null
        currentContext.set(context);
    }

    /**
     * Retrieves an environment of type <code>name</code> with context
     * <code>context</code>.
     *
     * <p>Environments are held weakly by this class.  If the requested
     * environment already exists, this methods simply returns it.  Otherwise,
     * this method looks for it under
     * <code>com.sun.star.lib.uno.environments.<var>name</var>.<!--
     * --><var>name</var>_environment</code>.</p>
     *
     * @param name the name of the environment
     * @param context the context of the environment
     * @see com.sun.star.uno.IEnvironment
     *
     * @deprecated As of UDK&nbsp;3.2.0, this method is deprecated, without
     * offering a replacement.
     */
    public static IEnvironment getEnvironment(String name, Object context)
        throws java.lang.Exception
    {
        synchronized (environments) {
            IEnvironment env = WeakMap.getValue(
                environments.get(name + context));
            if (env == null) {
                Class<?> c = Class.forName(
                    "com.sun.star.lib.uno.environments." + name + "." + name
                    + "_environment");
                Constructor<?> ctor = c.getConstructor(
                    new Class[] { Object.class });
                env = (IEnvironment) ctor.newInstance(new Object[] { context });
                environments.put(name + context, env);
            }
            return  env;
        }
    }

    /**
     * Gets a bridge from environment <code>from</code> to environment
     * <code>to</code>.
     *
     * <p>Creates a new bridge, if the requested bridge does not yet exist, and
     * hands the arguments to the bridge.</p>
     *
     * <p>If the requested bridge does not exist, it is searched for in package
     * <code>com.sun.star.lib.uno.bridges.<var>from</var>_<var>to</var>;</code>
     * and the root classpath as
     * <code><var>from</var>_<var>to</var>_bridge</code>.</p>
     *
     * @param from the source environment
     * @param to the target environment
     * @param args the initial arguments for the bridge
     * @return the requested bridge
     * @see #getBridgeByName
     * @see com.sun.star.uno.IBridge
     * @see com.sun.star.uno.IEnvironment
     *
     * @deprecated As of UDK&nbsp;3.2.0, this method is deprecated, without
     * offering a replacement.
     */
    public static IBridge getBridge(
        IEnvironment from, IEnvironment to, Object[] args)
        throws java.lang.Exception
    {
        synchronized (bridges) {
            String name = from.getName() + "_" + to.getName();
            String hashName = from.getName() + from.getContext() + "_"
                + to.getName() + to.getContext();
            IBridge bridge = WeakMap.getValue(bridges.get(hashName));
            if(bridge == null) {
                Class<?> zClass = null;
                String className =  name + "_bridge";
                try {
                    zClass = Class.forName(className);
                } catch (ClassNotFoundException e) {
                    className = "com.sun.star.lib.uno.bridges." + name + "."
                        + className;
                    zClass = Class.forName(className);
                }
                Class<?>[] signature = {
                    IEnvironment.class, IEnvironment.class, args.getClass() };
                Constructor<?> constructor = zClass.getConstructor(signature);
                Object[] iargs = { from, to, args };
                bridge = (IBridge) constructor.newInstance(iargs);
                bridges.put(hashName, bridge);
            }
            return bridge;
        }
    }

    /**
     * Gets a bridge from environment <code>from</code> to environment
     * <code>to</code>.
     *
     * <p>Creates a new bridge, if the requested bridge does not yet exist, and
     * hands the arguments to the bridge.</p>
     *
     * <p>If the requested bridge does not exist, it is searched for in package
     * <code>com.sun.star.lib.uno.bridges.<var>from</var>_<var>to</var>;</code>
     * and the root classpath as
     * <code><var>from</var>_<var>to</var>_bridge</code>.  The used environments
     * are retrieved through <code>getEnvironment</code>.</p>
     *
     * @param from the name of the source environment
     * @param fromContext the context for the source environment
     * @param to the name of the target environment
     * @param toContext the context for the target environment
     * @param args the initial arguments for the bridge
     * @return the requested bridge
     * @see #getBridge
     * @see #getEnvironment
     * @see com.sun.star.uno.IBridge
     * @see com.sun.star.uno.IEnvironment
     *
     * @deprecated As of UDK&nbsp;3.2.0, this method is deprecated, without
     * offering a replacement.
     */
    public static IBridge getBridgeByName(
        String from, Object fromContext, String to, Object toContext,
        Object[] args) throws java.lang.Exception
    {
        return getBridge(
            getEnvironment(from, fromContext), getEnvironment(to, toContext),
            args);
    }

    /**
     * Returns an array of all active bridges.
     *
     * @return an array of <code>IBridge</code> objects
     * @see com.sun.star.uno.IBridge
     *
     * @deprecated As of UDK&nbsp;3.2.0, this method is deprecated, without
     * offering a replacement.
     */
    public static IBridge[] getBridges() {
        ArrayList<Object> l = new ArrayList<Object>();
        synchronized (bridges) {
            for (Iterator<java.lang.ref.WeakReference<IBridge>> i = bridges.values().iterator(); i.hasNext();) {
                IBridge o = WeakMap.getValue(i.next());
                if (o != null) {
                    l.add(o);
                }
            }
        }
        return l.toArray(new IBridge[l.size()]);
    }

    /**
     * Gets a mapping from environment <code>from</code> to environment
     * <code>to</code>.
     *
     * <p>Mappings are like bridges, except that with mappings one can only map
     * in one direction.  Mappings are here for compatibility with the binary
     * UNO API.  Mappings are implemented as wrappers around bridges.</p>
     *
     * @param from the source environment
     * @param to the target environment
     * @return the requested mapping
     * @see com.sun.star.uno.IEnvironment
     * @see com.sun.star.uno.IMapping
     *
     * @deprecated As of UDK&nbsp;3.2.0, this method is deprecated, without
     * offering a replacement.
     */
    public static IMapping getMapping(IEnvironment from, IEnvironment to)
        throws java.lang.Exception
    {
        IBridge bridge;
        try {
            bridge = getBridge(from, to, null);
        }
        catch (ClassNotFoundException e) {
            bridge = new BridgeTurner(getBridge(to, from, null));
        }
        return new MappingWrapper(bridge);
    }

    /**
     * Gets a mapping from environment <code>from</code> to environment
     * <code>to</code>.
     *
     * <p>The used environments are retrieved through
     * <code>getEnvironment</code>.</p>
     *
     * @param from the name of the source environment
     * @param to the name of the target environment
     * @return the requested mapping
     * @see #getEnvironment
     * @see #getMapping
     * @see com.sun.star.uno.IMapping
     *
     * @deprecated As of UDK&nbsp;3.2.0, this method is deprecated, without
     * offering a replacement.
     */
    public static IMapping getMappingByName(String from, String to)
        throws java.lang.Exception
    {
        return getMapping(getEnvironment(from, null), getEnvironment(to, null));
    }

    /**
     * Resets this <code>UnoRuntime</code> to its initial state.
     *
     * <p>Releases all references to bridges and environments.</p>
     *
     * @deprecated As of UDK&nbsp;3.2.0, this method is deprecated, without
     * offering a replacement.
     */
    static public boolean reset() {
        synchronized (bridges) {
            for (Iterator<java.lang.ref.WeakReference<IBridge>> i = bridges.values().iterator(); i.hasNext();) {
                IBridge b = WeakMap.getValue(i.next());
                if (b != null) {
                    // The following call to dispose was originally made to
                    // com.sun.star.lib.sandbox.Disposable.dispose, which cannot
                    // throw an InterruptedException or IOException:
                    try {
                        b.dispose();
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupted();
                        throw new RuntimeException(
                            "Unexpected exception in UnoRuntime.reset: " + e);
                    } catch (IOException e) {
                        throw new RuntimeException(
                            "Unexpected exception in UnoRuntime.reset: " + e);
                    }
                }
            }
            bridges.clear();
        }
        environments.clear();
        return bridges.isEmpty() && environments.isEmpty();
    }

    /**
     * @deprecated As of UDK&nbsp;3.2.0, do not use this internal field.
     */
    static public final boolean DEBUG = false;

    private static final class BridgeTurner implements IBridge {
        public BridgeTurner(IBridge bridge) {
            this.bridge = bridge;
        }

        public Object mapInterfaceTo(Object object, Type type) {
            return bridge.mapInterfaceFrom(object, type);
        }

        public Object mapInterfaceFrom(Object object, Type type) {
            return bridge.mapInterfaceTo(object, type);
        }

        public IEnvironment getSourceEnvironment() {
            return bridge.getTargetEnvironment();
        }

        public IEnvironment getTargetEnvironment() {
            return bridge.getSourceEnvironment();
        }

        public void acquire() {
            bridge.acquire();
        }

        public void release() {
            bridge.release();
        }

        public void dispose() throws InterruptedException, IOException {
            bridge.dispose();
        }

        private final IBridge bridge;
    }

    private static final class MappingWrapper implements IMapping {
        public MappingWrapper(IBridge bridge) {
            this.bridge = bridge;
        }

        public Object mapInterface(Object object, Type type) {
            return bridge.mapInterfaceTo(object, type);
        }

        private final IBridge bridge;
    }

    private static final String uniqueKeyHostPrefix
    = Integer.toString(new Object().hashCode(), 16) + ":";
    private static final Object uniqueKeyLock = new Object();
    private static long uniqueKeyTime = System.currentTimeMillis();
    private static long uniqueKeyCount = Long.MIN_VALUE;

    private static final String oidSuffix = ";java[];" + getUniqueKey();

    private static final ThreadLocal<XCurrentContext> currentContext = new ThreadLocal<XCurrentContext>();

    private static final WeakMap<String,IEnvironment> environments = new WeakMap<String,IEnvironment>();
    private static final WeakMap<String,IBridge> bridges = new WeakMap<String,IBridge>();
}
