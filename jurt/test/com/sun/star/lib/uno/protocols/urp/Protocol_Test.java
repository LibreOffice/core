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

import com.sun.star.lib.uno.environments.remote.Message;
import com.sun.star.lib.uno.environments.remote.IProtocol;
import com.sun.star.lib.uno.environments.remote.ThreadId;
import com.sun.star.lib.uno.typedesc.TypeDescription;
import com.sun.star.uno.Any;
import com.sun.star.uno.IBridge;
import com.sun.star.uno.Type;
import com.sun.star.uno.XInterface;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;
import java.util.LinkedList;
import org.junit.Test;
import static org.junit.Assert.*;

public final class Protocol_Test {
    @Test public void test() throws Exception {
        IBridge iBridge = new TestBridge();
        PipedInputStream inA = new PipedInputStream();
        PipedOutputStream outA = new PipedOutputStream(inA);
        PipedInputStream inB = new PipedInputStream();
        PipedOutputStream outB = new PipedOutputStream(inB);
        Endpoint iSender = new Endpoint(iBridge, inA, outB);
        Endpoint iReceiver = new Endpoint(iBridge, inB, outA);

        TestObject testObject = new TestObject();
        String oId = (String)iBridge.mapInterfaceTo(testObject, new Type(XInterface.class));

        testCall(iSender, iReceiver, oId);
        testCallWithInParameter(iSender, iReceiver, oId);
        testCallWithOutParameter(iSender, iReceiver, oId);
        testCallWithInOutParameter(iSender, iReceiver, oId);
        testCallWithResult(iSender, iReceiver, oId);
        testCallWhichRaisesException(iSender, iReceiver, oId);
        testCallWithIn_Out_InOut_Paramters_and_result(iSender, iReceiver, oId);
        testCallWhichReturnsAny(iSender, iReceiver, oId);
    }

    public void testCall(
        Endpoint iSender, Endpoint iReceiver, String oId) throws Exception
    {
        // send an ordinary request
        iSender.writeRequest(
            oId, TypeDescription.getTypeDescription(TestXInterface.class),
            "method", new ThreadId(new byte[] { 0, 1 }), new Object[0]);
        iReceiver.readMessage();

        // send a reply
        iReceiver.writeReply(false, new ThreadId(new byte[] { 0, 1 }), null);
        iSender.readMessage();
    }

    public void testCallWithInParameter(
        Endpoint iSender, Endpoint iReceiver, String oId) throws Exception
    {
        // send an ordinary request
        iSender.writeRequest(
            oId, TypeDescription.getTypeDescription(TestXInterface.class),
            "methodWithInParameter", new ThreadId(new byte[] { 0, 1 }),
            new Object[] { "hallo" });
        Message iMessage = iReceiver.readMessage();
        Object[] t_params = iMessage.getArguments();
        assertEquals("hallo", t_params[0]);

        // send a reply
        iReceiver.writeReply(false, new ThreadId(new byte[] { 0, 1 }), null);
        iMessage = iSender.readMessage();
    }

    public void testCallWithOutParameter(
        Endpoint iSender, Endpoint iReceiver, String oId) throws Exception
    {
        Object params[] = new Object[]{new String[1]};
        iSender.writeRequest(
            oId, TypeDescription.getTypeDescription(TestXInterface.class),
            "methodWithOutParameter", new ThreadId(new byte[] { 0, 1 }),
            params);
        Message iMessage = iReceiver.readMessage();


        Object[] t_params = iMessage.getArguments();
        ((String [])t_params[0])[0] = "testString";

        // send an exception as reply
        iReceiver.writeReply(false, new ThreadId(new byte[] { 0, 1 }), null);
        iSender.readMessage();

        assertEquals("testString", ((String [])params[0])[0]);
    }

    public void testCallWithInOutParameter(
        Endpoint iSender, Endpoint iReceiver, String oId) throws Exception
    {
        Object params[] = new Object[]{new String[]{"inString"}};
        iSender.writeRequest(
            oId, TypeDescription.getTypeDescription(TestXInterface.class),
            "methodWithInOutParameter", new ThreadId(new byte[] { 0, 1 }),
            params);
        Message iMessage = iReceiver.readMessage();


        Object[] t_params = iMessage.getArguments();
        assertEquals("inString", ((String [])t_params[0])[0]);

        // provide reply
        ((String [])t_params[0])[0] = "outString";

        // send an exception as reply
        iReceiver.writeReply(false, new ThreadId(new byte[] { 0, 1 }), null);
        iSender.readMessage();

        assertEquals("outString", ((String [])params[0])[0]);
    }

    public void testCallWithResult(
        Endpoint iSender, Endpoint iReceiver, String oId) throws Exception
    {
        // send an ordinary request
        iSender.writeRequest(
            oId, TypeDescription.getTypeDescription(TestXInterface.class),
            "methodWithResult", new ThreadId(new byte[] { 0, 1 }),
            new Object[0]);
        iReceiver.readMessage();

        // send a reply
        iReceiver.writeReply(
            false, new ThreadId(new byte[] { 0, 1 }), "resultString");
        Message iMessage = iSender.readMessage();
        Object result = iMessage.getResult();

        assertEquals("resultString", result);
    }

