/*************************************************************************
 *
 *  $RCSfile: java_remote_bridge.java,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: hr $ $Date: 2003-08-13 17:22:30 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.lib.uno.bridges.java_remote;


import java.io.IOException;
import java.io.DataOutputStream;
import java.io.EOFException;
import java.io.InputStream;
import java.io.OutputStream;

import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import java.util.Enumeration;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Map;
import java.util.Vector;


import com.sun.star.lib.util.IInvokeHook;
import com.sun.star.lib.util.IInvokable;

import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XInstanceProvider;

import com.sun.star.comp.loader.FactoryHelper;

import com.sun.star.connection.XConnection;

import com.sun.star.lang.EventObject;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.DisposedException;

import com.sun.star.lib.sandbox.Disposable;

import com.sun.star.lib.uno.environments.remote.IMessage;
import com.sun.star.lib.uno.environments.remote.IProtocol;
import com.sun.star.lib.uno.environments.remote.IReceiver;
import com.sun.star.lib.uno.environments.remote.Job;
import com.sun.star.lib.uno.environments.remote.ThreadId;
import com.sun.star.lib.uno.environments.remote.ThreadPoolFactory;
import com.sun.star.lib.uno.environments.remote.IThreadPool;

import com.sun.star.lib.uno.typedesc.TypeDescription;

import com.sun.star.lib.util.IStableObject;
import com.sun.star.lib.util.IStableListener;

import com.sun.star.registry.XRegistryKey;

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
 * @version     $Revision: 1.32 $ $ $Date: 2003-08-13 17:22:30 $
 * @author      Kay Ramme
 * @see         com.sun.star.lib.uno.environments.remote.IProtocol
 * @since       UDK1.0
 */
