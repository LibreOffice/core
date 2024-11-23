/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

package com.sun.star.lib.uno.protocols.urp;

import com.sun.star.bridge.InvalidProtocolChangeException;
import com.sun.star.bridge.ProtocolProperty;
import com.sun.star.bridge.XProtocolProperties;
import com.sun.star.lang.DisposedException;
import com.sun.star.lib.uno.environments.remote.IProtocol;
import com.sun.star.lib.uno.environments.remote.Message;
import com.sun.star.lib.uno.environments.remote.ThreadId;
import com.sun.star.lib.uno.typedesc.MethodDescription;
import com.sun.star.lib.uno.typedesc.TypeDescription;
import com.sun.star.uno.Any;
import com.sun.star.uno.IBridge;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XCurrentContext;
import java.io.DataInput;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.Random;
import java.util.StringTokenizer;

/**
 * This class internally relies on the availability of Java UNO type information
 * for the interface type <code>com.sun.star.bridge.XProtocolProperties</code>,
 * even though URP itself does not rely on that type.
 */
public final class urp implements IProtocol {
    public urp(
        IBridge bridge, String attributes, InputStream input,
        OutputStream output)
    {
        this.input = new DataInputStream(input);
        this.output = new DataOutputStream(output);
        marshal = new Marshal(bridge, CACHE_SIZE);
        unmarshal = new Unmarshal(bridge, CACHE_SIZE);
        forceSynchronous = parseAttributes(attributes);
    }

    /**
     *
     * @see IProtocol#init
     */
    public void init() throws IOException {
        synchronized (monitor) {
            if (state == STATE_INITIAL0) {
                sendRequestChange();
            }
        }
    }

    /**
     *
     * @see IProtocol#terminate
     */
    public void terminate() {
        synchronized (monitor) {
            state = STATE_TERMINATED;
            initialized = true;
            monitor.notifyAll();
        }
    }

    /**
     *
     * @see IProtocol#readMessage
     */
    public Message readMessage() throws IOException {
        for (;;) {
            if (!unmarshal.hasMore()) {
                unmarshal.reset(readBlock());
                if (!unmarshal.hasMore()) {
                    throw new IOException("closeConnection message received");
                }
            }
            UrpMessage msg;
            int header = unmarshal.read8Bit();
            if ((header & HEADER_LONGHEADER) != 0) {
                if ((header & HEADER_REQUEST) != 0) {
                    msg = readLongRequest(header);
                } else {
                    msg = readReply(header);
                }
            } else {
                msg = readShortRequest(header);
            }
            if (msg.isInternal()) {
                handleInternalMessage(msg);
            } else {
                return msg;
            }
        }
    }

    /**
     *
     * @see IProtocol#writeRequest
     */
    public boolean writeRequest(
        String oid, TypeDescription type, String function, ThreadId tid,
        Object[] arguments)
        throws IOException
    {
        if (oid.equals(PROPERTIES_OID)) {
            throw new IllegalArgumentException("illegal OID " + oid);
        }
        synchronized (monitor) {
            while (!initialized) {
                try {
                    monitor.wait();
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                    throw new RuntimeException(e);
                }
            }
            if (state == STATE_TERMINATED) {
                throw new DisposedException();
            }
            return writeRequest(false, oid, type, function, tid, arguments);
        }
    }

