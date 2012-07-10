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

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;


/**
 * The class is used to send a statement to the automation server.
 *
 *
 */
public class CommandCaller implements CommunicationListener, Constant {

    private ByteArrayOutputStream dataOutput = new ByteArrayOutputStream(1024);

    private ByteArrayInputStream dataInput = null;

    private CommunicationManager communicationManager = null;

    private int sequence = 0;

    private WinInfoReceiver winInfoReceiver = null;

    private boolean receivingWinInfo = false;

    /**Store the response**/
    private Object response = null;

    private SmartId responseExceptionId = null;

    private String responseExceptionMessage = null;

    /**A variable to indicate if the server answered the request **/
    private boolean answered = false;

    public CommandCaller(CommunicationManager communicationManager) {
        this.communicationManager = communicationManager;
        communicationManager.addListener(this);
    }

    private void write(byte[] bytes) {
        try {
            dataOutput.write(bytes);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private byte[] read(int len) {
        byte[] bytes = new byte[len];
        try {
            dataInput.read(bytes);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return bytes;
    }


    /**
     * Write the data type
     * @param i
     */
    private void writeChar(int i) {
        byte[] bytes = new byte[2];
        bytes[1] = (byte) ((i & 0xFF00) >> 8);
        bytes[0] = (byte) (i & 0x00FF);
        write(bytes);
    }

    private int readChar() {
        byte[] bytes = read(2);
        return (bytes[0] & 0x00FF) + ((bytes[1] & 0x00FF) << 8);
    }


    /**
     * Check the type of next data
     * The bytes used to identify the type will not be read out from the stream
     * @return
     */
    private int nextType() {
        dataInput.mark(0);
        int type = readChar();
        dataInput.reset();
        return type;
    }

    /**
     * Write an unsigned 16-bit integer
     * @param i
     */
    private void writeUShort(int i) {
        writeChar(BinUSHORT);
        writeChar(i);
    }

    /**
     * Read an unsigned 16-bit integer
     * @return
     */
    private int readUShort() {
        if (readChar() != BinUSHORT)
            throw new RuntimeException("Bad data!");

        byte[] bytes = read(2);
        return (bytes[0] & 0x00FF) + ((bytes[1] & 0x00FF) << 8);
    }

    /**
     * Write an unsigned 32-bit integer
     * @param i
     */
    private void writeULong(long i)  {
        writeChar(BinULONG);
        byte[] bytes = new byte[4];
        bytes[3] = (byte) ((i & 0xFF000000L) >> 24);
        bytes[2] = (byte) ((i & 0x00FF0000L) >> 16);
        bytes[1] = (byte) ((i & 0x0000FF00L) >> 8);
        bytes[0] = (byte) ((i & 0x000000FFL));
        write(bytes);
    }

    /**
     * Read an unsigned 32-bit integer
     * @return
     */
    private long readULong() {
        if (readChar() != BinULONG)
            throw new RuntimeException("Bad data!");
        byte[] bytes = read(4);
        return (bytes[0] & 0x00FFL) + ((bytes[1] & 0x00FFL) << 8) + ((bytes[2] & 0x00FFL) << 16) + ((bytes[3] & 0x00FFL) << 24);
    }

    /**
     * Write boolean
     * @param bBool
     */
    private void writeBoolean(boolean bBool) {
        writeChar(BinBool);
        write(new byte[] { bBool ? (byte) 1 : 0 });
    }

    /**
     * Read boolean
     * @return
     */
    private boolean readBoolean() {
        if (readChar() != BinBool)
            throw new RuntimeException("Bad data!");
        byte[] bytes = read(1);
        return bytes[0] != 0;
    }

    /**
     * Write a string
     * @param str
     */
    private void writeString(String str) {
        writeChar(BinString);
        int len =  str.length();
        if (len > 0xFFFF) {
            throw new RuntimeException("String is too long.");
        }
        writeChar(len);
        char[] chars = str.toCharArray();
        for (int i = 0 ; i < len; i++)
            writeChar(chars[i]);
    }

    /**
     * Read a string
     * @return
     */
    private String readString() {
        if (readChar() != BinString)
            throw new RuntimeException("Bad data!");
        int len = readChar();
        char[] chars = new char[len];
        for (int i = 0; i < len; i++) {
            chars[i] = (char) readChar();
        }
        return new String(chars);
    }

    private void writeParams(Object[] args) {
        int nParams = PARAM_NONE;
        int nNr1=0;
        int nNr2=0;
        int nNr3=0;
        int nNr4=0;
        long nLNr1=0;
        String aString1=null;
        String aString2=null;
        boolean bBool1=false;
        boolean bBool2=false;

        if (args != null) {
            for (int i = 0; i < args.length; i++) {
                if (args[i] instanceof Short || args[i] instanceof Integer) {
                    int c =  ((Number) args[i]).intValue();
                    if ((nParams & PARAM_USHORT_1) == 0) {
                        nParams |= PARAM_USHORT_1;
                        nNr1 = c;
                    } else if ((nParams & PARAM_USHORT_2) == 0) {
                        nParams |= PARAM_USHORT_2;
                        nNr2 = c;
                    } else if ((nParams & PARAM_USHORT_3) == 0) {
                        nParams |= PARAM_USHORT_3;
                        nNr3 = c;
                    } else if ((nParams & PARAM_USHORT_4) == 0) {
                        nParams |= PARAM_USHORT_4;
                        nNr4 = c;
                    } else {
                        //TODO error
                    }
                } else if (args[i] instanceof Long) {
                    long l = ((Long) args[i]).longValue();
                    nParams |= PARAM_ULONG_1;
                    nLNr1 = l;
                } else if (args[i] instanceof Boolean) {
                    if ((nParams & PARAM_BOOL_1) == 0) {
                        nParams |= PARAM_BOOL_1;
                        bBool1 = ((Boolean) args[i]).booleanValue();
                    } else if ((nParams & PARAM_BOOL_2) == 0) {
                        nParams |= PARAM_BOOL_2;
                        bBool2 = ((Boolean) args[i]).booleanValue();
                    } else {
                        //TODO error
                    }

                } else if (args[i] instanceof String) {
                    if ((nParams & PARAM_STR_1) == 0) {
                        nParams |= PARAM_STR_1;
                        aString1 = (String) args[i];
                    } else if ((nParams & PARAM_STR_2) == 0) {
                        nParams |= PARAM_STR_2;
                        aString2 = (String) args[i];
                    } else {
                        //TODO error
                    }
                }
            }
        }

        writeUShort(nParams);
        if ((nParams & PARAM_USHORT_1) != 0) {
            writeUShort(nNr1);
        }

        if ((nParams & PARAM_USHORT_2) != 0) {
            writeUShort(nNr2);
        }

        if ((nParams & PARAM_USHORT_3) != 0) {
            writeUShort(nNr3);
        }

        if ((nParams & PARAM_USHORT_4) != 0) {
            writeUShort(nNr4);
        }

        if ((nParams & PARAM_ULONG_1) != 0) {
            writeULong(nLNr1);
        }

        if ((nParams & PARAM_STR_1) != 0) {
            writeString(aString1);
        }

        if ((nParams & PARAM_STR_2) != 0) {
            writeString(aString2);
        }

        if ((nParams & PARAM_BOOL_1) != 0) {
            writeBoolean(bBool1);
        }
        if ((nParams & PARAM_BOOL_2) != 0) {
            writeBoolean(bBool2);
        }

    }

    private void send() {
        byte[] data = dataOutput.toByteArray();
        dataOutput.reset();
        int protocal = CM_PROTOCOL_OLDSTYLE;
        byte[] header = new byte[]{(byte)((protocal >>> 8) & 0xFF), (byte) ((protocal >>> 0) & 0xFF)};
        communicationManager.sendPackage(CH_SimpleMultiChannel, header, data);
    }

    /**
     * The data arrives
     */
    public synchronized void received(int headerType, byte[] header, byte[] data) {
        if (headerType != CommunicationManager.CH_Handshake) {
            dataInput = new ByteArrayInputStream(data);
            handleResponse();
            dataInput = null;
            answered = true;
            notifyAll();
        }
    }

    /**
     * This method is called when the communication is started.
     */
    public void start() {
    }

    /**
     * This method is called when the communication is closed
     */
    public synchronized void stop() {
        answered = true;
        notifyAll();
    }


    private SmartId readId() {
        int type = nextType();
        if ( type == BinString) {
            return new SmartId(readString());
        } else if (type == BinULONG) {
            return new SmartId(readULong());
        }

        throw new RuntimeException("Bad data!");
    }

    private void handleResponse() {
        this.response = null;
        this.responseExceptionId = null;
        this.responseExceptionMessage = null;

        while (dataInput.available() >= 2) {
            int id = readUShort();
            switch (id) {
            case SIReturn:
                int returnType = readUShort();
                SmartId sid = readId();
                int nNr1 = 0;
                long nLNr1 = 0;
                String aString1 = null;
                boolean bBool1 = false;
                int params = readUShort();
                if ((params & PARAM_USHORT_1) != 0)
                    nNr1 = readUShort();
                if ((params & PARAM_ULONG_1) != 0)
                    nLNr1 = readULong();
                if ((params & PARAM_STR_1) != 0)
                    aString1 = readString();
                if ((params & PARAM_BOOL_1) != 0)
                    bBool1 = readBoolean();
                if ((params & PARAM_SBXVALUE_1) != 0) {
                    // Don't support????
                }

                switch (returnType) {
                case RET_Sequence:
                    if (sid.getId() != sequence)
                        this.responseExceptionMessage = "Bad sequence of command.";
                    break;
                case RET_Value:
                    List<Object> ret = new ArrayList<Object>();
                    if ((params & PARAM_USHORT_1) != 0)
                        ret.add(new Integer(nNr1));
                    if ((params & PARAM_ULONG_1) != 0)
                        ret.add(new Long(nLNr1));
                    if ((params & PARAM_STR_1) != 0)
                        ret.add(aString1);
                    if ((params & PARAM_BOOL_1) != 0)
                        ret.add(new Boolean(bBool1));
                    this.response = ret.size() == 1 ? ret.get(0): ret;
                    break;
                case RET_WinInfo:
                    if (bBool1) {
                        receivingWinInfo = true;
                        if (winInfoReceiver != null)
                            winInfoReceiver.onStartReceiving();
                    } else {
                        if (winInfoReceiver != null)
                            winInfoReceiver.addWinInfo(sid, nLNr1, aString1);
                    }
                    break;
                }

                break;
            case SIReturnError:
                this.responseExceptionId = readId();
                this.responseExceptionMessage = readString();
                break;
            }
        }

        if (receivingWinInfo) {
            if (winInfoReceiver != null)
                winInfoReceiver.onFinishReceiving();
            receivingWinInfo = false;
        }
    }


    public void setWinInfoReceiver(WinInfoReceiver receiver) {
        this.winInfoReceiver = receiver;
    }

    private void callFlow(int nArt) {
        writeUShort(SIFlow);
        writeUShort(nArt);
        writeUShort(PARAM_NONE);
    }


    private void callFlow(int nArt, long nLNr1) {
        writeUShort(SIFlow);
        writeUShort(nArt);
        writeUShort(PARAM_ULONG_1);
        writeULong(nLNr1);
    }

    /**
     * Tell automation server to execute a 'StatementCommand'
     * @param methodId The method ID
     * @param args the arguments. The arguments can be Integer, Long, Boolean and String.
     * @return The return can be Integer, Long, String and Boolean or an Object[] includes these types of object.
     */
    public synchronized Object callCommand(int methodId, Object... args) {
        beginBlock();
        writeUShort(SICommand);
        writeUShort(methodId);
        writeParams(args);
        endBlock();

        if ((methodId & M_WITH_RETURN) != 0) {
            return response;
        }

        return null;
    }

    public synchronized Object callCommand(int methodId) {
        return callCommand(methodId, (Object)null);
    }

    /**
     *  Tell automation server to execute a 'StatementControl'
     * @param id the control ID
     * @param methodId the method ID defined Constant class
     * @param args the arguments. The arguments can be Integer, Long, Boolean and String.
     * @return The return can be Integer, Long, String and Boolean or an Object[] includes these types of object.
     */
    public synchronized Object callControl(String id, int methodId, Object... args){
        beginBlock();
        try {
            long noId = Long.parseLong(id);
            writeUShort(SIControl);
            writeULong(noId);
        } catch (NumberFormatException e) {
            writeUShort(SIStringControl);
            writeString(id);
        }
        writeUShort(methodId);
        writeParams(args);
        endBlock();

        if ((methodId & M_WITH_RETURN) != 0) {
            return response;
        }

        return null;
    }

    /**
     * Tell automation server to execute a 'StatementUNOSlot'
     * @param url the UNO slot url
     */
    public synchronized void callUNOSlot(String url) {
        beginBlock();
        writeUShort(SIUnoSlot);
        writeString(url);
        endBlock();
    }

    /**
     * Tell automation server to execute a 'StatementSlot'
     * @param id the slot ID
     * @param args the slot args
     */
    public synchronized void callSlot(int id, Object... args) {
        beginBlock();
        writeUShort(SISlot);
        writeUShort(id);
        if (args.length % 2 != 0)
            throw new RuntimeException("bad arg");
        writeUShort(args.length / 2);
        for (int i = 0; i < args.length; i++) {
            if (!(args[i] instanceof String))
                throw new RuntimeException("bad arg");
            writeString((String)args[i]);
            i++;
            if (args[i] instanceof Boolean) {
                writeBoolean((Boolean)args[i]);
            } else if (args[i] instanceof String) {
                writeString((String)args[i]);
            } else if (args[i] instanceof Short || args[i] instanceof Integer) {
                writeUShort(((Number) args[i]).intValue());
            } else if (args[i] instanceof Long) {
                writeULong((Long) args[i]);
            } else {
                throw new RuntimeException("bad arg");
            }
        }
        endBlock();
    }


    private void beginBlock() {
        callFlow(F_Sequence, ++sequence);
    }

    private void endBlock() {
        callFlow(F_EndCommandBlock);
        answered = false;
        send();
        int MAX_RETRY = 240;//max waiting time is two minutes.
        for (int i = 0; !answered && i < MAX_RETRY; i++) {
            try {
                wait(500);
            } catch (InterruptedException e) {

            }
        }

        // Still answered
        if (!answered) {
            communicationManager.stop();
            throw new CommunicationException("Failed to get data from automation server!");
        }

        if (responseExceptionId != null || responseExceptionMessage != null)
            throw new VclHookException(responseExceptionId, responseExceptionMessage);

    }
}
