/*************************************************************************
 *
 *  $RCSfile: urp.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kr $ $Date: 2001-03-12 15:46:07 $
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

package com.sun.star.lib.uno.protocols.urp;

import java.lang.reflect.Array;

import java.io.IOException;
import java.io.DataInput;
import java.io.DataInputStream;
import java.io.DataOutput;
import java.io.InputStream;
import java.io.OutputStream;


import com.sun.star.container.NoSuchElementException;

import com.sun.star.uno.IBridge;

import com.sun.star.lib.uno.environments.remote.IMarshal;
import com.sun.star.lib.uno.environments.remote.IMessage;
import com.sun.star.lib.uno.environments.remote.Protocol;
import com.sun.star.lib.uno.environments.remote.IUnmarshal;
import com.sun.star.lib.uno.environments.remote.ThreadID;

import com.sun.star.lib.uno.typedesc.MethodDescription;
import com.sun.star.lib.uno.typedesc.TypeDescription;
//import com.sun.star.lib.uno.typeinfo.TypeInfo;

import com.sun.star.uno.Any;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Type;


/**
 * This class implements the complete urp protocol
 * from uno. The functionality is reachable through
 * the <code>IProtocol</code> interface.
 * <p>
 * @version     $Revision: 1.5 $ $ $Date: 2001-03-12 15:46:07 $
 * @author      Kay Ramme
 * @see         com.sun.star.lib.uno.environments.remote.IProtocol
 * @since       UDK1.0
 */
public class urp extends Protocol {
    /**
     * When set to true, enables various debugging output.
     */
    static public final boolean DEBUG = false;

    static private final TypeDescription __emptyTypeDescArray[] = new TypeDescription[0];
    static private final short           __cache_size = 256;


    protected IBridge           _iBridge;
    protected boolean           _bIgnoreNextCloseConnection = false;

    private String   _in_oid;
    private TypeDescription     _in_interface;
    private ThreadID _in_threadId;

    private String   _out_oid;
    private TypeDescription     _out_interface;
    private ThreadID _out_threadId;

    private int       _message_count;
    private boolean   _ignore_cache;
    private Marshal   _marshal;
    private Unmarshal _unmarshal;

    static private final byte BIG_HEADER   = (byte)0x80;
    // big header flags
    static private final byte REQUEST      = 0x40;
    static private final byte NEWTYPE      = 0x20;
    static private final byte NEWOID       = 0x10;
    static private final byte NEWTID       = 0x08;
    static private final byte LONGMETHODID = 0x04;
    static private final byte IGNORECACHE  = 0x02;
    static private final byte MOREFLAGS    = 0x01;

    // MOREFLAGS flags
    static private final byte MUSTREPLY    = (byte)0x80;
    static private final byte SYNCHRONOUSE = (byte)0x40;

    static private final byte DIR_MID      = 0x40;
    static private final byte EXCEPTION    = 0x20;



    public urp(IBridge iBridge) {
        _iBridge           = iBridge;

        _marshal = new Marshal(iBridge, __cache_size);
        _unmarshal = new Unmarshal(iBridge, __cache_size);
    }


    /**
     * Returns a string representation of this object.
     * <p>
     * @return   the string representation
     * @see      java.lang.Object#toString
     */
//      public String toString() {
//          return getName() + ": statistics - requests send:" + _requestsSend + " requests recieved:" + _requestsRecieved;
//      }

    /**
     * Gets the name of the protocol.
     * <p>
     * @result  the name of the protocol (iiop)
     * @see     com.sun.star.lib.uno.environments.remote.IProtocol#getName
     */
    public String getName() {
        return "urp";
    }

    /**
     * Tells the protocol to ignore the next <code>closeConnection</code>
     * meta request.
     * <p>
     * @see     com.sun.star.lib.uno.environments.remote.IProtocol#ignore_next_closeConnection
     */
    public void ignore_next_closeConnection() {
        _bIgnoreNextCloseConnection = true;
    }

    /**
     * Tells the protocol to send a <code>closeConnection</code>
     * meta request.
     * <p>
     * @param outputStream   the output stream
     * @see                  com.sun.star.lib.uno.environments.remote.IProtocol#send_closeConnection
     */
    public void send_closeConnection(OutputStream outputStream) throws IOException {
    }