    public void testCallWhichRaisesException(
        Endpoint iSender, Endpoint iReceiver, String oId) throws Exception
    {
        // send a second request
        iSender.writeRequest(
            oId, TypeDescription.getTypeDescription(TestXInterface.class),
            "method", new ThreadId(new byte[] { 0, 1 }), new Object[0]);
        iReceiver.readMessage();

        // send an exception as reply
        iReceiver.writeReply(
            true, new ThreadId(new byte[] { 0, 1 }),
            new com.sun.star.uno.RuntimeException("test the exception"));
        Message iMessage = iSender.readMessage();

        Object result = iMessage.getResult();

        assertTrue(result instanceof com.sun.star.uno.RuntimeException);
    }

    public void testCallWithIn_Out_InOut_Paramters_and_result(
        Endpoint iSender, Endpoint iReceiver, String oId) throws Exception
    {
        Object params[] = new Object[]{"hallo", new String[1], new String[]{"inOutString"}};
        iSender.writeRequest(
            oId, TypeDescription.getTypeDescription(TestXInterface.class),
            "MethodWithIn_Out_InOut_Paramters_and_result",
            new ThreadId(new byte[] { 0, 1 }), params);
        Message iMessage = iReceiver.readMessage();

        Object[] t_params = iMessage.getArguments();

        assertEquals("hallo", t_params[0]);

        assertEquals("inOutString", ((String [])t_params[2])[0]);

        ((String [])t_params[1])[0] = "outString";
        ((String [])t_params[2])[0] = "inOutString_res";

        // send an exception as reply
        iReceiver.writeReply(
            false, new ThreadId(new byte[] { 0, 1 }), "resultString");
        iMessage = iSender.readMessage();

        Object result = iMessage.getResult();
        assertEquals("outString", ((String [])params[1])[0]);

        assertEquals("inOutString_res", ((String [])params[2])[0]);

        assertEquals("resultString", result);
    }

    public void testCallWhichReturnsAny(
        Endpoint iSender, Endpoint iReceiver, String oId) throws Exception
    {
        // send an ordinary request
        iSender.writeRequest(
            oId, TypeDescription.getTypeDescription(TestXInterface.class),
            "returnAny", new ThreadId(new byte[] { 0, 1 }), null);
        iReceiver.readMessage();
        // send a reply
        iReceiver.writeReply(
            false, new ThreadId(new byte[] { 0, 1 }), Any.VOID);
        Message iMessage = iSender.readMessage();
        Object result = iMessage.getResult();
        assertTrue(
            result instanceof Any &&
            ((TypeDescription.getTypeDescription(((Any) result).getType()).
              getZClass()) ==
             void.class));

        // send an ordinary request
        iSender.writeRequest(
            oId, TypeDescription.getTypeDescription(TestXInterface.class),
            "returnAny", new ThreadId(new byte[] { 0, 1 }), null);
        iReceiver.readMessage();
        // send a reply
        iReceiver.writeReply(
            false, new ThreadId(new byte[] { 0, 1 }),
            new Any(XInterface.class, null));
        iMessage = iSender.readMessage();
        result = iMessage.getResult();
        assertNull(result);

        // send an ordinary request
        iSender.writeRequest(
            oId, TypeDescription.getTypeDescription(TestXInterface.class),
            "returnAny", new ThreadId(new byte[] { 0, 1 }), null);
        iReceiver.readMessage();
        // send a reply
        iReceiver.writeReply(
            false, new ThreadId(new byte[] { 0, 1 }), new Integer(501));
        iMessage = iSender.readMessage();
        result = iMessage.getResult();
        assertEquals(501, result);
    }

    private static final class Endpoint {
        public Endpoint(IBridge bridge, InputStream input, OutputStream output)
            throws IOException
        {
            protocol = new urp(bridge, null, input, output);
            new Thread() {
                public void run() {
                    for (;;) {
                        Object o;
                        try {
                            o = protocol.readMessage();
                        } catch (IOException e) {
                            o = e;
                        }
                        synchronized (queue) {
                            queue.addLast(o);
                        }
                    }
                }
            }.start();
            protocol.init();
        }

        public Message readMessage() throws IOException {
            for (;;) {
                synchronized (queue) {
                    if (!queue.isEmpty()) {
                        Object o = queue.removeFirst();
                        if (o instanceof Message) {
                            return (Message) o;
                        } else {
                            throw (IOException) o;
                        }
                    }
                }
            }
        }

        public boolean writeRequest(
            String oid, TypeDescription type, String function, ThreadId tid,
            Object[] arguments)
            throws IOException
        {
            return protocol.writeRequest(oid, type, function, tid, arguments);
        }

        public void writeReply(boolean exception, ThreadId tid, Object result)
            throws IOException
        {
            protocol.writeReply(exception, tid, result);
        }

        private final IProtocol protocol;
        private final LinkedList<Object> queue = new LinkedList<Object>();
    }
}
