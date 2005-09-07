/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: urp.java,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:04:14 $
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

import com.sun.star.lib.uno.environments.remote.IMessage;
import com.sun.star.lib.uno.environments.remote.Protocol;
import com.sun.star.lib.uno.environments.remote.ThreadId;

import com.sun.star.uno.IMethodDescription;
import com.sun.star.uno.ITypeDescription;


import com.sun.star.lib.uno.typedesc.TypeDescription;

import com.sun.star.uno.Any;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Type;


/**
 * This class implements the complete urp protocol
 * from uno.
 * <p>
 * @version     $Revision: 1.16 $ $ $Date: 2005-09-07 19:04:14 $
 * @author      Kay Ramme
 * @since       UDK1.0
 */
public class urp extends Protocol {
    /**
     * When set to true, enables various debugging output.
     */
    static public final boolean DEBUG = false;

    static private final ITypeDescription __emptyITypeDescArray[] = new ITypeDescription[0];
    static private final short           __cache_size = 256;


    protected IBridge           _iBridge;

    private String   _in_oid;
    private ITypeDescription     _in_interface;
    private ThreadId _in_threadId;

    private String   _out_oid;
    private ITypeDescription     _out_interface;
    private ThreadId _out_threadId;

    private int       _message_count;
    private boolean   _ignore_cache;
    private Marshal   _marshal;
    private Unmarshal _unmarshal;

    private String _operationContainer[]  = new String[1];
    private Object _paramsContainer[][]   = new Object[1][];
    private boolean _synchronContainer[]  = new boolean[1];
    private boolean _mustReplyContainer[] = new boolean[1];
    private boolean _exceptionContainer[] = new boolean[1];

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
     * Gets the name of the protocol.
     * <p>
     * @result  the name of the protocol (iiop)
     */
    public String getName() {
        return "urp";
    }

    private Object readReply(int header, boolean exception[]) {
        if((header & NEWTID) != 0) // new thread id ?
            _in_threadId = _unmarshal.readThreadId();

        // get the out signature and parameter array of the reply
        Object objects[] = (Object[])removePendingRequest(_in_threadId);
        Object param[] = (Object[])objects[0];
        ITypeDescription signature[] = (ITypeDescription[])objects[1];
        TypeDescription resultType = (TypeDescription)objects[2];

        exception[0] = (header & EXCEPTION) != 0;
        if(exception[0]) {// Exception? So the reply has an any as the result
            signature = __emptyITypeDescArray;
            try {
                resultType = TypeDescription.getTypeDescription("any");
            } catch (ClassNotFoundException e) {
                throw new RuntimeException("this cannot happen: " + e);
            }
        }

        // read the result object
        Object result = null;
        if(resultType != null)
            result = _unmarshal.readValue(resultType);

        // read the out parameters
        for(int i = 0; i < signature.length; ++ i) {
            if(signature[i] != null) // is this an out parameter
                Array.set(
                    param[i], 0,
                    _unmarshal.readValue(
                        (TypeDescription) signature[i].getComponentType()));
        }

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readReply:" + result);

        return result;
    }


    private Object []readParams(IMethodDescription iMethodDescription) {
        ITypeDescription in_sig[] = iMethodDescription.getInSignature();
        ITypeDescription out_sig[] = iMethodDescription.getOutSignature();

        Object params[] = new Object[in_sig.length];

        for(int i = 0; i < params.length; ++ i) {
            if(in_sig[i] != null) // is it an in parameter?
                if(out_sig[i] != null) {// is it also an out -> inout?
                    Object inout = Array.newInstance(out_sig[i].getComponentType().getZClass(), 1);
                    Array.set(
                        inout, 0,
                        _unmarshal.readValue(
                            (TypeDescription) out_sig[i].getComponentType()));
                    params[i] = inout;
                }
                else  // it is only an in parameter
                    params[i] = _unmarshal.readValue(
                        (TypeDescription) in_sig[i]);
            else // it is only an out parameter, so provide the holder
                params[i] = Array.newInstance(out_sig[i].getComponentType().getZClass(), 1);
        }

        return params;
    }


    private void readShortRequest(int header, String operation[], Object param[][], boolean synchron[]) {
        ++ _requestsRecieved;

        int methodId;
        if ((header & DIR_MID) != 0) {
            methodId = ((header & 0x3F) << 8) | _unmarshal.read8Bit();
        } else {
            methodId = header & 0x3F;
        }

        IMethodDescription iMethodDescription = _in_interface.getMethodDescription(methodId);
        operation[0] = iMethodDescription.getName();

        synchron[0] = !iMethodDescription.isOneway();

        param[0] = readParams(iMethodDescription);

        if(synchron[0]) { // if the request is synchron, it is pending
            putPendingReply(_in_threadId, new Object[]{param[0], iMethodDescription.getOutSignature(), iMethodDescription.getReturnSignature()/*, mMDesc*/});
        }

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readShortRequest:" + _in_oid + " " + operation[0] + " " + synchron[0]);
    }

