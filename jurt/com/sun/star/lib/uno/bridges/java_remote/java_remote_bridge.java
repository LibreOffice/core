/*************************************************************************
 *
 *  $RCSfile: java_remote_bridge.java,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: kr $ $Date: 2001-03-12 15:40:09 $
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
import java.util.Hashtable;
import java.util.Vector;


import com.sun.star.lib.util.IInvokeHook;
import com.sun.star.lib.util.IInvokable;

import com.sun.star.lib.sandbox.generic.DispatcherAdapterBase;


import com.sun.star.bridge.XBridge;
import com.sun.star.bridge.XInstanceProvider;

import com.sun.star.comp.loader.FactoryHelper;

import com.sun.star.connection.XConnection;

import com.sun.star.lang.EventObject;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.lib.sandbox.Disposable;

import com.sun.star.lib.uno.environments.java.IRequester;
import com.sun.star.lib.uno.environments.java.Proxy;

import com.sun.star.lib.uno.environments.remote.IMessage;
import com.sun.star.lib.uno.environments.remote.IProtocol;
import com.sun.star.lib.uno.environments.remote.IReceiver;
import com.sun.star.lib.uno.environments.remote.Job;
import com.sun.star.lib.uno.environments.remote.ThreadID;
import com.sun.star.lib.uno.environments.remote.ThreadPool;

import com.sun.star.lib.uno.typedesc.TypeDescription;

import com.sun.star.lib.util.IStableObject;
import com.sun.star.lib.util.IStableListener;

import com.sun.star.registry.XRegistryKey;

import com.sun.star.uno.IBridge;
import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.MappingException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.Type;
import com.sun.star.uno.IQueryInterface;

/**
 * This class implements a remote bridge. Therefor
 * various interfaces are implemented.
 * <p>
 * The protocol to used is passed by name, the bridge
 * then looks for it under <code>com.sun.star.lib.uno.protocols</code>.
 * <p>
 * @version     $Revision: 1.17 $ $ $Date: 2001-03-12 15:40:09 $
 * @author      Kay Ramme
 * @see         com.sun.star.lib.uno.environments.remote.IProtocol
 * @since       UDK1.0
 */
public class java_remote_bridge implements IBridge, IReceiver, IRequester, XBridge, Disposable, XComponent, IStableObject {
    /**
     * When set to true, enables various debugging output.
     */
    static private final boolean DEBUG = false;

    /**
     * E.g. to get privleges for security managers, it is
     * possible to set a hook for the <code>MessageDispatcher</code> thread.
     */
    static public IInvokeHook __MessageDispatcher_run_hook;

    /**
     * The name of the service.
     * <p>
     * @deprecated as of UDK 1.0
     */
    static protected final String __serviceName = "com.sun.star.bridge.Bridge";

    /**
     * Gives a factory for creating the service.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @deprecated as of UDK 1.0
     * <p>
     * @return  returns a <code>XSingleServiceFactory</code> for creating the component
     * @param   implName     the name of the implementation for which a service is desired
     * @param   multiFactory the service manager to be uses if needed
     * @param   regKey       the registryKey
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static XSingleServiceFactory __getServiceFactory(String implName,
                                                            XMultiServiceFactory multiFactory,
                                                            XRegistryKey regKey)
    {
        XSingleServiceFactory xSingleServiceFactory = null;

        if (implName.equals(java_remote_bridge.class.getName()) )
            xSingleServiceFactory = FactoryHelper.getServiceFactory(java_remote_bridge.class,
                                                                    __serviceName,
                                                                    multiFactory,
                                                                    regKey);

        return xSingleServiceFactory;
    }

    /**
     * Writes the service information into the given registry key.
     * This method is called by the <code>JavaLoader</code>
     * <p>
     * @deprecated as of UDK 1.0
     * <p>
     * @return  returns true if the operation succeeded
     * @param   regKey       the registryKey
     * @see                  com.sun.star.comp.loader.JavaLoader
     */
    public static boolean __writeRegistryServiceInfo(XRegistryKey regKey) {
        return FactoryHelper.writeRegistryServiceInfo(java_remote_bridge.class.getName(), __serviceName, regKey);
    }



