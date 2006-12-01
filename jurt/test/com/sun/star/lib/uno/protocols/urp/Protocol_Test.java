/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Protocol_Test.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 14:56:28 $
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

import com.sun.star.lib.uno.environments.remote.Message;
import com.sun.star.lib.uno.environments.remote.IProtocol;
import com.sun.star.lib.uno.environments.remote.ThreadId;
import com.sun.star.lib.uno.typedesc.TypeDescription;
import com.sun.star.uno.Any;
import com.sun.star.uno.IBridge;
import com.sun.star.uno.Type;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PipedInputStream;
import java.io.PipedOutputStream;
import java.util.LinkedList;

public final class Protocol_Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws Exception {
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
        assure("", "hallo".equals((String)t_params[0]));

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

        assure("", "testString".equals(((String [])params[0])[0]));
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
        assure("", "inString".equals(((String [])t_params[0])[0]));

        // provide reply
        ((String [])t_params[0])[0] = "outString";

        // send an exception as reply
        iReceiver.writeReply(false, new ThreadId(new byte[] { 0, 1 }), null);
        iSender.readMessage();

        assure("", "outString".equals(((String [])params[0])[0]));
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

        assure("", "resultString".equals(result));
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

        assure("", result instanceof com.sun.star.uno.RuntimeException);
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

        assure("", "hallo".equals((String)t_params[0]));

        assure("", "inOutString".equals(((String [])t_params[2])[0]));

        ((String [])t_params[1])[0] = "outString";
        ((String [])t_params[2])[0] = "inOutString_res";

        // send an exception as reply
        iReceiver.writeReply(
            false, new ThreadId(new byte[] { 0, 1 }), "resultString");
        iMessage = iSender.readMessage();

        Object result = iMessage.getResult();
        assure("", "outString".equals(((String [])params[1])[0]));

        assure("", "inOutString_res".equals(((String [])params[2])[0]));

        assure("", "resultString".equals(result));
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
        assure("", result instanceof Any
               && (TypeDescription.getTypeDescription(((Any) result).getType()).
                   getZClass() == void.class));

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
        assure("", result == null);

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
        assure("", result.equals(new Integer(501)));
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
        private final LinkedList queue = new LinkedList();
    }
}