    /**
     *
     * @see IProtocol#writeReply
     */
    public void writeReply(boolean exception, ThreadId tid, Object result)
        throws IOException
    {
        synchronized (output) {
            writeQueuedReleases();
            int header = HEADER_LONGHEADER;
            PendingRequests.Item pending = pendingIn.pop(tid);
            TypeDescription resultType;
            TypeDescription[] argTypes;
            Object[] args;
            if (exception) {
                header |= HEADER_EXCEPTION;
                resultType = TypeDescription.getTypeDescription(TypeClass.ANY);
                argTypes = null;
                args = null;
            } else {
                resultType = pending.function.getReturnSignature();
                argTypes = pending.function.getOutSignature();
                args = pending.arguments;
            }
            if (!tid.equals(outL1Tid)) {
                header |= HEADER_NEWTID;
                outL1Tid = tid;
            } else {
                tid = null;
            }
            marshal.write8Bit(header);
            if (tid != null) {
                marshal.writeThreadId(tid);
            }
            marshal.writeValue(resultType, result);
            if (argTypes != null) {
                for (int i = 0; i < argTypes.length; ++i) {
                    if (argTypes[i] != null) {
                        marshal.writeValue(
                            argTypes[i].getComponentType(),
                            Array.get(args[i], 0));
                    }
                }
            }
            writeBlock(true);
        }
    }

    private void sendRequestChange() throws IOException {
        if (propertiesTid == null) {
            propertiesTid = ThreadId.createFresh();
        }
        random = randomGenerator.nextInt();
        writeRequest(
            true, PROPERTIES_OID,
            TypeDescription.getTypeDescription(XProtocolProperties.class),
            PROPERTIES_FUN_REQUEST_CHANGE, propertiesTid,
            new Object[] { Integer.valueOf(random) });
        state = STATE_REQUESTED;
    }

    private void handleInternalMessage(Message message) throws IOException {
        if (message.isRequest()) {
            String t = message.getType().getTypeName();
            if (!t.equals("com.sun.star.bridge.XProtocolProperties")) {
                throw new IOException(
                    "read URP protocol properties request with unsupported"
                    + " type " + t);
            }
            int fid = message.getMethod().getIndex();
            switch (fid) {
            case PROPERTIES_FID_REQUEST_CHANGE:
                checkSynchronousPropertyRequest(message);
                synchronized (monitor) {
                    switch (state) {
                    case STATE_INITIAL0:
                    case STATE_INITIAL:
                        writeReply(
                            false, message.getThreadId(), Integer.valueOf(1));
                        state = STATE_WAIT;
                        break;
                    case STATE_REQUESTED:
                        int n
                            = ((Integer) message.getArguments()[0]).intValue();
                        if (random < n) {
                            writeReply(
                                false, message.getThreadId(), Integer.valueOf(1));
                            state = STATE_WAIT;
                        } else if (random == n) {
                            writeReply(
                                false, message.getThreadId(), Integer.valueOf(-1));
                            state = STATE_INITIAL;
                            sendRequestChange();
                        } else {
                            writeReply(
                                false, message.getThreadId(), Integer.valueOf(0));
                        }
                        break;
                    default:
                        writeReply(
                            true, message.getThreadId(),
                            new com.sun.star.uno.RuntimeException(
                                "read URP protocol properties requestChange"
                                + " request in illegal state"));
                        break;
                    }
                }
                break;
            case PROPERTIES_FID_COMMIT_CHANGE:
                checkSynchronousPropertyRequest(message);
                synchronized (monitor) {
                    if (state == STATE_WAIT) {
                        ProtocolProperty[] p = (ProtocolProperty[])
                            message.getArguments()[0];
                        boolean ok = true;
                        boolean cc = false;
                        int i = 0;
                        for (; i < p.length; ++i) {
                            if (p[i].Name.equals(PROPERTY_CURRENT_CONTEXT)) {
                                cc = true;
                            } else {
                                ok = false;
                                break;
                            }
                        }
                        if (ok) {
                            writeReply(false, message.getThreadId(), null);
                        } else {
                            writeReply(
                                true, message.getThreadId(),
                                new InvalidProtocolChangeException(
                                    "", null, p[i], 1));
                        }
                        state = STATE_INITIAL;
                        if (!initialized) {
                            if (cc) {
                                currentContext = true;
                                initialized = true;
                                monitor.notifyAll();
                            } else {
                                sendRequestChange();
                            }
                        }
                    } else {
                        writeReply(
                            true, message.getThreadId(),
                            new com.sun.star.uno.RuntimeException(
                                "read URP protocol properties commitChange"
                                + " request in illegal state"));
                    }
                }
                break;
            default:
                throw new IOException(
                    "read URP protocol properties request with unsupported"
                    + " function ID " + fid);
            }
        } else {
            synchronized (monitor) {
                if (state == STATE_COMMITTED) {
                    // commitChange reply:
                    if (!message.isAbnormalTermination()) {
                        currentContext = true;
                    }
                    state = STATE_INITIAL;
                    initialized = true;
                    monitor.notifyAll();
                } else {
                    // requestChange reply:
                    if (message.isAbnormalTermination()) {
                        // remote side probably does not support negotiation:
                        state = STATE_INITIAL;
                        initialized = true;
                        monitor.notifyAll();
                    } else {
                        int n = ((Integer) message.getResult()).intValue();
                        switch (n) {
                        case -1:
                        case 0:
                            break;
                        case 1:
                            writeRequest(
                                true, PROPERTIES_OID,
                                TypeDescription.getTypeDescription(
                                    XProtocolProperties.class),
                                PROPERTIES_FUN_COMMIT_CHANGE, propertiesTid,
                                new Object[] {
                                    new ProtocolProperty[] {
                                        new ProtocolProperty(
                                            PROPERTY_CURRENT_CONTEXT,
                                            Any.VOID) } });
                            state = STATE_COMMITTED;
                            break;
                        default:
                            throw new IOException(
                                "read URP protocol properties "
                                + PROPERTIES_FUN_REQUEST_CHANGE
                                + " reply with illegal return value " + n);
                        }
                    }
                }
            }
        }
    }