    private void readLongRequest(int header, String operation[], Object param[][], boolean synchron[], boolean mustReply[]) {
        ++ _requestsRecieved;

        // read the extended flags
        if((header & MOREFLAGS) != 0) {// is there an extended flags byte?
            int exFlags = _unmarshal.read8Bit();

            mustReply[0] = (exFlags & MUSTREPLY) != 0;
            synchron[0]  = (exFlags & SYNCHRONOUSE) != 0;
        }

        int methodId;
        if ((header & LONGMETHODID) != 0) {
            methodId = _unmarshal.read16Bit();
        } else {
            methodId = _unmarshal.read8Bit();
        }

        if((header & NEWTYPE) != 0)
            _in_interface = _unmarshal.readType();

        IMethodDescription iMethodDescription = _in_interface.getMethodDescription(methodId);

        if((header & MOREFLAGS) == 0) // no ex flags, so get info from typeinfo
            synchron[0] = !iMethodDescription.isOneway();

        if((header & NEWOID) != 0) // new oid?
            _in_oid = _unmarshal.readObjectId();

        if((header & NEWTID) != 0) // new thread id ?
            _in_threadId = _unmarshal.readThreadId();

        _ignore_cache = ((header & IGNORECACHE) != 0); // do not use cache for this request?

        operation[0] = iMethodDescription.getName();

        param[0] = readParams(iMethodDescription);

        if(synchron[0]) // if the request is synchron, the reply is pending
            putPendingReply(_in_threadId, new Object[]{param[0], iMethodDescription.getOutSignature(), iMethodDescription.getReturnSignature()/*, mMDesc*/});

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readLongRequest:" + _in_oid + " " + operation[0] + " " + synchron[0]);
    }

    private Object readMessage(String operation[], Object param[], boolean synchron[], boolean mustReply[], boolean exception[]) {
        int header = _unmarshal.read8Bit();

        Class signature[];
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
                             ThreadId threadId,
                             Object params[],
                             Boolean synchron[],
                             Boolean mustReply[])
   {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeRequest:" + oid + " " + zInterface + " " + operation);

        ++ _requestsSend;
        ++ _message_count;

        IMethodDescription iMethodDescription = zInterface.getMethodDescription(operation);

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
            if(iMethodDescription.isOneway() == synchron[0].booleanValue()) {
                bigHeader = true;
                hasExFlags = true;
            }
        }
        else
            synchron[0] = new Boolean(!iMethodDescription.isOneway());

        // if mustReply is provided and if it differs from synchron
        // then we have to write it
        if(mustReply[0] != null && (mustReply[0] != synchron[0])) {
            bigHeader = true;
            hasExFlags = true;
        }
        else
            mustReply[0] = synchron[0];

        // Long request headers can handle 16-bit method IDs, and short request
        // headers can handle 14-bit method IDs:
        int methodId = iMethodDescription.getIndex();
        if (methodId < 0 || methodId >= 0x10000) {
            throw new IllegalArgumentException(
                "Method ID " + methodId + " out of range");
        } else if (methodId >= 0xC000) {
            bigHeader = true;
        }

        if(bigHeader) { // something has changed, send big header
            header |= BIG_HEADER; // big header
            header |= REQUEST;
            header |= hasExFlags ? MOREFLAGS : 0;

            if(methodId > 255)
                header |= LONGMETHODID;

            _marshal.write8Bit(header);

            if(hasExFlags) {// are there extended flags to write?
                byte exFlags = 0;

                exFlags |= synchron[0].booleanValue() ? SYNCHRONOUSE : 0;
                exFlags |= mustReply[0].booleanValue() ? MUSTREPLY : 0;

                _marshal.write8Bit(exFlags);
            }

            // write the method id
            if(methodId > 255)
                _marshal.write16Bit(methodId);
            else
                _marshal.write8Bit(methodId);

            if(zInterface != null) // has the interface changed? -> write it
                _marshal.writeType(zInterface);

            if(oid != null) // has the oid changed? -> write it
                _marshal.writeObjectId(_out_oid);

            if(threadId != null) // has the thread id changed? -> write it
                _marshal.writeThreadId(threadId);
        }
        else { // simple request
            if(methodId <= 0x2f) // does the method id fit in the header?
                _marshal.write8Bit(methodId);
            else { // no
                header |= DIR_MID;
                header |= methodId >> 8;

                _marshal.write8Bit(header);
                _marshal.write8Bit(methodId);
            }
        }

        // write the in parameters
        ITypeDescription in_sig[] = iMethodDescription.getInSignature();
        ITypeDescription out_sig[] = iMethodDescription.getOutSignature();
        for(int i = 0; i < in_sig.length; ++ i) {
            if(in_sig[i] != null) { // is it an in parameter?
                if(out_sig[i] != null)  // is it also an out parameter?
                    _marshal.writeValue(
                        (TypeDescription) out_sig[i].getComponentType(),
                        ((Object[]) params[i])[0]);
                else // in only
                    _marshal.writeValue((TypeDescription) in_sig[i], params[i]);
            }
        }

