/*************************************************************************
 *
 *  $RCSfile: PipedConnection_Test.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:27:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.comp.connections;


import java.util.Vector;


public class PipedConnection_Test {
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

    static public boolean test(Vector notpassed) throws Exception {
        System.err.println("\tTesting PipedConnection...");

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

        boolean passed = writer._state && reader._state;

        System.err.println("PipedConnection - passed? " + passed + " transmitted:" + theByte[0]);
        if(!passed)
            notpassed.addElement("PipedConnection - passed? " + passed + " transmitted:" + theByte[0]);

        return passed;
    }

    static public void main(String argv[]) throws Exception {
        test(null);
    }
}