public class java_remote_bridge
    implements IBridge, IReceiver, RequestHandler, XBridge, Disposable,
        XComponent, IStableObject
{
    /**
     * When set to true, enables various debugging output.
     */
    static private final boolean DEBUG = false;

    /**
     * E.g. to get privleges for security managers, it is
     * possible to set a hook for the <code>MessageDispatcher</code> thread.
     */
    static public IInvokeHook __MessageDispatcher_run_hook;


    public class MessageDispatcher extends Thread implements IInvokable {
        private ThreadId _threadId;

        MessageDispatcher() {
            super("MessageDispatcher");
        }

        public void run() {
            _threadId = ThreadPoolFactory.getThreadId();

            if(__MessageDispatcher_run_hook != null) {
                try {
                    __MessageDispatcher_run_hook.invoke(this, null);
                }
                catch(Exception exception) { // should not fly
                    System.err.println(getClass().getName() + " - unexpected: method >invoke< threw an exception - " + exception);
                    exception.printStackTrace();
                }
            }
            else
                invoke(null);
        }

        public Object invoke(Object params[]) {
            Throwable throwable = null;

            try {
                for (;;) {
                    synchronized (this) {
                        if (terminate) {
                            break;
                        }
                    }

                    // Use the protocol to read a job.
                    IMessage iMessage = _iProtocol.readMessage(_inputStream);


                    if(iMessage.getThreadId().equals(_threadId)) {
                        continue;
                    }

                    // Take care of special methods release and acquire
                    String operation = iMessage.getOperation();
                    String oid = iMessage.getOid();
                    if(operation != null && operation.equals("release")) {
                        Type interfaceType = new Type(iMessage.getInterface());
                        _java_environment.revokeInterface(oid, interfaceType );
                        remRefHolder(interfaceType, oid);

                        if(iMessage.mustReply())
                            sendReply(false, iMessage.getThreadId(), null);
                    }
                    else if(operation != null && operation.equals("acquire")) {
                        Type interfaceType = new Type(iMessage.getInterface());
                        String oid_o[] = new String[]{oid};
                        _java_environment.registerInterface(null, oid_o, interfaceType );

                        addRefHolder(null, interfaceType, oid);
                    }
                    else {
                        Object object = null;

                        if(operation != null) { // is it a request
                            Type interfaceType = new Type(iMessage.getInterface());
                            object = _java_environment.getRegisteredInterface(oid, interfaceType);

                            Object xexception = null;

                            if(object == null) { // this is an unknown oid, so we may have to ask the XInstanceProvider
                                if(_xInstanceProvider == null) // we have neither an object nor an instance provider -> exception
                                    xexception = new com.sun.star.uno.RuntimeException(getClass().getName() + ".dispatch - no instance provider set and unknown object:" + oid);

                                else {
                                    try {
                                        object = _xInstanceProvider.getInstance(oid);

                                        if(object == null && !operation.equals("queryInterface"))
                                            xexception = new com.sun.star.uno.RuntimeException(
                                                getClass().getName()
                                                + ".dispatch: instance provider returned null and operation >"
                                                + operation
                                                + "< not supported on null");
                                    }
                                    catch(com.sun.star.container.NoSuchElementException noSuchElementException) {
                                        xexception = new com.sun.star.uno.RuntimeException(getClass().getName() + ".dispatch - wrapped exception:" + noSuchElementException);
                                    }
                                    catch(com.sun.star.uno.RuntimeException runtimeException) {
                                        xexception = runtimeException;
                                    }
                                }
                            }

                            if(xexception != null) {
                                // an exception occurred while trying to get an instance.
                                // propagate it.
                                sendReply(true, iMessage.getThreadId(), xexception);
                                iMessage = null;
                            }
                        }

                        if(iMessage != null) {
                                // Queue the job for later execution.
                                // Give this bridge as the disposeId, needed in case of disposing this bridge
                            Job job = new Job(object, java_remote_bridge.this, iMessage);

                            _iThreadPool.putJob(job);
                            job = null;
                        }
                    }

                    iMessage = null;
                    // this is important to get rid of the job (especially while testing lifecycles)
                }
            }
            catch(EOFException eofException) {
                if(DEBUG) {
                    System.err.println(getClass() + " - reading message - exception occurred: \"" + eofException + "\"");
                    System.err.println(getClass() + " - giving up");
                }

                throwable = new DisposedException( eofException.getMessage() );
            }
            catch(Exception exception) {
                if(DEBUG) {
                    System.err.println(getClass() + " - reading message - exception occurred: \"" + exception + "\"");
                    exception.printStackTrace();
                    System.err.println(getClass() + " - giving up");
                }
                  if(DEBUG)
                    exception.printStackTrace();

                throwable = new DisposedException( exception.getMessage() );
            }

            java_remote_bridge.this.dispose(throwable);

            return null;
        }

        public synchronized void terminate() {
            terminate = true;
        }

        // Variable terminate must only be used while synchronized on this
        // object:
        private boolean terminate = false;
    }


    protected XConnection       _xConnection;
    protected InputStream       _inputStream;       // wraps the connection to be an InputStream
    protected DataOutputStream      _outputStream;      // wraps the connection to be an OutputStream

    protected XInstanceProvider _xInstanceProvider;

    protected String            _name = "remote";
    protected IProtocol         _iProtocol;
    protected IEnvironment      _java_environment;
    protected MessageDispatcher _messageDispatcher;
    protected int               _life_count = 0;    // determines if this bridge is alife, which is controlled by acquire and release calls

    protected Vector            _listeners;
    protected Vector            _stableListeners;

    protected boolean           _negotiate;
    protected boolean           _forceSynchronous;

    protected IThreadPool       _iThreadPool;

    // Variable state must only be used while synchronized on this object:
    private int state = STATE_ALIVE;
    private static final int STATE_ALIVE = 0;
    private static final int STATE_DISPOSING = 1;
    private static final int STATE_DISPOSED = 2;

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

    private final HashMap refHolders = new HashMap();
        // from OID (String) to LinkedList of RefHolder

    private boolean hasRefHolder(String oid, Type type) {
        synchronized (refHolders) {
            LinkedList l = (LinkedList) refHolders.get(oid);
            if (l != null) {
                for (Iterator i = l.iterator(); i.hasNext();) {
                    RefHolder rh = (RefHolder) i.next();
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
            LinkedList l = (LinkedList) refHolders.get(oid);
            if (l == null) {
                l = new LinkedList();
                refHolders.put(oid, l);
            }
            boolean found = false;
            for (Iterator i = l.iterator(); !found && i.hasNext();) {
                RefHolder rh = (RefHolder) i.next();
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
            LinkedList l = (LinkedList) refHolders.get(oid);
            if (l != null) {
                for (Iterator i = l.iterator(); i.hasNext();) {
                    RefHolder rh = (RefHolder) i.next();
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
            for (Iterator i1 = refHolders.entrySet().iterator(); i1.hasNext();)
            {
                Map.Entry e = (Map.Entry) i1.next();
                String oid = (String) e.getKey();
                LinkedList l = (LinkedList) e.getValue();
                for (Iterator i2 = l.iterator(); i2.hasNext();) {
                    RefHolder rh = (RefHolder) i2.next();
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


    private String parseAttributes(String attributeList) {
        attributeList = attributeList.trim().toLowerCase();


        String protocol = null;

        int index = attributeList.indexOf(',');
        if(index >= 0) { // there are parameters
            protocol = attributeList.substring(0, index);
            attributeList = attributeList.substring(index + 1).trim();
        }
        else {
            protocol = attributeList;
            attributeList = "";
        }
        protocol = protocol.trim();

        boolean negotiateTouched = false;

        while(attributeList.length() > 0) {
            index = attributeList.indexOf(',');

            String word = null;

            if(index >= 0) {
                word = attributeList.substring(0, index).trim();
                attributeList = attributeList.substring(index + 1).trim();
            }
            else {
                word = attributeList.trim();
                attributeList = "";
            }

            String left = null;
            String right = null;

            index = word.indexOf('=');
            if(index >= 0) {
                left = word.substring(0, index).trim();
                right = word.substring(index + 1).trim();
            }
            else
                left = word;

            if(left.equals("negotiate")) {
                if(right != null)
                    _negotiate = (Integer.parseInt(right) == 1);
                else
                    _negotiate = true;

                negotiateTouched = true;
            }
            else if(left.equals("forcesynchronous")) {
                if(right != null)
                    _forceSynchronous = (Integer.parseInt(right) == 1);
                else
                    _forceSynchronous = true;

                if(_forceSynchronous && !negotiateTouched)
                    _negotiate = true;
            }
            else
                System.err.println(getClass().getName() + ".<init> - unknown attribute:" + left);
        }

        if(_negotiate)
            throw new com.sun.star.uno.RuntimeException("java_remote_bridge: negotiation not available yet, use negotiate=0 to disable");

        return protocol;
    }

    /**
     * Constructs a new bridge.
     * <p>
     * This method is not part of the provided <code>api</code>
     * and should only be used by the UNO runtime.
     * <p>
     * @param  java_remote        the source environment
     * @param  remote_environment the remote environement, which is not neede by this bridge
     * @param  args               the custom parameters: arg[0] == protocol_name, arg[1] == xConnection, arg[2] == xInstanceProvider
     */
    public java_remote_bridge(IEnvironment java_environment, IEnvironment remote_environment, Object args[]) throws Exception {
        if(DEBUG) System.err.println("#### " + getClass().getName() + " - instantiated:" + args);

        String protocol = parseAttributes((String)args[0]);

        _java_environment   = java_environment;

        Class protocol_class = Class.forName("com.sun.star.lib.uno.protocols." + protocol + "." + protocol);
        Constructor protocol_constructor = protocol_class.getConstructor(new Class[] {IBridge.class});

          _iProtocol          = (IProtocol)protocol_constructor.newInstance(new Object[]{this});
        _xConnection        = (XConnection)args[1];
        _xInstanceProvider  = (XInstanceProvider)args[2];
        _inputStream        = new XConnectionInputStream_Adapter(_xConnection);
        _outputStream       = new DataOutputStream( new XConnectionOutputStream_Adapter(_xConnection) );

        if(args.length > 3)
            _name = (String)args[3];

        // be sure that all neccessary members are set
        if(_java_environment == null
        || _xConnection      == null
        || _iProtocol        == null
        || _inputStream      == null
        || _outputStream     == null)
            throw new com.sun.star.lang.IllegalArgumentException(getClass().getName());

        _listeners        = new Vector();
        _stableListeners  = new Vector();

        proxyFactory = new ProxyFactory(this);

        // create the message dispatcher and start it
          _messageDispatcher  = new MessageDispatcher();
        _messageDispatcher.start();

        _iThreadPool = ThreadPoolFactory.createThreadPool();
    }


    private void notifyListeners() {
        EventObject eventObject = new EventObject(this);

        Enumeration elements = _listeners.elements();
        while(elements.hasMoreElements()) {
            XEventListener xEventListener = (XEventListener)elements.nextElement();

            try {
                xEventListener.disposing(eventObject);
            }
            catch(com.sun.star.uno.RuntimeException runtimeException) {
                // we are here not interested in any exceptions
            }
        }
    }

    private void notifyStableListeners() {
        EventObject eventObject = new EventObject(this);

        Enumeration elements = _stableListeners.elements();
        while(elements.hasMoreElements()) {
            IStableListener iStableListener = (IStableListener)elements.nextElement();

            iStableListener.deStable();
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

    /**
     * Maps an object from the source environment to the destination environment.
     * <p>
     * @return     the object in the destination environment
     * @param      object     the object to map
     * @param      type       the interface under which is to be mapped
     * @see                   com.sun.star.uno.IBridge#mapInterfaceTo
     */
    public Object mapInterfaceTo(Object object, Type type) {
        checkDisposed();

        String oid[] = new String[1];

        // if object is a string, than it is already mapped as a virtuell proxy
        if(object instanceof String)
            oid[0] = (String)object;
        else {
            object = _java_environment.registerInterface(object, oid, type);
            if (!proxyFactory.isProxy(object)) {
                addRefHolder(object, type, oid[0]);
            }
        }
          if(DEBUG) System.err.println("##### " + getClass() + " - mapInterfaceTo:" + object + " interface:" + type + " " + oid[0]);

        return oid[0];
    }

    /**
     * Maps an object from destination environment to the source environment.
     * <p>
     * @return     the object in the source environment
     * @param      object     the object to map
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
            try {
                sendRequest(oid, type, "release", null,
                            new Boolean[] { new Boolean(_forceSynchronous) },
                            new Boolean[] { new Boolean(_forceSynchronous) });
            } catch (Error e) {
                throw e;
            } catch (RuntimeException e) {
                throw e;
            } catch (Throwable e) {
                throw new com.sun.star.uno.RuntimeException(
                    getClass().getName() + ".mapInterfaceFrom - unexpected: "
                    + e);
            }
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
            dispose(new com.sun.star.uno.RuntimeException("end of life"));
        }
    }

    public void dispose() {
        dispose(new com.sun.star.uno.RuntimeException("user dispose"));
    }

    private void dispose(Throwable throwable) {
        synchronized (this) {
            if (state != STATE_ALIVE) {
                return;
            }
            state = STATE_DISPOSING;
        }

        notifyListeners();
        notifyStableListeners();

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
                // use this workaround for Linux JDK1.3.0 from Sun or Blackdown.
                // This workaround is dangerouse and may hardlock the VM.
                if (System.getProperty("os.name", "").toLowerCase().equals(
                        "linux")
                    && System.getProperty("java.version", "").startsWith(
                        "1.3.0")
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

            // release all outmapped objects
            freeHolders();

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

            // TODO!  Is it intended that state is left as STATE_DISPOSING when
            // an exception is thrown?
            synchronized (this) {
                state = STATE_DISPOSED;
            }
        } catch (InterruptedException e) {
            System.err.println(getClass().getName()
                               + ".dispose - InterruptedException:" + e);
        } catch (com.sun.star.io.IOException e) {
            System.err.println(getClass().getName() + ".dispose - IOException:"
                               + e);
        }
    }

    /**
     * Asks to map a remote object of name sInstanceName.
     * <p>
     * @param   sInstanceName   the name of the instance
     * @see     com.sun.star.bridge.XBridge#getInstance
     */
    public Object getInstance(String sInstanceName) {
        Object object = null;


        Type xInterface_type = null;
        try {
            xInterface_type = new Type(XInterface.class);
        }
        catch(Exception exception) {
              throw new com.sun.star.uno.RuntimeException(exception.getMessage());
          }


        try {
            object = sendRequest(sInstanceName,
                             xInterface_type,
                             "queryInterface",
                             new Object[]{xInterface_type},
                             null,
                             null);
        }
        catch(RuntimeException runtimeException) {
            throw runtimeException;
        }
        catch(Throwable throwable) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".getInstance - unexpected:" + throwable);
        }

        return object;
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
        return _iProtocol.getName() + "," + _xConnection.getDescription();
    }

    public void sendReply(boolean exception, ThreadId threadId, Object result) {
        if (DEBUG) {
            System.err.println("##### " + getClass().getName() + ".sendReply: "
                               + exception + " " + result);
        }

        checkDisposed();

        try {
            synchronized (_outputStream) {
                _iProtocol.writeReply(exception, threadId, result);
                _iProtocol.flush(_outputStream);
                _outputStream.flush();
            }
        } catch (Exception e) {
            dispose(e);
            throw new DisposedException(getClass().getName()
                                        + ".sendReply - unexpected: " + e);
        } catch (Error e) {
            dispose(e);
            throw e;
        }
    }

    public Object sendRequest(String oid, Type type, String operation,
                              Object[] args)
        throws Throwable
    {
        return sendRequest(
            oid, type, operation, args,
            _forceSynchronous ? new Boolean[] { Boolean.TRUE } : null,
            _forceSynchronous ? new Boolean[] { Boolean.TRUE } : null);
    }

    private Object sendRequest(Object object, Type type, String operation, Object params[], Boolean synchron[], Boolean mustReply[]) throws Throwable {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".sendRequest:" + object + " " + type +" " + operation + " " + synchron + " " + mustReply);
        Object result = null;

        if(synchron == null)
            synchron = new Boolean[1];

        if(mustReply == null)
            mustReply = new Boolean[1];

        checkDisposed();

        if(operation.equals("acquire")) acquire();  // keep this bridge alife

        boolean goThroughThreadPool = false;

        ThreadId threadId = ThreadPoolFactory.getThreadId();
        Object handle = null;
        try {
            synchronized(_outputStream) {
                _iProtocol.writeRequest((String)object, TypeDescription.getTypeDescription(type), operation, threadId , params, synchron, mustReply);

                goThroughThreadPool = synchron[0].booleanValue()  && Thread.currentThread() != _messageDispatcher;

                if(goThroughThreadPool) // prepare a queue for this thread in the threadpool
                    handle = _iThreadPool.attach( threadId );

                try {
                    _iProtocol.flush(_outputStream);
                    _outputStream.flush();
                }
                catch(IOException iOException) {
                    DisposedException disposedException =
                        new DisposedException( iOException.getMessage() );
                    dispose(disposedException);
                    throw disposedException;
                }
            }

            if(goThroughThreadPool)
                result = _iThreadPool.enter( handle, threadId);

        }
        finally {
            if(goThroughThreadPool)
                _iThreadPool.detach( handle , threadId);

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


    // Methods XComponent
    public void addEventListener(XEventListener xEventListener) {
        _listeners.addElement(xEventListener);
    }

    public void removeEventListener(XEventListener xEventListener) {
        _listeners.removeElement(xEventListener);
    }


    public void addStableListener(IStableListener stableListener) {
        _stableListeners.addElement(stableListener);
    }

    public void removeStableListener(IStableListener stableListener) {
        _stableListeners.removeElement(stableListener);
    }

    // This function must only be called while synchronized on this object:
    private synchronized void checkDisposed() {
        if (state == STATE_DISPOSED) {
            throw new DisposedException("java_remote_bridge " + this
                                        + " is disposed");
        }
    }

    private final ProxyFactory proxyFactory;
}
