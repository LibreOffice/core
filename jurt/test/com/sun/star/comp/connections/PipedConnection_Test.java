/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PipedConnection_Test.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:09:38 $
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

package com.sun.star.comp.connections;

import complexlib.ComplexTestCase;

public final class PipedConnection_Test extends ComplexTestCase {
    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "test" };
    }

    public void test() throws Exception {
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

        assure("", writer._state && reader._state);
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