    private void checkSynchronousPropertyRequest(Message message)
        throws IOException
    {
        if (!message.isSynchronous()) {
            throw new IOException(
                "read URP protocol properties request for synchronous function"
                + " marked as not SYNCHRONOUS");
        }
    }

    private byte[] readBlock() throws IOException {
        int size = input.readInt();
        input.readInt(); // ignore count
        byte[] bytes = new byte[size];
        input.readFully(bytes);
        return bytes;
    }

    private UrpMessage readLongRequest(int header) throws IOException {
        boolean sync = false;
        if ((header & HEADER_MOREFLAGS) != 0) {
            if (unmarshal.read8Bit() != (HEADER_MUSTREPLY | HEADER_SYNCHRONOUS))
            {
                throw new IOException(
                    "read URP request with bad MUSTREPLY/SYNCHRONOUS byte");
            }
            sync = true;
        }
        int funId = (header & HEADER_FUNCTIONID16) != 0
            ? unmarshal.read16Bit() : unmarshal.read8Bit();
        if ((header & HEADER_NEWTYPE) != 0) {
            inL1Type = unmarshal.readType();
            if (inL1Type.getTypeClass() != TypeClass.INTERFACE) {
                throw new IOException(
                    "read URP request with non-interface type " + inL1Type);
            }
        }
        if ((header & HEADER_NEWOID) != 0) {
            inL1Oid = unmarshal.readObjectId();
        }
        if ((header & HEADER_NEWTID) != 0) {
            inL1Tid = unmarshal.readThreadId();
        }
        return readRequest(funId, sync);
    }

    private UrpMessage readShortRequest(int header) throws IOException {
        int funId = (header & HEADER_FUNCTIONID14) != 0
            ? ((header & HEADER_FUNCTIONID) << 8) | unmarshal.read8Bit()
            : header & HEADER_FUNCTIONID;
        return readRequest(funId, false);
    }

