/*************************************************************************
 *
 *  $RCSfile: java_remote_bridge.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kr $ $Date: 2000-09-28 11:47:06 $
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
 * @version     $Revision: 1.3 $ $ $Date: 2000-09-28 11:47:06 $
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



    private class MessageDispatcher extends Thread {
        boolean _quit = false;

        MessageDispatcher() {
            super("MessageDispatcher");
        }

        public void run() {
            try {
                do {
                    try {
                        // Use the protocol to read a job.
                        IMessage iMessage = _iProtocol.readMessage(_inputStream);

                        // Take care of special methods release and acquire
                        if(iMessage.getOperation() != null && iMessage.getOperation().equals("release")) {
                            _java_environment.revokeInterface(iMessage.getOid(), iMessage.getInterface());
                            remRefHolder(iMessage.getInterface(), iMessage.getOid());

                        }
                        else if(iMessage.getOperation() != null && iMessage.getOperation().equals("acquire")) {
                            String oid_o[] = new String[]{iMessage.getOid()};
                            _java_environment.registerInterface(null, oid_o, iMessage.getInterface());

                            addRefHolder(iMessage.getInterface(), iMessage.getOid());
                        }
                        else {
                            Object object = null;

                            if(iMessage.getOperation() != null) { // is it a request
                                object = _java_environment.getRegisteredInterface(iMessage.getOid(), iMessage.getInterface());

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
                if(!_quit) {
                    System.err.println(getClass() + " - reading message - exception occurred: \"" + eofException + "\"");
                    System.err.println(getClass() + " - giving up");
                }
            }
            catch(Exception exception) {
                System.err.println(getClass() + " - reading message - exception occurred: \"" + exception + "\"");
                System.err.println(getClass() + " - giving up");
                  if(DEBUG);
                    exception.printStackTrace();
            }

            // dispose this bridge only within an error
            if(!_quit && !java_remote_bridge.this._disposed)
                java_remote_bridge.this.dispose();
        }
    }


    protected XConnection       _xConnection;
    protected InputStream       _inputStream;       // wraps the connection to be an InputStream
    protected OutputStream      _outputStream;      // wraps the connection to be an OutputStream

    protected XInstanceProvider _xInstanceProvider;

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
        Class  _zInterface;
        String _oid;
        int    _mapCount;
    }


    void addRefHolder(Class zInterface, String oid) {
        acquire();

        RefHolder refHolder = (RefHolder)_refHolders.get(oid + zInterface);

        if(refHolder == null) {
            refHolder = new RefHolder();
            refHolder._zInterface = zInterface;
            refHolder._oid = oid;

            _refHolders.put(oid + zInterface, refHolder);
        }

        ++ refHolder._mapCount;
    }

    void remRefHolder(Class zInterface, String oid) {
        RefHolder refHolder = (RefHolder)_refHolders.get(oid + zInterface);

        if(refHolder != null) {
            -- refHolder._mapCount;
            if(refHolder._mapCount <= 0)
                _refHolders.remove(oid + zInterface);

            release();
        }
        else
            System.err.println(getClass().getName() + ".remRefHolder - warning - unknown oid:" + oid + " " + zInterface);
    }


    void freeHolders() {
        if(DEBUG) System.err.println("#### " + getClass().getName() + ".freeHolders:" + _refHolders.size());

        Enumeration elements = _refHolders.elements();
        while(elements.hasMoreElements()) {
            RefHolder refHolder = (RefHolder)elements.nextElement();

            while(refHolder._mapCount > 0) {
                -- refHolder._mapCount;

                _java_environment.revokeInterface(refHolder._oid, refHolder._zInterface);
                release();
            }
        }
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

        String protocolDescription = (String)args[0];

        Class protocol_class = Class.forName("com.sun.star.lib.uno.protocols." + protocolDescription + "." + protocolDescription);
        Constructor protocol_constructor = protocol_class.getConstructor(new Class[] {IBridge.class});

          _iProtocol          = (IProtocol)protocol_constructor.newInstance(new Object[]{this});
        _xConnection        = (XConnection)args[1];
        _xInstanceProvider  = (XInstanceProvider)args[2];
        _inputStream        = new XConnectionInputStream_Adapter(_xConnection);
        _outputStream       = new XConnectionOutputStream_Adapter(_xConnection);

        _java_environment   = java_environment;


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
     * @param      zInterface the interface under which is to be mapped
     * @see                   com.sun.star.uno.IBridge#mapInterfaceTo
     */
    public Object mapInterfaceTo(Object object, Class zInterface) throws MappingException {
        if(_disposed) throw new RuntimeException("java_remote_bridge(" + this + ").mapInterfaceTo - is disposed");

        String oid[] = new String[1];

        // if object is a string, than it is already mapped as a virtuell proxy
        if(object instanceof String)
            oid[0] = (String)object;
        else {
            Object xobject = _java_environment.registerInterface(object, oid, zInterface);
              if(!(xobject instanceof com.sun.star.lib.uno.environments.java.java_environment.HolderProxy))
                addRefHolder(zInterface, oid[0]);
        }
          if(DEBUG) System.err.println("##### " + getClass() + " - mapInterfaceTo:" + object + " interface:" + zInterface + " " + oid[0]);

        return oid[0];
    }

    /**
     * Maps an object from destination environment to the source environment.
     * <p>
     * @return     the object in the source environment
     * @param      object     the object to map
     * @param      zInterface the interface under which is to be mapped
     * @see                   com.sun.star.uno.IBridge#mapInterfaceFrom
     */
    public Object mapInterfaceFrom(Object oId, Class zInterface) throws MappingException    {
        if(_disposed) throw new RuntimeException("java_remote_bridge(" + this + ").mapInterfaceFrom - is disposed");

        // see if we already have object with zInterface of given oid
        Object object = _java_environment.getRegisteredInterface((String)oId, zInterface);
        if(object != null) {
            if(object instanceof DispatcherAdapterBase) {
                DispatcherAdapterBase dispatcherAdapterBase = (DispatcherAdapterBase)object;

                if(dispatcherAdapterBase.getObject() instanceof DispatcherAdapterBase) {
                    dispatcherAdapterBase = (DispatcherAdapterBase)dispatcherAdapterBase.getObject();

                    if(!(dispatcherAdapterBase.getObject() instanceof String)) { // is it not my object?
                        try {
                            sendRequest(oId, new Type(zInterface), "release", null, null);
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

            Object proxy = Proxy.create(this, oid[0], zInterface, false); // this proxy sends a release, when finalized
            object = _java_environment.registerInterface(proxy, oid, zInterface);
            acquire();
        }

          if(DEBUG) System.err.println("##### " + getClass() + " - mapInterfaceFrom:" + oId + " interface:" + zInterface + " "  + object);

        return object;
    }

//      public Object mapInterfaceFrom(Object oId, Class zInterface) throws MappingException    {
//          if(_disposed) throw new RuntimeException("java_remote_bridge(" + this + ").mapInterfaceFrom - is disposed");

//          String oid[] = new String[]{(String)oId};

//          Object proxy = Proxy.create(this, oid[0], zInterface, false); // this proxy sends a release, when finalized
//          Object object = _java_environment.registerInterface(proxy, oid, zInterface);
//          acquire();

//          if(DEBUG) System.err.println("##### " + getClass() + " - mapInterfaceFrom:" + oId + " interface:" + zInterface + " "  + object + " " + proxy);

//          return object;
//      }

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
    public void acquire() {
        ++ _life_count;
    }

    /**
     * Decreases the life count.
     * If the life count drops to zero, the bridge disposes itself.
     * <p>
     * @see com.sun.star.uno.IBridge#release
     */
    public void release() {
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

        synchronized(_xConnection) {
            _iProtocol.ignore_next_closeConnection();
            _iProtocol.send_closeConnection(_outputStream);
            try {
                _xConnection.flush();
            }
            catch(com.sun.star.io.IOException ioException) {
                throw new IOException(ioException.toString());
            }
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
                if(_life_count != 0)
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
        return "remote";
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

        synchronized(_xConnection) {
            _iProtocol.writeReply(exception, threadId, result);
            _iProtocol.flush(new DataOutputStream(_outputStream));

              _xConnection.flush();
        }
    }

    public Object sendRequest(Object object, Type type, String operation, Object params[], Boolean synchron[]) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".sendRequest:" + object + " " + type +" " + operation + " " + synchron);
        Object result = null;

        if(synchron == null)
            synchron = new Boolean[1];

        try {
            if(_disposed) throw new RuntimeException("java_remote_bridge(" + this + ").sendRequest - is disposed");

            if(operation.equals("acquire")) acquire();  // keep this bridge alife

            // is this what we realy want to do, is the writing to the stream realy protected? Not that
            // an other thread flushes the output and an reply arrives before we have added the thread queue!!!
            synchronized(_xConnection) {
                _iProtocol.writeRequest((String)object, type, operation, ThreadPool.getThreadId(), params, synchron);

                if(synchron[0].booleanValue()) // prepare a queue for this thread in the threadpool
                    ThreadPool.addThread(this);

                _iProtocol.flush(new DataOutputStream(_outputStream));
                _outputStream.flush();
            }
        }
          catch(Exception exception) {
            if(DEBUG) ;{
                System.err.println("##### " + getClass().getName() + ".sendRequest - exception occurred:" + exception);
                exception.printStackTrace();
            }
            ThreadPool.removeThread();

            throw exception;
        }

        _xConnection.flush();

          if(synchron[0].booleanValue()) {
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