    private Object readReply(byte header, boolean exception[]) throws Exception {
        if((header & NEWTID) != 0) // new thread id ?
            _in_threadId = _unmarshal.readThreadID();

        // get the out signature and parameter array of the reply
        Object objects[] = (Object[])removePendingRequest(_in_threadId);
        Object param[] = (Object[])objects[0];
        TypeDescription signature[] = (TypeDescription[])objects[1];
        TypeDescription resultType = (TypeDescription)objects[2];

        exception[0] = (header & EXCEPTION) != 0;
        if(exception[0]) {// Exception? So the reply has an any as the result
            signature = __emptyTypeDescArray;
            resultType = TypeDescription.__any_TypeDescription;
        }

        // read the result object
        Object result = null;
        if(resultType != null)
            result = _unmarshal.readObject(resultType);

        // read the out parameters
        for(int i = 0; i < signature.length; ++ i) {
            if(signature[i] != null) // is this an out parameter
                Array.set(param[i], 0, _unmarshal.readObject(signature[i].getComponentType()));
        }

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readReply:" + result);

        return result;
    }


    private Object []readParams(MethodDescription methodDescription) throws Exception {
        TypeDescription in_sig[] = methodDescription.getInSignature();
        TypeDescription out_sig[] = methodDescription.getOutSignature();

        Object params[] = new Object[in_sig.length];

        for(int i = 0; i < params.length; ++ i) {
            if(in_sig[i] != null) // is it an in parameter?
                if(out_sig[i] != null) {// is it also an out -> inout?
                    Object inout = Array.newInstance(out_sig[i].getComponentType().getZClass(), 1);
                    Array.set(inout, 0, _unmarshal.readObject(out_sig[i].getComponentType()));
                    params[i] = inout;
                }
                else  // it is only an in parameter
                    params[i] = _unmarshal.readObject(in_sig[i]);
            else // it is only an out parameter, so provide the holder
                params[i] = Array.newInstance(out_sig[i].getComponentType().getZClass(), 1);
        }

        return params;
    }


    private void readShortRequest(byte header, String operation[], Object param[][], boolean synchron[]) throws Exception  {
        ++ _requestsRecieved;

        int methodId = 0;

        if((header & DIR_MID) != 0)
            methodId = (((header & 0x3f) << 8) | _unmarshal.readbyte()) & 0x3fff;
        else
            methodId = (header & 0x3f);

        MethodDescription methodDescription = _in_interface.getMethodDescription(methodId);
        operation[0] = methodDescription.getName();

        synchron[0] = !methodDescription.isOneway();

        param[0] = readParams(methodDescription);

        if(synchron[0]) { // if the request is synchron, it is pending
            putPendingReply(_in_threadId, new Object[]{param[0], methodDescription.getOutSignature(), methodDescription.getReturnSig()/*, mMDesc*/});
        }

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readShortRequest:" + _in_oid + " " + operation[0] + " " + synchron[0]);
    }

    private void readLongRequest(byte header, String operation[], Object param[][], boolean synchron[], boolean mustReply[]) throws Exception  {
        ++ _requestsRecieved;

        // read the extended flags
        if((header & MOREFLAGS) != 0) {// is there an extended flags byte?
            byte exFlags = _unmarshal.readbyte();

            mustReply[0] = (exFlags & MUSTREPLY) != 0;
            synchron[0]  = (exFlags & SYNCHRONOUSE) != 0;
        }

        // read the method id
        int methodId = 0;
        if((header & LONGMETHODID) != 0) // usigned short ?
            methodId = _unmarshal.readshort();
        else
            methodId = _unmarshal.readbyte();

        if((header & NEWTYPE) != 0)
            _in_interface = _unmarshal.readTypeDescription();

        MethodDescription methodDescription = _in_interface.getMethodDescription(methodId);

        if((header & MOREFLAGS) == 0) // no ex flags, so get info from typeinfo
            synchron[0] = !methodDescription.isOneway();

        if((header & NEWOID) != 0) // new oid?
            _in_oid = _unmarshal.readOid();

        if((header & NEWTID) != 0) // new thread id ?
            _in_threadId = _unmarshal.readThreadID();

        _ignore_cache = ((header & IGNORECACHE) != 0); // do not use cache for this request?

        operation[0] = methodDescription.getName();

        param[0] = readParams(methodDescription);

        if(synchron[0]) // if the request is synchron, the reply is pending
            putPendingReply(_in_threadId, new Object[]{param[0], methodDescription.getOutSignature(), methodDescription.getReturnSig()/*, mMDesc*/});

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readLongRequest:" + _in_oid + " " + operation[0] + " " + synchron[0]);
    }

    private Object readMessage(String operation[], Object param[], boolean synchron[], boolean mustReply[], boolean exception[]) throws Exception {
        byte header = _unmarshal.readbyte();

        Class signature[];
        int methodId;
        Object result = null;

        if((header & BIG_HEADER) != 0) { // full header?
            if((header & REQUEST) != 0) // a request ?
                readLongRequest(header, operation, (Object [][])param, synchron, mustReply);
            else // a reply
                result = readReply(header, exception);
        }
        else // only a short request header
            readShortRequest(header, operation, (Object[][])param, synchron);

        if(synchron[0]) // synchron implies MUSTREPLY
            mustReply[0] = true;


        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readMessage:" + _in_oid + " " + operation[0] + " " + _in_threadId + " " + param[0] + " " + result);

        return result;
    }