        if(synchron[0].booleanValue()) // if we are waiting for a reply, the reply is pending
            putPendingRequest(_out_threadId, new Object[]{params, out_sig, iMethodDescription.getReturnSignature()});
    }

    public void writeReply(boolean exception, ThreadId threadId, Object result) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeReply:" + exception + " " + threadId + " " + result);

        ++ _message_count;

        Object objects[] = (Object[])removePendingReply(threadId);
        Object params[] = (Object[])objects[0];
        ITypeDescription signature[] = (ITypeDescription[])objects[1];
        TypeDescription resType     = (TypeDescription)objects[2];

        byte header = BIG_HEADER; // big header

        if(exception) { // has an exception occurred?
            header |= EXCEPTION;

            signature = __emptyITypeDescArray;
            try {
                resType = TypeDescription.getTypeDescription("any");
            } catch (ClassNotFoundException e) {
                throw new RuntimeException("this cannot happen: " + e);
            }
        }

        if(_out_threadId == null || !_out_threadId.equals(threadId)) { // change thread id ?
            header |= NEWTID;

            _out_threadId = threadId;
        }
        else
            threadId = null;

        _marshal.write8Bit(header);

        if(threadId != null) // has the thread id changed? -> write it
            _marshal.writeThreadId(threadId);

        // write the result
        _marshal.writeValue(resType, result);

        // write the out parameters
        for(int i = 0; i < signature.length; ++ i)
            if(signature[i] != null)
                _marshal.writeValue(
                    (TypeDescription) signature[i].getComponentType(),
                    Array.get(params[i], 0));
    }


    private byte []readBlock(DataInput dataInput) throws IOException {
        int size = dataInput.readInt();
        int message_count = dataInput.readInt();

        byte bytes[] = new byte[size];

        dataInput.readFully(bytes);

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readBlock: size:" + size + " message count:" + message_count);

        return bytes;
    }

    private void writeBlock(DataOutput out, byte[] data, int messageCount)
        throws IOException
    {
        out.writeInt(data.length);
        out.writeInt(messageCount);
        out.write(data);
    }

    static class Message implements IMessage {
        String    _oid;
        Object    _result;
        ITypeDescription      _iTypeDescription;
        String    _operation;
        ThreadId  _threadId;
        boolean   _synchron;
        boolean   _mustReply;
        boolean   _exception;
        Object    _params[];

        Message(String    oid,
                Object    result,
                ITypeDescription      iTypeDescription,
                String    operation,
                ThreadId  threadId,
                boolean   synchron,
                boolean   mustReply,
                boolean   exception,
                Object    params[])
        {
            _oid       = oid;
            _result    = result;
            _iTypeDescription      = iTypeDescription;
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

        public ThreadId getThreadId() {
            return _threadId;
        }

        public ITypeDescription getInterface() {
            return _iTypeDescription;
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

        public Object getData(Object params[][]) {
            params[0]    = _params;

            return _result;
        }
    }

    /**
     * reads a job from the given stream.
     * <p>
     * @return  thread read job.
     * @see     com.sun.star.lib.uno.environments.remote.Job
     */
    public IMessage readMessage(InputStream inputStream) throws IOException {
        IMessage iMessage = null;

        DataInput dataInput = new DataInputStream( inputStream );
          while(iMessage == null) { // try hard to get a message
            if(!_unmarshal.hasMore()) { // the last block is empty, get a new one
                byte bytes[] = readBlock(dataInput);
                _unmarshal.reset(bytes);
            }

            if(!_unmarshal.hasMore()) // we already got a new block and there are still no bytes left? -> a close message
                throw new java.io.IOException("connection close message received");

            else {
                Object result = readMessage(_operationContainer, _paramsContainer, _synchronContainer,
                                            _mustReplyContainer, _exceptionContainer);

                if(_operationContainer[0] == null) { // a reply ?
                    iMessage = new Message(null, // oid
                                           result, // object
                                           null, // interface
                                           null, // operation
                                           _in_threadId,
                                           false,
                                           false,
                                           _exceptionContainer[0],
                                           _paramsContainer[0]);
                }
                else { // a request
                    iMessage = new Message(_in_oid,
                                           null,
                                           _in_interface,
                                           _operationContainer[0],
                                           _in_threadId,
                                           _synchronContainer[0],
                                           _mustReplyContainer[0],
                                           false,
                                           _paramsContainer[0]);
                }
                _operationContainer[0] = null;
                _paramsContainer[0] = null;
                _synchronContainer[0] = false;
                _exceptionContainer[0] = false;
                _mustReplyContainer[0] = false;
            }
          }

        return iMessage;
    }


    public void flush(DataOutput dataOutput) throws IOException {
        if(_message_count > 0) {
            writeBlock(dataOutput, _marshal.reset(), _message_count);

            _message_count = 0;
        }
    }
}

