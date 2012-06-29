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

package com.sun.star.comp.connections;

import org.junit.Test;
import static org.junit.Assert.*;

public final class PipedConnection_Test {
    @Test public void test() throws Exception {
        PipedConnection rightSide = new PipedConnection(new Object[0]);
        PipedConnection leftSide = new PipedConnection(new Object[]{rightSide});

        byte theByte[] = new byte[1];

        Reader reader = new Reader(rightSide, theByte);
        Writer writer = new Writer(leftSide, theByte, reader);

        reader.start();
        writer.start();

        Thread.sleep(2000);

        writer.term();
        writer.join();

        reader.join();

        assertTrue(writer._state);
        assertTrue(reader._state);
    }

    static class Reader extends Thread {
        PipedConnection _pipedConnection;
        byte _theByte[];
        boolean _quit;
        boolean _state = false;

        Reader(PipedConnection pipedConnection, byte theByte[]) {
            _pipedConnection = pipedConnection;
            _theByte = theByte;
        }

        public void run() {
            try {
                byte bytes[][] = new byte[1][];

                while(!_quit) {
                    int read = _pipedConnection.read(bytes, 1);

                    if(read == 1) {
//                          System.err.println("read :" + bytes[0][0]);

                        if(_theByte[0] != bytes[0][0])
                            throw new NullPointerException();

                        synchronized(this) {
                            notifyAll();
                        }
                    }
                    else
                        _quit = true; // EOF
                }

                _pipedConnection.close();
                _state = true;
            }
            catch(com.sun.star.io.IOException ioException) {
                System.err.println("#### Reader - unexpected:" + ioException);
            }

        }
    }

    static class Writer extends Thread {
        PipedConnection _pipedConnection;
        byte _theByte[];
        Reader _reader;
        boolean _quit;
        boolean _state = false;

        Writer(PipedConnection pipedConnection, byte theByte[], Reader reader) {
            _pipedConnection = pipedConnection;
            _reader = reader;
            _theByte = theByte;
        }

        public void run() {
            try {
                while(!_quit) {
                    synchronized(_reader) {
                        _pipedConnection.write(_theByte);
                        _pipedConnection.flush();
//                          System.err.println("written :" + _theByte[0]);

                        _reader.wait();
                    }
                    ++ _theByte[0];
                }

                _pipedConnection.close();

                _state = true;
            }
            catch(com.sun.star.io.IOException ioException) {
                System.err.println("#### Writer:" + ioException);
            }
            catch(InterruptedException interruptedException) {
                System.err.println("#### Writer:" + interruptedException);
            }
        }

        public void term() {
            _quit = true;
        }
    }
}