    public void writeRequest(String oid,
                             TypeDescription zInterface,
                             String operation,
                             ThreadID threadId,
                             Object params[],
                             Boolean synchron[],
                             Boolean mustReply[]) throws Exception
   {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeRequest:" + oid + " " + zInterface + " " + operation);

        ++ _requestsSend;
        ++ _message_count;

        MethodDescription methodDescription = zInterface.getMethodDescription(operation);

        byte header = 0;
        boolean bigHeader = false;

        if(_out_oid == null || !oid.equals(_out_oid)) { // change the oid?
            header |= NEWOID;

            _out_oid = oid;
            bigHeader = true;
        }
        else
            oid = null;


        if(_out_interface == null || !_out_interface.equals(zInterface)) { // change interface
            header |= NEWTYPE;

            _out_interface = zInterface;
            bigHeader = true;
        }
        else
            zInterface = null;

        if(_out_threadId == null || !_out_threadId.equals(threadId)) { // change thread id
            header |= NEWTID;

            _out_threadId = threadId;

            bigHeader = true;
        }
        else
            threadId = null;

        boolean hasExFlags = false;

        // if synchron is provided, test if it differs from declaration
        if(synchron[0] != null) {
            if(methodDescription.isOneway() == synchron[0].booleanValue()) {
                bigHeader = true;
                hasExFlags = true;
            }
        }
        else
            synchron[0] = new Boolean(!methodDescription.isOneway());

        // if mustReply is provided and if it differs from synchron
        // then we have to write it
        if(mustReply[0] != null && (mustReply[0] != synchron[0])) {
            bigHeader = true;
            hasExFlags = true;
        }
        else
            mustReply[0] = synchron[0];

        if(bigHeader) { // something has changed, send big header
            header |= BIG_HEADER; // big header
            header |= REQUEST;
            header |= hasExFlags ? MOREFLAGS : 0;

            if(methodDescription.getIndex() > 255)
                header |= LONGMETHODID;

            _marshal.writebyte(header);

            if(hasExFlags) {// are there extended flags to write?
                byte exFlags = 0;

                exFlags |= synchron[0].booleanValue() ? SYNCHRONOUSE : 0;
                exFlags |= mustReply[0].booleanValue() ? MUSTREPLY : 0;

                _marshal.writebyte(exFlags);
            }

            // write the method id
            if(methodDescription.getIndex() > 255)
                _marshal.writeshort((short)methodDescription.getIndex());
            else
                _marshal.writebyte((byte)methodDescription.getIndex());

            if(zInterface != null) // has the interface changed? -> write it
                _marshal.writeTypeDescrption(zInterface);

            if(oid != null) // has the oid changed? -> write it
                _marshal.writeOid(_out_oid);

            if(threadId != null) // has the thread id changed? -> write it
                _marshal.writeThreadID(threadId);
        }
        else { // simple request
            if(methodDescription.getIndex() <= 0x2f) // does the method id fit in the header?
                _marshal.writebyte((byte)methodDescription.getIndex());
            else { // no
                header |= DIR_MID;
                header |= methodDescription.getIndex() >> 8;

                _marshal.writebyte(header);
                _marshal.writebyte((byte)(methodDescription.getIndex() & 0xff));
            }
        }

        // write the in parameters
        TypeDescription in_sig[] = methodDescription.getInSignature();
        TypeDescription out_sig[] = methodDescription.getOutSignature();
        for(int i = 0; i < in_sig.length; ++ i) {
            if(in_sig[i] != null) { // is it an in parameter?
                if(out_sig[i] != null)  // is it also an out parameter?
                    _marshal.writeObject(out_sig[i].getComponentType(), ((Object [])params[i])[0]);
                else // in only
                    _marshal.writeObject(in_sig[i], params[i]);
            }
        }

        if(synchron[0].booleanValue()) // if we are waiting for a reply, the reply is pending
            putPendingRequest(_out_threadId, new Object[]{params, out_sig, methodDescription.getReturnSig()});
    }

    public void writeReply(boolean exception, ThreadID threadId, Object result) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeReply:" + exception + " " + threadId + " " + result);

        ++ _message_count;

        Object objects[] = (Object[])removePendingReply(threadId);
        Object params[] = (Object[])objects[0];
        TypeDescription signature[] = (TypeDescription[])objects[1];
        TypeDescription resType     = (TypeDescription)objects[2];

        byte header = BIG_HEADER; // big header

        if(exception) { // has an exception occurred?
            header |= EXCEPTION;

            signature = __emptyTypeDescArray;
            resType = TypeDescription.__any_TypeDescription;
        }