    public class MessageDispatcher extends Thread implements IInvokable {
        boolean _quit = false;

        private ThreadID _threadID;

        MessageDispatcher() {
            super("MessageDispatcher");
        }

        public void run() {
            _threadID = ThreadPool.getThreadId();

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
            try {
                do {
                    try {
                        // Use the protocol to read a job.
                        IMessage iMessage = _iProtocol.readMessage(_inputStream);


                        if(iMessage.getThreadID().equals(_threadID)) {
                            continue;
                        }

                        // Take care of special methods release and acquire
                        if(iMessage.getOperation() != null && iMessage.getOperation().equals("release")) {
                            _java_environment.revokeInterface(iMessage.getOid(), new Type(iMessage.getInterface()));
                            remRefHolder(new Type(iMessage.getInterface()), iMessage.getOid());

                            if(iMessage.mustReply())
                                sendReply(false, iMessage.getThreadID(), null);
                        }
                        else if(iMessage.getOperation() != null && iMessage.getOperation().equals("acquire")) {
                            String oid_o[] = new String[]{iMessage.getOid()};
                            _java_environment.registerInterface(null, oid_o, new Type(iMessage.getInterface()));

                            addRefHolder(new Type(iMessage.getInterface()), iMessage.getOid());
                        }
                        else {
                            Object object = null;

                            if(iMessage.getOperation() != null) { // is it a request
                                object = _java_environment.getRegisteredInterface(iMessage.getOid(), new Type(iMessage.getInterface()));

                                Object xexception = null;

                                try {
                                // if we don't have an object we ask the instance provider
                                    if(object == null && _xInstanceProvider != null)
                                        object = _xInstanceProvider.getInstance(iMessage.getOid());

                                    if(object == null)
                                        xexception = new com.sun.star.uno.RuntimeException("urp: no instance provider set and unknown object:" + iMessage.getOid());

                                }
                                catch(com.sun.star.container.NoSuchElementException noSuchElementException) {
                                    xexception = noSuchElementException;
                                }
                                catch(com.sun.star.uno.RuntimeException runtimeException) {
                                    xexception = runtimeException;
                                }

                                if(xexception != null) {
                                    // an exception occurred while trying to get an instance.
                                    // propagate it.
                                    sendReply(true, iMessage.getThreadID(), xexception);
                                    iMessage = null;
                                }
                            }

                            if(iMessage != null) {
                                // Queue the job for later execution.
                                // Give this bridge as the disposeId, needed in case of disposing this bridge
                                Job job = new Job(object, java_remote_bridge.this, iMessage);

                                ThreadPool.putJob(job, java_remote_bridge.this);
                                job = null;
                            }
                        }

                        iMessage = null;
                        // this is important to get rid of the job (especially while testing lifecycles)
                    }
                    catch(MappingException mappingException) {
                        System.err.println("MessageDispatcher - exception occurred:" + mappingException);
                    }
                }
                while(!_quit);
            }
            catch(EOFException eofException) {
                if(!_quit && DEBUG) {
                    System.err.println(getClass() + " - reading message - exception occurred: \"" + eofException + "\"");
                    System.err.println(getClass() + " - giving up");
                }
            }
            catch(Exception exception) {
                if(DEBUG) {
                    System.err.println(getClass() + " - reading message - exception occurred: \"" + exception + "\"");
                    exception.printStackTrace();
                    System.err.println(getClass() + " - giving up");
                }
                  if(DEBUG)
                    exception.printStackTrace();
            }

            // dispose this bridge only within an error
            if(!_quit && !java_remote_bridge.this._disposed)
                java_remote_bridge.this.dispose();

            return null;
        }
    }


    protected XConnection       _xConnection;
    protected InputStream       _inputStream;       // wraps the connection to be an InputStream
    protected OutputStream      _outputStream;      // wraps the connection to be an OutputStream

    protected XInstanceProvider _xInstanceProvider;

