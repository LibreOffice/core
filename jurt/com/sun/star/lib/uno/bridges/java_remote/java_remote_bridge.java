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
package com.sun.star.lib.uno.bridges.java_remote;


import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;


import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Map;

import com.sun.star.lib.util.DisposeListener;
import com.sun.star.lib.util.DisposeNotifier;

import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XInstanceProvider;

import com.sun.star.connection.XConnection;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.DisposedException;

import com.sun.star.lib.uno.environments.java.java_environment;
import com.sun.star.lib.uno.environments.remote.IProtocol;
import com.sun.star.lib.uno.environments.remote.IReceiver;
import com.sun.star.lib.uno.environments.remote.Job;
import com.sun.star.lib.uno.environments.remote.Message;
import com.sun.star.lib.uno.environments.remote.ThreadId;
import com.sun.star.lib.uno.environments.remote.ThreadPoolManager;
import com.sun.star.lib.uno.environments.remote.IThreadPool;

import com.sun.star.lib.uno.typedesc.MethodDescription;
import com.sun.star.lib.uno.typedesc.TypeDescription;


import com.sun.star.uno.IBridge;
import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.Any;

/**
 * This class implements a remote bridge. Therefor
 * various interfaces are implemented.
 * <p>
 * The protocol to used is passed by name, the bridge
 * then looks for it under <code>com.sun.star.lib.uno.protocols</code>.
 * <p>
 * @since       UDK1.0
 */