    private UrpMessage readRequest(int functionId, boolean forcedSynchronous)
        throws IOException
    {
        boolean internal = PROPERTIES_OID.equals(inL1Oid);
            // inL1Oid may be null in XInstanceProvider.getInstance("")
        XCurrentContext cc;
        synchronized (monitor) {
            cc = (currentContext && !internal
                  && functionId != MethodDescription.ID_RELEASE)
                ? (XCurrentContext) unmarshal.readInterface(
                    new Type(XCurrentContext.class))
                : null;
        }
        MethodDescription desc = inL1Type.getMethodDescription(functionId);
        if (desc == null) {
            throw new IOException(
                "read URP request with unsupported function ID " + functionId);
        }
        TypeDescription[] inSig = desc.getInSignature();
        TypeDescription[] outSig = desc.getOutSignature();
        Object[] args = new Object[inSig.length];
        for (int i = 0; i < args.length; ++i) {
            if (inSig[i] != null) {
                if (outSig[i] != null) {
                    Object inout = Array.newInstance(
                        outSig[i].getComponentType().getZClass(), 1);
                    Array.set(
                        inout, 0,
                        unmarshal.readValue(
                            outSig[i].getComponentType()));
                    args[i] = inout;
                } else {
                    args[i] = unmarshal.readValue(inSig[i]);
                }
            } else {
                args[i] = Array.newInstance(
                    outSig[i].getComponentType().getZClass(), 1);
            }
        }
        boolean sync = forcedSynchronous || !desc.isOneway();
        if (sync) {
            pendingIn.push(
                inL1Tid, new PendingRequests.Item(internal, desc, args));
        }
        return new UrpMessage(
            inL1Tid, true, inL1Oid, inL1Type, desc, sync, cc, false, null, args,
            internal);
    }

    private UrpMessage readReply(int header) {
        if ((header & HEADER_NEWTID) != 0) {
            inL1Tid = unmarshal.readThreadId();
        }
        PendingRequests.Item pending = pendingOut.pop(inL1Tid);
        TypeDescription resultType;
        TypeDescription[] argTypes;
        Object[] args;
        boolean exception = (header & HEADER_EXCEPTION) != 0;
        if (exception) {
            resultType = TypeDescription.getTypeDescription(TypeClass.ANY);
            argTypes = null;
            args = null;
        } else {
            resultType = pending.function.getReturnSignature();
            argTypes = pending.function.getOutSignature();
            args = pending.arguments;
        }
        Object result = resultType == null
            ? null : unmarshal.readValue(resultType);
        if (argTypes != null) {
            for (int i = 0; i < argTypes.length; ++i) {
                if (argTypes[i] != null) {
                    Array.set(
                        args[i], 0,
                        unmarshal.readValue(
                            argTypes[i].getComponentType()));
                }
            }
        }
        return new UrpMessage(
            inL1Tid, false, null, null, null, false, null, exception, result,
            args, pending.internal);
    }

    private boolean writeRequest(
        boolean internal, String oid, TypeDescription type, String function,
        ThreadId tid, Object[] arguments)
        throws IOException
    {
        MethodDescription desc = type.getMethodDescription(function);
        synchronized (output) {
            if (desc.getIndex() == MethodDescription.ID_RELEASE
                && releaseQueue.size() < MAX_RELEASE_QUEUE_SIZE)
            {
                releaseQueue.add(
                    new QueuedRelease(internal, oid, type, desc, tid));
                return false;
            } else {
                writeQueuedReleases();
                return writeRequest(
                    internal, oid, type, desc, tid, arguments, true);
            }
        }
    }

