/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
