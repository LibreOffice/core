/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package org.openoffice.test.vcl.client;

import java.io.IOException;



/**
 * The entry to remotely invoke the methods supported by the automation server
 *
 */
public class VclHook {

    private static final String DEFAULT_HOST = "localhost";

    private static final int DEFAULT_PORT = 12479;

    private static CommunicationManager communicationManager = null;

    private static CommandCaller commandCaller = null;

    private static Handshaker handshaker = null;

    static {
        init();
    }

    private static void init() {
        String host = System.getProperty("AutomationServerHost", DEFAULT_HOST);
        int port = DEFAULT_PORT;
        try {
            port = Integer.parseInt(System.getProperty("AutomationServerPort"));
        } catch(NumberFormatException e) {

        }
        communicationManager = new CommunicationManager(host, port);
        commandCaller = new CommandCaller(communicationManager);
        communicationManager.addListener(commandCaller);
        handshaker = new Handshaker(communicationManager);
        communicationManager.addListener(handshaker);
    }

    public static CommandCaller getCommandCaller() {
        return commandCaller;
    }

    public static CommunicationManager getCommunicationManager() {
        return communicationManager;
    }

    public static boolean available() {
        try {
            communicationManager.connect();
        } catch (IOException e) {
            return false;
        }

        return true;
    }

    public static Object invokeControl(SmartId uid, int methodId, Object[] args) {
        return commandCaller.callControl(uid, methodId, args);
    }

    public static Object invokeControl(SmartId uid, int methodId) {
        return commandCaller.callControl(uid, methodId, null);
    }

    public static Object invokeCommand(int methodId, Object... args) {
        return commandCaller.callCommand(methodId, args);
    }

    public static Object invokeCommand(int methodId) {
        return commandCaller.callCommand(methodId, null);
    }

    public static void invokeUNOSlot(String url) {
        commandCaller.callUNOSlot(url);
    }

    public static void invokeSlot(int id) {
        commandCaller.callSlot(id, new Object[]{});
    }

    public static void invokeSlot(int id, String arg0, Object val0) {
        commandCaller.callSlot(id, new Object[]{arg0, val0});
    }

    public static void invokeSlot(int id, String arg0, Object val0, String arg1, Object val1) {
        commandCaller.callSlot(id, new Object[]{arg0, val0, arg1, val1});
    }

    public static void invokeSlot(int id, String arg0, Object val0, String arg1, Object val1, String arg2, Object val2) {
        commandCaller.callSlot(id, new Object[]{arg0, val0, arg1, val1, arg2, val2});
    }
}