    protected String            _name = "remote";
    protected IProtocol         _iProtocol;
    protected IEnvironment      _java_environment;
    protected MessageDispatcher _messageDispatcher;
    protected int               _life_count = 0;    // determines if this bridge is alife, which is controlled by acquire and release calls
    protected boolean           _disposed = false;
    protected boolean           _disposing = false;

    protected Hashtable         _refHolders;        // holds descriptions for out mapped objects, so we can release
                                                    // the outmapped objects when the bridge is to be disposed

    protected Vector            _listeners;
    protected Vector            _stableListeners;

    protected boolean           _negotiate;
    protected boolean           _forceSynchronous;

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


    // use a static class, it is smaller
    private static class RefHolder {
        Type  _type;
        String _oid;
        int    _mapCount;
    }


    final void addRefHolder(Type type, String oid) {
        acquire();

        synchronized(_refHolders) {
            RefHolder refHolder = (RefHolder)_refHolders.get(oid + type);

            if(refHolder == null) {
                refHolder = new RefHolder();
                refHolder._type = type;
                refHolder._oid = oid;

                _refHolders.put(oid + type, refHolder);
            }

            ++ refHolder._mapCount;
        }
    }

    final void remRefHolder(Type type, String oid) {
        synchronized(_refHolders) {
            RefHolder refHolder = (RefHolder)_refHolders.get(oid + type);

            if(refHolder != null) {
                -- refHolder._mapCount;
                if(refHolder._mapCount <= 0)
                    _refHolders.remove(oid + type);

                release();
            }
            else
                System.err.println(getClass().getName() + ".remRefHolder - warning - unknown oid:" + oid + " " + type);
        }
    }