        if(_out_threadId == null || !_out_threadId.equals(threadId)) { // change thread id ?
            header |= NEWTID;

            _out_threadId = threadId;
        }
        else
            threadId = null;

        _marshal.writebyte(header);

        if(threadId != null) // has the thread id changed? -> write it
            _marshal.writeThreadID(threadId);

        // write the result
        _marshal.writeObject(resType, result);

        // write the out parameters
        for(int i = 0; i < signature.length; ++ i)
            if(signature[i] != null)
                _marshal.writeObject(signature[i].getComponentType(), Array.get(params[i], 0));
    }


    private byte []readBlock(DataInput dataInput) throws Exception {
        int size = dataInput.readInt();
        int message_count = dataInput.readInt();

        byte bytes[] = new byte[size];

        dataInput.readFully(bytes);

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readBlock: size:" + size + " message count:" + message_count);

        return bytes;
    }

    private void writeBlock(DataOutput  dataOutput, byte bytes[], int message_count) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeBlock: size:" + bytes.length + " message_count:" + message_count);

          if(message_count != 1)
              System.err.println("##### " + getClass().getName() + ".writeBlock: size:" + bytes.length + " message_count:" + message_count);

        dataOutput.writeInt(bytes.length);
        dataOutput.writeInt(message_count);

        dataOutput.write(bytes);
    }

    static class Message implements IMessage {
        String    _oid;
        Object    _result;
        TypeDescription      _typeDescription;
        String    _operation;
        ThreadID  _threadId;
        boolean   _synchron;
        boolean   _mustReply;
        boolean   _exception;
        Object    _params[];

        Message(String    oid,
                Object    result,
                TypeDescription      typeDescription,
                String    operation,
                ThreadID  threadId,
                boolean   synchron,
                boolean   mustReply,
                boolean   exception,
                Object    params[])
        {
            _oid       = oid;
            _result    = result;
            _typeDescription      = typeDescription;
            _operation = operation;
            _threadId  = threadId;
            _synchron  = synchron;
            _mustReply = mustReply;
            _exception = exception;
            _params    = params;
        }

        public String getOperation() {
            return _operation;
        }

        public ThreadID getThreadID() {
            return _threadId;
        }

        public TypeDescription getInterface() {
            return _typeDescription;
        }

        public boolean isSynchron() {
            return _synchron;
        }

        public boolean mustReply() {
            return _mustReply;
        }

        public boolean isException() {
            return _exception;
        }

        public String getOid() {
            return _oid;
        }

        public Object getData(Object params[][]) throws Exception {
            params[0]    = _params;

            return _result;
        }
    }

    /**
     * reads a job from the given stream.
     * <p>
     * @return  thread read job.
     * @see     com.sun.star.lib.uno.environments.remote.Job
     * @see                  com.sun.star.lib.uno.environments.remote.IProtocol#readJob
     */
    public IMessage readMessage(InputStream inputStream) throws Exception {
        IMessage iMessage = null;

          while(iMessage == null) { // try hard to get a message
            if(_unmarshal.bytesLeft() <= 0) { // the last block is empty, get a new one
                byte bytes[] = readBlock(new DataInputStream(inputStream));
                _unmarshal.reset(bytes);
            }

            if(_unmarshal.bytesLeft() == 0) {// we already got a new block and there are still no bytes left? -> close or ignore
                // (i hope we can rid of this in the nead future
                if(!_bIgnoreNextCloseConnection)
                    inputStream.close();

                throw new java.io.IOException("connection close message received");
            }
            else {
                String operation[]  = new String[1];
                Object params[][]   = new Object[1][];
                boolean synchron[]  = new boolean[1];
                boolean mustReply[] = new boolean[1];
                boolean exception[] = new boolean[1];

                Object result = readMessage(operation, params, synchron, mustReply, exception);

                if(operation[0] == null) { // a reply ?
                    iMessage = new Message(null, // oid
                                           result, // object
                                           null, // interface
                                           null, // operation
                                           _in_threadId,
                                           false,
                                           false,
                                           exception[0],
                                           params[0]);
                }
                else { // a request
                    iMessage = new Message(_in_oid,
                                           null,
                                           _in_interface,
                                           operation[0],
                                           _in_threadId,
                                           synchron[0],
                                           mustReply[0],
                                           false,
                                           params[0]);
                }
            }
          }

        return iMessage;
    }


    public void flush(DataOutput  dataOutput) throws Exception {
        if(_message_count > 0) {
            writeBlock(dataOutput, _marshal.reset(), _message_count);

            _message_count = 0;
        }
    }

    public IMarshal createMarshal() {
        return new Marshal(_iBridge, __cache_size);
    }

    public IUnmarshal createUnmarshal(byte bytes[]) {
        Unmarshal unmarshal =  new Unmarshal(_iBridge, __cache_size);
        unmarshal.reset(bytes);

        return unmarshal;
    }
}