    private boolean writeRequest(
        boolean internal, String oid, TypeDescription type,
        MethodDescription desc, ThreadId tid, Object[] arguments,
        boolean flush)
        throws IOException
    {
        int funId = desc.getIndex();
        if (funId < 0 || funId > MAX_FUNCTIONID16) {
            throw new IllegalArgumentException(
                "function ID " + funId + " out of range");
        }
        boolean forceSync = forceSynchronous
            && funId != MethodDescription.ID_RELEASE;
        boolean moreFlags = forceSync && desc.isOneway();
        boolean longHeader = moreFlags;
        int header = 0;
        if (!type.equals(outL1Type)) {
            longHeader = true;
            header |= HEADER_NEWTYPE;
            outL1Type = type;
        } else {
            type = null;
        }
        if (!oid.equals(outL1Oid)) {
            longHeader = true;
            header |= HEADER_NEWOID;
            outL1Oid = oid;
        } else {
            oid = null;
        }
        if (!tid.equals(outL1Tid)) {
            longHeader = true;
            header |= HEADER_NEWTID;
            outL1Tid = tid;
        } else {
            tid = null;
        }
        if (funId > MAX_FUNCTIONID14) {
            longHeader = true;
        }
        if (longHeader) {
            header |= HEADER_LONGHEADER | HEADER_REQUEST;
            if (funId > MAX_FUNCTIONID8) {
                header |= HEADER_FUNCTIONID16;
            }
            if (moreFlags) {
                header |= HEADER_MOREFLAGS;
            }
            marshal.write8Bit(header);
            if (moreFlags) {
                marshal.write8Bit(HEADER_MUSTREPLY | HEADER_SYNCHRONOUS);
            }
            if (funId > MAX_FUNCTIONID8) {
                marshal.write16Bit(funId);
            } else {
                marshal.write8Bit(funId);
            }
            if (type != null) {
                marshal.writeType(type);
            }
            if (oid != null) {
                marshal.writeObjectId(oid);
            }
            if (tid != null) {
                marshal.writeThreadId(tid);
            }
        } else {
            if (funId > HEADER_FUNCTIONID) {
                marshal.write8Bit(HEADER_FUNCTIONID14 | (funId >> 8));
            }
            marshal.write8Bit(funId);
        }
        if (currentContext && !internal
            && funId != MethodDescription.ID_RELEASE)
        {
            marshal.writeInterface(
                UnoRuntime.getCurrentContext(),
                new Type(XCurrentContext.class));
        }
        TypeDescription[] inSig = desc.getInSignature();
        TypeDescription[] outSig = desc.getOutSignature();
        for (int i = 0; i < inSig.length; ++i) {
            if (inSig[i] != null) {
                if (outSig[i] != null) {
                    marshal.writeValue(
                        outSig[i].getComponentType(),
                        ((Object[]) arguments[i])[0]);
                } else {
                    marshal.writeValue(
                        inSig[i], arguments[i]);
                }
            }
        }
        boolean sync = forceSync || !desc.isOneway();
        if (sync) {
            pendingOut.push(
                outL1Tid, new PendingRequests.Item(internal, desc, arguments));
        }
        writeBlock(flush);
        return sync;
    }

    private void writeBlock(boolean flush) throws IOException {
        byte[] data = marshal.reset();
        output.writeInt(data.length);
        output.writeInt(1);
        output.write(data);
        if (flush) {
            output.flush();
        }
    }

    private void writeQueuedReleases() throws IOException {
        for (int i = releaseQueue.size(); i > 0;) {
            --i;
            QueuedRelease r = releaseQueue.get(i);
            writeRequest(
                r.internal, r.objectId, r.type, r.method, r.threadId, null,
                false);
            releaseQueue.remove(i);
        }
    }

    private static boolean parseAttributes(String attributes) {
        boolean forceSynchronous = true;
        if (attributes != null) {
            StringTokenizer t = new StringTokenizer(attributes, ",");
            while (t.hasMoreTokens()) {
                String a = t.nextToken();
                String v = null;
                int i = a.indexOf('=');
                if (i >= 0) {
                    v = a.substring(i + 1);
                    a = a.substring(0, i);
                }
                if (a.equalsIgnoreCase("ForceSynchronous")) {
                    forceSynchronous = parseBooleanAttributeValue(a, v);
                } else if (a.equalsIgnoreCase("negotiate")) {
                    // Ignored:
                    parseBooleanAttributeValue(a, v);
                } else {
                    throw new IllegalArgumentException(
                        "unknown protocol attribute " + a);
                }
            }
        }
        return forceSynchronous;
    }