    final void freeHolders() {
        if(DEBUG) System.err.println("#### " + getClass().getName() + ".freeHolders:" + _refHolders.size());

        synchronized(_refHolders) {
        Enumeration elements = _refHolders.elements();
        while(elements.hasMoreElements()) {
            RefHolder refHolder = (RefHolder)elements.nextElement();

            while(refHolder._mapCount > 0) {
                -- refHolder._mapCount;

                _java_environment.revokeInterface(refHolder._oid, refHolder._type);
                release();
            }
        }
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
        _outputStream       = new XConnectionOutputStream_Adapter(_xConnection);

        if(args.length > 3)
            _name = (String)args[3];

        // be sure that all neccessary members are set
        if(_java_environment == null
        || _xConnection      == null
        || _iProtocol        == null
        || _inputStream      == null
        || _outputStream     == null)
            throw new com.sun.star.lang.IllegalArgumentException(getClass().getName());

        _refHolders       = new Hashtable();
        _listeners        = new Vector();
        _stableListeners  = new Vector();

        // create the message dispatcher and start it
          _messageDispatcher  = new MessageDispatcher();
        _messageDispatcher.start();
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
    public Object mapInterfaceTo(Object object, Type type) throws MappingException {
        if(_disposed) throw new RuntimeException("java_remote_bridge(" + this + ").mapInterfaceTo - is disposed");

        String oid[] = new String[1];

        // if object is a string, than it is already mapped as a virtuell proxy
        if(object instanceof String)
            oid[0] = (String)object;
        else {
            Object xobject = _java_environment.registerInterface(object, oid, type);
              if(!(xobject instanceof com.sun.star.lib.uno.environments.java.java_environment.HolderProxy))
                addRefHolder(type, oid[0]);
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
    public Object mapInterfaceFrom(Object oId, Type type) throws MappingException   {
        if(_disposed) throw new RuntimeException("java_remote_bridge(" + this + ").mapInterfaceFrom - is disposed");

        acquire();

        // see if we already have object with zInterface of given oid
        Object object = _java_environment.getRegisteredInterface((String)oId, type);

        if(object != null) {
            if(object instanceof DispatcherAdapterBase) {
                DispatcherAdapterBase dispatcherAdapterBase = (DispatcherAdapterBase)object;

                if(dispatcherAdapterBase.getObject() instanceof DispatcherAdapterBase) {
                    dispatcherAdapterBase = (DispatcherAdapterBase)dispatcherAdapterBase.getObject();

                    if((dispatcherAdapterBase.getObject() instanceof String)) { // is it my proxy?
                        try {
                            sendRequest(oId,
                                        type,
                                        "release",
                                        null,
                                        new Boolean[]{new Boolean(_forceSynchronous)},
                                          new Boolean[]{new Boolean(_forceSynchronous)});
                        }
                        catch(Exception exception) {
                            throw new MappingException(exception.getMessage());
                        }
                    }
                }
            }
        }
        else {
            String oid[] = new String[]{(String)oId};

            Object proxy = Proxy.create(this, oid[0], type, false, _forceSynchronous); // this proxy sends a release, when finalized
            object = _java_environment.registerInterface(proxy, oid, type);
        }

          if(DEBUG) System.err.println("##### " + getClass() + " - mapInterfaceFrom:" + oId + " interface:" + type + " "  + object);

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
    }

    /**
     * Decreases the life count.
     * If the life count drops to zero, the bridge disposes itself.
     * <p>
     * @see com.sun.star.uno.IBridge#release
     */
    public synchronized void release() {
        -- _life_count;

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".release:" + _life_count);

        try {
            if(_life_count <= 0)
                dispose();
        }
        catch(Exception exception) {
            System.err.println("######### " + getClass().getName() + ".release - exception:" + exception);
        }
    }

    /**
     * Resets the bridge, sends an procotol "closeConnection"
     * and ignores the next incoming procotol "closeConnection".
     * E.g. this is useful for inheriting connections on the server
     * side from one process to another.
     * Resets the proxy and stub refs.
     * <p>
     * @see com.sun.star.uno.IBridge#reset
     */
    public synchronized void reset() throws IOException {
        // we need to release the outmapped objects here
        freeHolders();

        // unfortunately we can not set the _life_count to zero
        // because there can be active proxies
//          _life_count = 0;

        synchronized(_outputStream) {
            _iProtocol.ignore_next_closeConnection();
            _iProtocol.send_closeConnection(_outputStream);
            _outputStream.flush();
        }
    }

    /**
     * Disposes the bridge. Sends belonging threads an interrupt exception.
     * Releases mapped objects.
     * <p>
     * @see com.sun.star.uno.IBridge#dispose
     */
    public synchronized void dispose() {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".dispose - life count:" + _life_count);

        if(_disposed) throw new RuntimeException("java_remote_bridge(" + this + ").dispose - is disposed");

        if(!_disposing) {
            _disposing = true;

            notifyListeners();
            notifyStableListeners();

            try {
                // stop the dispatcher thread
                _messageDispatcher._quit = true;

                // close the connection
                _xConnection.close();

                // interrupt the dispatcher thread, if this thread is not the dispatcher
                if(Thread.currentThread() != _messageDispatcher &&  _messageDispatcher.isAlive()) {

                    // THIS IS A ***WORKAROUND*** FOR LINUX SUN JDK1.3 PROBLEM:
                    // THE MESSAGEDISPATCHER STAYS IN THE SOCKET READ METHOD,
                    // EVEN IF THE SOCKET HAS BEEN CLOSED.
                    // SUSPENDING AND RESUMING THE MESSAGEDISPATCHER LET IT
                    // NOTICE THE CLOSED SOCKET
                    _messageDispatcher.suspend();
                    _messageDispatcher.resume();

                    _messageDispatcher.join(1000); // wait for thread to die

                    if(_messageDispatcher.isAlive()) { // has not died yet, interrupt it
                        _messageDispatcher.interrupt();
                        _messageDispatcher.join();
                    }
                }

                // interrupt all jobs queued by this bridge
                ThreadPool.dispose(this);

                // release all outmapped objects
                freeHolders();

                // see if life count is zero, if not give a warning
                if(_life_count != 0 && DEBUG)
                    System.err.println(getClass().getName() + ".dispose - life count (proxies left):" + _life_count);


                if(DEBUG)
                    _java_environment.list();


                // clear members
                _xConnection        = null;
                _java_environment   = null;
                _messageDispatcher  = null;

                _disposed = true;
            }
            catch(InterruptedException interruptedException) {
                System.err.println(getClass().getName() + ".dispose - InterruptedException:" + interruptedException);
            }
            catch(com.sun.star.io.IOException ioException) {
                System.err.println(getClass().getName() + ".dispose - IOException:" + ioException);
            }
        }
    }

    /**
     * Asks to map a remote object of name sInstanceName.
     * <p>
     * @param   sInstanceName   the name of the instance
     * @see     com.sun.star.bridge.XBridge#getInstance
     */
    public Object getInstance(String sInstanceName) throws com.sun.star.uno.RuntimeException {
        Object object = null;

        try {
            object = sendRequest(sInstanceName,
                                 new Type(XInterface.class),
                                 "queryInterface",
                                 new Object[]{new Type(XInterface.class)},
                                 null,
                                 null);
        }
        catch(Exception exception) {
            throw new com.sun.star.uno.RuntimeException(exception.getMessage());
        }

        return object;
    }

    /**
     * Gives the name of this bridge
     * <p>
     * @return  the name of this bridge
     * @see     com.sun.star.bridge.XBridge#getName
     */
    public String getName() throws com.sun.star.uno.RuntimeException {
        return _name;
    }

    /**
     * Gives a description of the connection type and protocol used
     * <p>
     * @return  connection type and protocol
     * @see     com.sun.star.bridge.XBridge#getDescription
     */
    public String getDescription() throws com.sun.star.uno.RuntimeException {
        return _iProtocol.getName() + "," + _xConnection.getDescription();
    }


    public void sendReply(boolean exception, ThreadID threadId, Object result) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".sendReply:" + exception + " " + result);

        if(_disposed) throw new RuntimeException("java_remote_bridge(" + this + ").sendReply - is disposed");

        synchronized(_outputStream) {
            _iProtocol.writeReply(exception, threadId, result);
            _iProtocol.flush(new DataOutputStream(_outputStream));

              _outputStream.flush();
        }
    }

