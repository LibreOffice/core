/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Protocol_Test.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:15:15 $
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

import com.sun.star.lib.uno.environments.remote.IMessage;
import com.sun.star.lib.uno.environments.remote.IProtocol;
import com.sun.star.lib.uno.environments.remote.ThreadId;
import com.sun.star.lib.uno.typedesc.TypeDescription;
import com.sun.star.uno.Any;
import com.sun.star.uno.IBridge;
import com.sun.star.uno.Type;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.lang.reflect.Constructor;

public final class Protocol_Test extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws Exception {
        String protocolDescription = "urp";

        IBridge iBrige = new TestBridge();

        Class protocol_class = Class.forName("com.sun.star.lib.uno.protocols." + protocolDescription + "." + protocolDescription);
        Constructor protocol_constructor = protocol_class.getConstructor(new Class[] {IBridge.class});

//          XInstanceProvider xInstanceProvider = new InstanceProvider();
        IProtocol iSender = (IProtocol)protocol_constructor.newInstance(new Object[]{iBrige});
        IProtocol iReciever = (IProtocol)protocol_constructor.newInstance(new Object[]{iBrige});




        TestObject testObject = new TestObject();
        String oId = (String)iBrige.mapInterfaceTo(testObject, new Type(XInterface.class));

        testCall(iSender, iReciever, oId);
        testCallWithInParameter(iSender, iReciever, oId);
        testCallWithOutParameter(iSender, iReciever, oId);
        testCallWithInOutParameter(iSender, iReciever, oId);
        testCallWithResult(iSender, iReciever, oId);
        testCallWhichRaisesException(iSender, iReciever, oId);
        testCallWithIn_Out_InOut_Paramters_and_result(iSender, iReciever, oId);
        testCallWhichReturnsAny(iSender, iReciever, oId);
    }

    static ByteArrayInputStream sendRequest(IProtocol sender,
                                            String oid,
                                            TypeDescription typeDescription,
                                            String operation,
                                            ThreadId threadId,
                                            Object params[],
                                            Boolean synchron[],
                                            Boolean mustReply[]) throws Exception
    {
        sender.writeRequest(oid, typeDescription, operation, threadId, params, synchron, mustReply);

        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        DataOutputStream dataOutputStream = new DataOutputStream(byteArrayOutputStream);

        sender.flush(dataOutputStream);

        return new ByteArrayInputStream(byteArrayOutputStream.toByteArray());
    }

    static ByteArrayInputStream sendReply(IProtocol sender, boolean exception, ThreadId threadId, Object result) throws Exception {
        sender.writeReply(exception,
                          threadId,
                          result);


        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        DataOutputStream dataOutputStream = new DataOutputStream(byteArrayOutputStream);

        sender.flush(dataOutputStream);

        return new ByteArrayInputStream(byteArrayOutputStream.toByteArray());
    }

    static IMessage receiveMessage(IProtocol receiver, ByteArrayInputStream byteArrayInputStream) throws Exception {
        return receiver.readMessage(byteArrayInputStream);
    }

    public void testCall(IProtocol iSender, IProtocol iReciever, String oId) throws Exception {
        // send an ordinary request
        IMessage iMessage = receiveMessage(iReciever, sendRequest(iSender,
                                                                  oId,
                                                                  TypeDescription.getTypeDescription(TestXInterface.class),
                                                                  "method",
                                                                  new ThreadId(new byte[]{0, 1}),
                                                                  new Object[0],
                                                                  new Boolean[1],
                                                                  new Boolean[1]));
        iMessage.getData(new Object[1][]);

        // send a reply
        iMessage = receiveMessage(iSender, sendReply(iReciever,
                                                     false,
                                                     new ThreadId(new byte[]{0, 1}),
                                                     null));
        iMessage.getData(new Object[1][]);
    }

    public void testCallWithInParameter(IProtocol iSender, IProtocol iReciever, String oId) throws Exception {
        // send an ordinary request
        IMessage iMessage = receiveMessage(iReciever, sendRequest(iSender,
                                                                  oId,
                                                                  TypeDescription.getTypeDescription(TestXInterface.class),
                                                                  "methodWithInParameter",
                                                                  new ThreadId(new byte[]{0, 1}),
                                                                  new Object[]{"hallo"},
                                                                  new Boolean[1],
                                                                  new Boolean[1]));
        Object t_params[][] = new Object[1][];
        iMessage.getData(t_params);
        assure("", "hallo".equals((String)t_params[0][0]));

        // send a reply
        iMessage = receiveMessage(iSender, sendReply(iReciever,
                                                     false,
                                                     new ThreadId(new byte[]{0, 1}),
                                                     null));
        iMessage.getData(new Object[1][]);
    }

    public void testCallWithOutParameter(IProtocol iSender, IProtocol iReciever, String oId) throws Exception {
        Object params[] = new Object[]{new String[1]};
        IMessage iMessage = receiveMessage(iReciever, sendRequest(iSender,
                                                                  oId,
                                                                  TypeDescription.getTypeDescription(TestXInterface.class),
                                                                  "methodWithOutParameter",
                                                                  new ThreadId(new byte[]{0, 1}),
                                                                  params,
                                                                  new Boolean[1],
                                                                  new Boolean[1]));


        Object t_params[][] = new Object[1][];
        iMessage.getData(t_params);
        ((String [])t_params[0][0])[0] = "testString";

        // send an exception as reply
        iMessage = receiveMessage(iSender, sendReply(iReciever,
                                                     false,
                                                     new ThreadId(new byte[]{0, 1}),
                                                     null));

        iMessage.getData(new Object[1][]);

        assure("", "testString".equals(((String [])params[0])[0]));
    }

    public void testCallWithInOutParameter(IProtocol iSender, IProtocol iReciever, String oId) throws Exception {
        Object params[] = new Object[]{new String[]{"inString"}};
        IMessage iMessage = receiveMessage(iReciever, sendRequest(iSender,
                                                                  oId,
                                                                  TypeDescription.getTypeDescription(TestXInterface.class),
                                                                  "methodWithInOutParameter",
                                                                  new ThreadId(new byte[]{0, 1}),
                                                                  params,
                                                                  new Boolean[1],
                                                                  new Boolean[1]));


        Object t_params[][] = new Object[1][];
        iMessage.getData(t_params);
        assure("", "inString".equals(((String [])t_params[0][0])[0]));

        // provide reply
        ((String [])t_params[0][0])[0] = "outString";

        // send an exception as reply
        iMessage = receiveMessage(iSender, sendReply(iReciever,
                                                     false,
                                                     new ThreadId(new byte[]{0, 1}),
                                                     null));

        iMessage.getData(new Object[1][]);

        assure("", "outString".equals(((String [])params[0])[0]));
    }

    public void testCallWithResult(IProtocol iSender, IProtocol iReciever, String oId) throws Exception {
        // send an ordinary request
        IMessage iMessage = receiveMessage(iReciever, sendRequest(iSender,
                                                                  oId,
                                                                  TypeDescription.getTypeDescription(TestXInterface.class),
                                                                  "methodWithResult",
                                                                  new ThreadId(new byte[]{0, 1}),
                                                                  new Object[0],
                                                                  new Boolean[1],
                                                                  new Boolean[1]));
        iMessage.getData(new Object[1][]);

        // send a reply
        iMessage = receiveMessage(iSender, sendReply(iReciever,
                                                     false,
                                                     new ThreadId(new byte[]{0, 1}),
                                                     "resultString"));
        Object result = iMessage.getData(new Object[1][]);

        assure("", "resultString".equals(result));
    }

    public void testCallWhichRaisesException(IProtocol iSender, IProtocol iReciever, String oId) throws Exception {
        // send a second request
        IMessage iMessage = receiveMessage(iReciever, sendRequest(iSender,
                                                                  oId,
                                                                  TypeDescription.getTypeDescription(TestXInterface.class),
                                                                  "method",
                                                                  new ThreadId(new byte[]{0, 1}),
                                                                  new Object[0],
                                                                  new Boolean[1],
                                                                  new Boolean[1]));
        iMessage.getData(new Object[1][]);

        // send an exception as reply
        iMessage = receiveMessage(iSender, sendReply(iReciever,
                                                     true,
                                                     new ThreadId(new byte[]{0, 1}),
                                                     new com.sun.star.uno.RuntimeException("test the exception")));

        Object result = iMessage.getData(new Object[1][]);

        assure("", result instanceof com.sun.star.uno.RuntimeException);
    }

    public void testCallWithIn_Out_InOut_Paramters_and_result(IProtocol iSender, IProtocol iReciever, String oId) throws Exception {
        Object params[] = new Object[]{"hallo", new String[1], new String[]{"inOutString"}};
        IMessage iMessage = receiveMessage(iReciever, sendRequest(iSender,
                                                                  oId,
                                                                  TypeDescription.getTypeDescription(TestXInterface.class),
                                                                  "MethodWithIn_Out_InOut_Paramters_and_result",
                                                                  new ThreadId(new byte[]{0, 1}),
                                                                  params,
                                                                  new Boolean[1],
                                                                  new Boolean[1]));

        Object t_params[][] = new Object[1][];
        iMessage.getData(t_params);

        assure("", "hallo".equals((String)t_params[0][0]));

        assure("", "inOutString".equals(((String [])t_params[0][2])[0]));

        ((String [])t_params[0][1])[0] = "outString";
        ((String [])t_params[0][2])[0] = "inOutString_res";

        // send an exception as reply
        iMessage = receiveMessage(iSender, sendReply(iReciever,
                                                     false,
                                                     new ThreadId(new byte[]{0, 1}),
                                                     "resultString"));

        Object result = iMessage.getData(new Object[1][]);
        assure("", "outString".equals(((String [])params[1])[0]));

        assure("", "inOutString_res".equals(((String [])params[2])[0]));

        assure("", "resultString".equals(result));
    }

    public void testCallWhichReturnsAny(IProtocol iSender, IProtocol iReciever, String oId) throws Exception {
        // send an ordinary request
        IMessage iMessage = receiveMessage(iReciever, sendRequest(iSender,
                                                                  oId,
                                                                  TypeDescription.getTypeDescription(TestXInterface.class),
                                                                  "returnAny",
                                                                  new ThreadId(new byte[]{0, 1}),
                                                                  null,
                                                                  new Boolean[1],
                                                                  new Boolean[1]));
        // send a reply
        iMessage = receiveMessage(iSender, sendReply(iReciever,
                                                     false,
                                                     new ThreadId(new byte[]{0, 1}),
                                                     new Any(Void.class, null)));
        Object result = iMessage.getData(new Object[1][]);
        assure("", result instanceof Any
               && (TypeDescription.getTypeDescription(((Any) result).getType()).
                   getZClass() == void.class));

        // send an ordinary request
        iMessage = receiveMessage(iReciever, sendRequest(iSender,
                                                                  oId,
                                                                  TypeDescription.getTypeDescription(TestXInterface.class),
                                                                  "returnAny",
                                                                  new ThreadId(new byte[]{0, 1}),
                                                                  null,
                                                                  new Boolean[1],
                                                                  new Boolean[1]));
        // send a reply
        iMessage = receiveMessage(iSender, sendReply(iReciever,
                                                     false,
                                                     new ThreadId(new byte[]{0, 1}),
                                                     new Any(XInterface.class, null)));
        result = iMessage.getData(new Object[1][]);
        assure("", result == null);

        // send an ordinary request
        iMessage = receiveMessage(iReciever, sendRequest(iSender,
                                                                  oId,
                                                                  TypeDescription.getTypeDescription(TestXInterface.class),
                                                                  "returnAny",
                                                                  new ThreadId(new byte[]{0, 1}),
                                                                  null,
                                                                  new Boolean[1],
                                                                  new Boolean[1]));
        // send a reply
        iMessage = receiveMessage(iSender, sendReply(iReciever,
                                                     false,
                                                     new ThreadId(new byte[]{0, 1}),
                                                     new Integer(501)));
        result = iMessage.getData(new Object[1][]);
        assure("", result.equals(new Integer(501)));
    }
}