    private static boolean parseBooleanAttributeValue(
        String attribute, String value)
    {
        if (value == null) {
            throw new IllegalArgumentException(
                "missing value for protocol attribute " + attribute);
        }
        if (value.equals("0")) {
            return false;
        } else if (value.equals("1")) {
            return true;
        } else {
            throw new IllegalArgumentException(
                "bad value " + value + " for protocol attribute " + attribute);
        }
    }

    private static final class QueuedRelease {
        public QueuedRelease(
            boolean internal, String objectId, TypeDescription type,
            MethodDescription method, ThreadId threadId)
        {
            this.internal = internal;
            this.objectId = objectId;
            this.type = type;
            this.method = method;
            this.threadId = threadId;
        }

        public final boolean internal;
        public final String objectId;
        public final TypeDescription type;
        public final MethodDescription method;
        public final ThreadId threadId;
    }

    private static final String PROPERTIES_OID = "UrpProtocolProperties";
    private static final int PROPERTIES_FID_REQUEST_CHANGE = 4;
    private static final String PROPERTIES_FUN_REQUEST_CHANGE = "requestChange";
    private static final int PROPERTIES_FID_COMMIT_CHANGE = 5;
    private static final String PROPERTIES_FUN_COMMIT_CHANGE = "commitChange";
    private static final String PROPERTY_CURRENT_CONTEXT = "CurrentContext";

    private static final short CACHE_SIZE = 256;

    private static final int HEADER_LONGHEADER = 0x80;
    private static final int HEADER_REQUEST = 0x40;
    private static final int HEADER_NEWTYPE = 0x20;
    private static final int HEADER_NEWOID = 0x10;
    private static final int HEADER_NEWTID = 0x08;
    private static final int HEADER_FUNCTIONID16 = 0x04;
    private static final int HEADER_MOREFLAGS = 0x01;
    private static final int HEADER_MUSTREPLY = 0x80;
    private static final int HEADER_SYNCHRONOUS = 0x40;
    private static final int HEADER_FUNCTIONID14 = 0x40;
    private static final int HEADER_FUNCTIONID = 0x3F;
    private static final int HEADER_EXCEPTION = 0x20;

    private static final int MAX_FUNCTIONID16 = 0xFFFF;
    private static final int MAX_FUNCTIONID14 = 0x3FFF;
    private static final int MAX_FUNCTIONID8 = 0xFF;

    private static final int STATE_INITIAL0 = 0;
    private static final int STATE_INITIAL = 1;
    private static final int STATE_REQUESTED = 2;
    private static final int STATE_COMMITTED = 3;
    private static final int STATE_WAIT = 4;
    private static final int STATE_TERMINATED = 5;

    private static final int MAX_RELEASE_QUEUE_SIZE = 100;

    private static final Random randomGenerator = new Random();

    private final DataInput input;
    private final DataOutputStream output;

    private final Marshal marshal;
    private final Unmarshal unmarshal;

    private final boolean forceSynchronous;

    private final PendingRequests pendingIn = new PendingRequests();
    private final PendingRequests pendingOut = new PendingRequests();

    private final Object monitor = new Object();
    private int state = STATE_INITIAL0;
    private boolean initialized = false;
    private ThreadId propertiesTid = null;
    private int random;
    private boolean currentContext = false;

    private ThreadId inL1Tid = null;
    private String inL1Oid = null;
    private TypeDescription inL1Type = null;

    private ThreadId outL1Tid = null;
    private String outL1Oid = null;
    private TypeDescription outL1Type = null;

    private final ArrayList<QueuedRelease> releaseQueue = new ArrayList<QueuedRelease>(); // of QueuedRelease
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