public class java_remote_bridge
    implements IBridge, IReceiver, RequestHandler, XBridge, XComponent,
        DisposeNotifier
{
    /**
     * When set to true, enables various debugging output.
     */
    static private final boolean DEBUG = false;

    private final class MessageDispatcher extends Thread {
        public MessageDispatcher() {
            super("MessageDispatcher");
        }

        public void run() {
            try {
                for (;;) {
                    synchronized (this) {
                        if (terminate) {
                            break;
                        }
                    }
                    Message msg = _iProtocol.readMessage();
                    Object obj = null;
                    if (msg.isRequest()) {
                        String oid = msg.getObjectId();
                        Type type = new Type(msg.getType());
                        int fid = msg.getMethod().getIndex();
                        if (fid == MethodDescription.ID_RELEASE) {
                            _java_environment.revokeInterface(oid, type);
                            remRefHolder(type, oid);
                            if (msg.isSynchronous()) {
                                sendReply(false, msg.getThreadId(), null);
                            }
                            continue;
                        }
                        obj = _java_environment.getRegisteredInterface(
                            oid, type);
                        if (obj == null
                            && fid == MethodDescription.ID_QUERY_INTERFACE)
                        {
                            if (_xInstanceProvider == null) {
                                sendReply(
                                    true, msg.getThreadId(),
                                    new com.sun.star.uno.RuntimeException(
                                        "unknown OID " + oid));
                                continue;
                            } else {
                                UnoRuntime.setCurrentContext(
                                    msg.getCurrentContext());
                                try {
                                    obj = _xInstanceProvider.getInstance(oid);
                                } catch (com.sun.star.uno.RuntimeException e) {
                                    sendReply(true, msg.getThreadId(), e);
                                    continue;
                                } catch (Exception e) {
                                    sendReply(
                                        true, msg.getThreadId(),
                                        new com.sun.star.uno.RuntimeException(
                                            e.toString()));
                                    continue;
                                } finally {
                                    UnoRuntime.setCurrentContext(null);
                                }
                            }
                        }
                    }
                    _iThreadPool.putJob(
                        new Job(obj, java_remote_bridge.this, msg));
                }
            } catch (Throwable e) {
                dispose(e);
            }
        }

        public synchronized void terminate() {
            terminate = true;
        }

        private boolean terminate = false;
    }

    protected XConnection       _xConnection;

    protected XInstanceProvider _xInstanceProvider;

    protected String            _name = "remote";
    private final String protocol;
    protected IProtocol         _iProtocol;
    protected IEnvironment      _java_environment;
    protected MessageDispatcher _messageDispatcher;
    protected int               _life_count = 0;    // determines if this bridge is alife, which is controlled by acquire and release calls

    private final ArrayList<XEventListener> _listeners = new ArrayList<XEventListener>();

    protected IThreadPool       _iThreadPool;

    // Variable disposed must only be used while synchronized on this object:
    private boolean disposed = false;

    /**
     * This method is for testing only.
     */
    int getLifeCount() {
        return _life_count;
    }

    /**
     * This method is for testing only.
     */
    IProtocol getProtocol() {
        return _iProtocol;
    }

    // The ref holder stuff strongly holds objects mapped out via this bridge
    // (the java_environment only holds them weakly).  When this bridge is
    // disposed, all remaining ref holder entries are released.

    private static final class RefHolder {
        public RefHolder(Type type, Object object) {
            this.type = type;
            this.object = object;
        }

        public Type getType() {
            return type;
        }

        public void acquire() {
            ++count;
        }

        public boolean release() {
            return --count == 0;
        }

        private final Type type;
        private final Object object;
        private int count = 1;
    }

    private final HashMap<String, LinkedList<RefHolder>> refHolders = new HashMap<String, LinkedList<RefHolder>>();
        // from OID (String) to LinkedList of RefHolder

    private boolean hasRefHolder(String oid, Type type) {
        synchronized (refHolders) {
            LinkedList<RefHolder> l = refHolders.get(oid);
            if (l != null) {
                for (Iterator<RefHolder> i = l.iterator(); i.hasNext();) {
                    RefHolder rh = i.next();
                    if (type.isSupertypeOf(rh.getType())) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    final void addRefHolder(Object obj, Type type, String oid) {
        synchronized (refHolders) {
            LinkedList<RefHolder> l = refHolders.get(oid);
            if (l == null) {
                l = new LinkedList<RefHolder>();
                refHolders.put(oid, l);
            }
            boolean found = false;
            for (Iterator<RefHolder> i = l.iterator(); !found && i.hasNext();) {
                RefHolder rh = i.next();
                if (rh.getType().equals(type)) {
                    found = true;
                    rh.acquire();
                }
            }
            if (!found) {
                l.add(new RefHolder(type, obj));
            }
        }
        acquire();
    }

    final void remRefHolder(Type type, String oid) {
        synchronized (refHolders) {
            LinkedList<RefHolder> l = refHolders.get(oid);
            if (l != null) {
                for (Iterator<RefHolder> i = l.iterator(); i.hasNext();) {
                    RefHolder rh = i.next();
                    if (rh.getType().equals(type)) {
                        try {
                            if (rh.release()) {
                                l.remove(rh);
                                if (l.isEmpty()) {
                                    refHolders.remove(oid);
                                }
                            }
                        } finally {
                            release();
                        }
                        break;
                    }
                }
            }
        }
    }

    final void freeHolders() {
        synchronized (refHolders) {
            for (Iterator<Map.Entry<String,LinkedList<RefHolder>>> i1 = refHolders.entrySet().iterator(); i1.hasNext();)
            {
                Map.Entry<String,LinkedList<RefHolder>> e = i1.next();
                String oid = e.getKey();
                LinkedList<RefHolder> l = e.getValue();
                for (Iterator<RefHolder> i2 = l.iterator(); i2.hasNext();) {
                    RefHolder rh = i2.next();
                    for (boolean done = false; !done;) {
                        done = rh.release();
                        _java_environment.revokeInterface(oid, rh.getType());
                        release();
                    }
                }
            }
            refHolders.clear();
        }
    }

    public java_remote_bridge(
        IEnvironment java_environment, IEnvironment remote_environment,
        Object[] args)
        throws Exception
    {
        _java_environment = java_environment;
        String proto = (String) args[0];
        _xConnection = (XConnection) args[1];
        _xInstanceProvider = (XInstanceProvider) args[2];
        if (args.length > 3) {
            _name = (String) args[3];
        }
        String attr;
        int i = proto.indexOf(',');
        if (i >= 0) {
            protocol = proto.substring(0, i);
            attr = proto.substring(i + 1);
        } else {
            protocol = proto;
            attr = null;
        }
        _iProtocol = (IProtocol) Class.forName(
            "com.sun.star.lib.uno.protocols." + protocol + "." + protocol).
            getConstructor(
                new Class[] {
                    IBridge.class, String.class, InputStream.class,
                    OutputStream.class }).
            newInstance(
                new Object[] {
                    this, attr,
                    new XConnectionInputStream_Adapter(_xConnection),
                    new XConnectionOutputStream_Adapter(_xConnection) });
        proxyFactory = new ProxyFactory(this, this);
        _iThreadPool = ThreadPoolManager.create();
        _messageDispatcher = new MessageDispatcher();
        _messageDispatcher.start();
        _iProtocol.init();
    }

    private void notifyListeners() {
        EventObject eventObject = new EventObject(this);

        Iterator<XEventListener> elements = _listeners.iterator();
        while(elements.hasNext()) {
            XEventListener xEventListener = elements.next();

            try {
                xEventListener.disposing(eventObject);
            }
            catch(com.sun.star.uno.RuntimeException runtimeException) {
                // we are here not interested in any exceptions
            }
        }
    }

    /**
     * Constructs a new bridge.
     * <p>
     * This method is not part of the provided <code>api</code>
     * and should only be used by the UNO runtime.
     * <p>
     * @deprecated as of UDK 1.0
     * <p>
     * @param  args               the custom parameters: arg[0] == protocol_name, arg[1] == xConnection, arg[2] == xInstanceProvider
     */
    public java_remote_bridge(Object args[]) throws Exception {
        this(UnoRuntime.getEnvironment("java", null), UnoRuntime.getEnvironment("remote", null), args);
    }

    // @see com.sun.star.uno.IBridge#mapInterfaceTo
    public Object mapInterfaceTo(Object object, Type type) {
        checkDisposed();
        if (object == null) {
            return null;
        } else {
            String[] oid = new String[1];
            object = _java_environment.registerInterface(object, oid, type);
            if (!proxyFactory.isProxy(object)) {
                // This branch must be taken iff object either is no proxy at
                // all or a proxy from some other bridge.  There are objects
                // that behave like objects for this bridge but that are not
                // detected as such by proxyFactory.isProxy.  The only known
                // case of such objects is com.sun.star.comp.beans.Wrapper,
                // which implements com.sun.star.lib.uno.Proxy and effectively
                // is a second proxy around a proxy that can be from this
                // bridge.  For that case, there is no problem, however:  Since
                // the proxies generated by ProxyFactory send each
                // queryInterface to the original object (i.e., they do not
                // short-circuit requests for a super-interface to themselves),
                // there will always be an appropriate ProxyFactory-proxy
                // registered at the _java_environment, so that the object
                // returned by _java_environment.registerInterface will never be
                // a com.sun.star.comp.beans.Wrapper.
                addRefHolder(object, type, oid[0]);
            }
            return oid[0];
        }
    }

    /**
     * Maps an object from destination environment to the source environment.
     * <p>
     * @return     the object in the source environment
     * @param      oId        the object to map
     * @param      type       the interface under which is to be mapped
     * @see                   com.sun.star.uno.IBridge#mapInterfaceFrom
     */
    public Object mapInterfaceFrom(Object oId, Type type) {
        checkDisposed();
        // TODO  What happens if an exception is thrown after the call to
        // acquire, but before it is guaranteed that a pairing release will be
        // called eventually?
        acquire();
        String oid = (String) oId;
        Object object = _java_environment.getRegisteredInterface(oid, type);
        if (object == null) {
            object = _java_environment.registerInterface(
                proxyFactory.create(oid, type), new String[] { oid }, type);
                // the proxy sends a release when finalized
        } else if (!hasRefHolder(oid, type)) {
            sendInternalRequest(oid, type, "release", null);
        }
        return object;
    }

    /**
     * Gives the source environment.
     * <p>
     * @return   the source environment of this bridge
     * @see      com.sun.star.uno.IBridge#getSourceEnvironment
     */
    public IEnvironment getSourceEnvironment() {
        return _java_environment;
    }

    /**
     * Gives the destination environment.
     * <p>
     * @return   the destination environment of this bridge
     * @see      com.sun.star.uno.IBridge#getTargetEnvironment
     */
    public IEnvironment getTargetEnvironment() {
        return null;
    }

    /**
     * Increases the life count.
     * <p>
     * @see com.sun.star.uno.IBridge#acquire
     */
    public synchronized void acquire() {
        ++ _life_count;

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".acquire:" + _life_count);
    }

    /**
     * Decreases the life count.
     * If the life count drops to zero, the bridge disposes itself.
     * <p>
     * @see com.sun.star.uno.IBridge#release
     */
    public void release() {
        boolean dispose;
        synchronized (this) {
            --_life_count;
            dispose = _life_count <= 0;
        }
        if (dispose) {
            dispose(new Throwable("end of life"));
        }
    }

    public void dispose() {
        dispose(new Throwable("user dispose"));
    }

    private void dispose(Throwable throwable) {
        synchronized (this) {
            if (disposed) {
                return;
            }
            disposed = true;
        }

        notifyListeners();
        for (Iterator<DisposeListener> i = disposeListeners.iterator(); i.hasNext();) {
            i.next().notifyDispose(this);
        }

        _iProtocol.terminate();

        try {
            _messageDispatcher.terminate();

            _xConnection.close();

            if (Thread.currentThread() != _messageDispatcher
                && _messageDispatcher.isAlive())
            {
                // This is a workaround for a Linux Sun JDK1.3 problem:  The
                // message dispatcher stays in the socket read method, even if
                // the socket has been closed.  Suspending and resuming the
                // message dispatcher lets it notice the closed socket.  Only
                // use this workaround for Linux JRE 1.3.0 and 1.3.1 from Sun
                // and Blackdown.  This workaround is dangerouse and may
                // hardlock the VM.
                if (System.getProperty("os.name", "").toLowerCase().equals(
                        "linux")
                    && System.getProperty("java.version", "").startsWith("1.3.")
                    && (System.getProperty("java.vendor", "").toLowerCase().
                            indexOf("sun") != -1
                        || System.getProperty("java.vendor", "").toLowerCase().
                            indexOf("blackdown") != -1))
                {
                    _messageDispatcher.suspend();
                    _messageDispatcher.resume();
                }

                _messageDispatcher.join(1000);
                if (_messageDispatcher.isAlive()) {
                    _messageDispatcher.interrupt();
                    _messageDispatcher.join();
                }
            }

            // interrupt all jobs queued by this bridge
            _iThreadPool.dispose(throwable);

            // release all out-mapped objects and all in-mapped proxies:
            freeHolders();
            // assert _java_environment instanceof java_environment;
            ((java_environment) _java_environment).revokeAllProxies();

            if (DEBUG) {
                if (_life_count != 0) {
                    System.err.println(getClass().getName()
                                       + ".dispose - life count (proxies left):"
                                       + _life_count);
                }
                _java_environment.list();
            }

            // clear members
            _xConnection        = null;
            _java_environment   = null;
            _messageDispatcher  = null;
        } catch (InterruptedException e) {
            System.err.println(getClass().getName()
                               + ".dispose - InterruptedException:" + e);
        } catch (com.sun.star.io.IOException e) {
            System.err.println(getClass().getName() + ".dispose - IOException:"
                               + e);
        }
    }

    // @see com.sun.star.bridge.XBridge#getInstance
    public Object getInstance(String instanceName) {
        Type t = new Type(XInterface.class);
        return sendInternalRequest(
            instanceName, t, "queryInterface", new Object[] { t });
    }

    /**
     * Gives the name of this bridge
     * <p>
     * @return  the name of this bridge
     * @see     com.sun.star.bridge.XBridge#getName
     */
    public String getName() {
        return _name;
    }

    /**
     * Gives a description of the connection type and protocol used
     * <p>
     * @return  connection type and protocol
     * @see     com.sun.star.bridge.XBridge#getDescription
     */
    public String getDescription() {
        return protocol + "," + _xConnection.getDescription();
    }

    public void sendReply(boolean exception, ThreadId threadId, Object result) {
        if (DEBUG) {
            System.err.println("##### " + getClass().getName() + ".sendReply: "
                               + exception + " " + result);
        }

        checkDisposed();

        try {
            _iProtocol.writeReply(exception, threadId, result);
        } catch (IOException e) {
            dispose(e);
            throw (DisposedException)
                (new DisposedException("unexpected " + e).initCause(e));
        } catch (RuntimeException e) {
            dispose(e);
            throw e;
        } catch (Error e) {
            dispose(e);
            throw e;
        }
    }

    public Object sendRequest(
        String oid, Type type, String operation, Object[] params)
        throws Throwable
    {
        Object result = null;

        checkDisposed();

        ThreadId threadId = _iThreadPool.getThreadId();
        Object handle = _iThreadPool.attach(threadId);
        try {
            boolean sync;
            try {
                sync = _iProtocol.writeRequest(
                    oid, TypeDescription.getTypeDescription(type), operation,
                    threadId, params);
            } catch (IOException e) {
                dispose(e);
                throw (DisposedException)
                    new DisposedException(e.toString()).initCause(e);
            }
            if (sync && Thread.currentThread() != _messageDispatcher) {
                result = _iThreadPool.enter(handle, threadId);
            }
        } finally {
            _iThreadPool.detach(handle, threadId);
            if(operation.equals("release"))
                release(); // kill this bridge, if this was the last proxy
        }

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".sendRequest left:" + result);

        // On the wire (at least in URP), the result of queryInterface is
        // transported as an ANY, but in Java it shall be transported as a
        // direct reference to the UNO object (represented as a Java Object),
        // never boxed in a com.sun.star.uno.Any:
        if (operation.equals("queryInterface") && result instanceof Any) {
            Any a = (Any) result;
            if (a.getType().getTypeClass() == TypeClass.INTERFACE) {
                result = a.getObject();
            } else {
                result = null; // should never happen
            }
        }

        return result;
    }

    private Object sendInternalRequest(
        String oid, Type type, String operation, Object[] arguments)
    {
        try {
            return sendRequest(oid, type, operation, arguments);
        } catch (Error e) {
            throw e;
        } catch (RuntimeException e) {
            throw e;
        } catch (Throwable e) {
            throw new RuntimeException("Unexpected " + e);
        }
    }

    // Methods XComponent
    public void addEventListener(XEventListener xEventListener) {
        _listeners.add(xEventListener);
    }

    public void removeEventListener(XEventListener xEventListener) {
        _listeners.remove(xEventListener);
    }

    // @see NotifyDispose.addDisposeListener
    public void addDisposeListener(DisposeListener listener) {
        synchronized (this) {
            if (!disposed) {
                disposeListeners.add(listener);
                return;
            }
        }
        listener.notifyDispose(this);
    }

    // This function must only be called while synchronized on this object:
    private synchronized void checkDisposed() {
        if (disposed) {
            throw new DisposedException("java_remote_bridge " + this
                                        + " is disposed");
        }
    }

    private final ProxyFactory proxyFactory;

    // Access to disposeListeners must be synchronized on <CODE>this</CODE>:
    private final ArrayList<DisposeListener> disposeListeners = new ArrayList<DisposeListener>();
}