    public Object sendRequest(Object object, Type type, String operation, Object params[], Boolean synchron[], Boolean mustReply[]) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".sendRequest:" + object + " " + type +" " + operation + " " + synchron + " " + mustReply);
        Object result = null;

        if(synchron == null)
            synchron = new Boolean[1];

        if(mustReply == null)
            mustReply = new Boolean[1];

        try {
            if(_disposed) throw new RuntimeException("java_remote_bridge(" + this + ").sendRequest - is disposed");

            if(operation.equals("acquire")) acquire();  // keep this bridge alife

            // is this what we realy want to do, is the writing to the stream realy protected? Not that
            // an other thread flushes the output and an reply arrives before we have added the thread queue!!!
            synchronized(_outputStream) {
                _iProtocol.writeRequest((String)object, (TypeDescription)type.getTypeDescription(), operation, ThreadPool.getThreadId(), params, synchron, mustReply);

                if(synchron[0].booleanValue()  && Thread.currentThread() != _messageDispatcher) // prepare a queue for this thread in the threadpool
                    ThreadPool.addThread(this);

                _iProtocol.flush(new DataOutputStream(_outputStream));
                _outputStream.flush();

            }
        }
          catch(Exception exception) {
            if(DEBUG) {
                System.err.println("##### " + getClass().getName() + ".sendRequest - exception occurred:" + exception);
                exception.printStackTrace();
            }
            ThreadPool.removeThread();

            throw exception;
        }

//          _xConnection.flush();

          if(synchron[0].booleanValue() && Thread.currentThread() != _messageDispatcher) { // the message dispatcher must not block
            result = ThreadPool.enter();
        }

        if(operation.equals("release"))
            release(); // kill this bridge, if this was the last proxy

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".sendRequest left:" + result);

        return result;
    }


    // Methods XComponent
    public void addEventListener(XEventListener xEventListener) throws com.sun.star.uno.RuntimeException {
        _listeners.addElement(xEventListener);
    }

    public void removeEventListener(XEventListener xEventListener) throws com.sun.star.uno.RuntimeException {
        _listeners.removeElement(xEventListener);
    }


    public void addStableListener(IStableListener stableListener) {
        _stableListeners.addElement(stableListener);
    }

    public void removeStableListener(IStableListener stableListener) {
        _stableListeners.removeElement(stableListener);
    }
}

