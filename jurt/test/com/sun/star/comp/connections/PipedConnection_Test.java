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

import com.sun.star.io.IOException;
import org.junit.Test;
import static org.junit.Assert.*;

public final class PipedConnection_Test {
    private static final int ROUNDS = 2000;

    @Test public void test() throws Exception {
        PipedConnection rightSide = new PipedConnection(new Object[0]);
        PipedConnection leftSide = new PipedConnection(new Object[]{rightSide});

        Reader reader = new Reader(rightSide);
        Writer writer = new Writer(leftSide);

        reader.start();
        writer.start();

        writer.join();
        reader.join();

        assertTrue(writer._state);
        assertTrue(reader._state);
        assertEquals(ROUNDS, reader._rounds);
    }

    private static class Reader extends Thread {
        PipedConnection _pipedConnection;
        boolean _state = false;
        int _rounds = 0;

        Reader(PipedConnection pipedConnection) {
            _pipedConnection = pipedConnection;
        }

        public void run() {
            try {
                for (byte v = 0;; v++) {
                    byte[][] b = new byte[1][];
                    int n = _pipedConnection.read(b, 1);
                    if (n == 0) {
                        break;
                    }
                    assertEquals(1, n);
                    assertEquals(1, b[0].length);
                    assertEquals(v, b[0][0]);
                    ++_rounds;
                }
                _pipedConnection.close();
                _state = true;
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
        }
    }

    private static class Writer extends Thread {
        PipedConnection _pipedConnection;
        boolean _state = false;

        Writer(PipedConnection pipedConnection) {
            _pipedConnection = pipedConnection;
        }

        public void run() {
            try {
                byte v = 0;
                for (int i = 0; i != ROUNDS; ++i) {
                    byte[] b = new byte[] { v++ };
                    _pipedConnection.write(b);
                    _pipedConnection.flush();
                }
                _pipedConnection.close();
                _state = true;
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
        }
    }
}
