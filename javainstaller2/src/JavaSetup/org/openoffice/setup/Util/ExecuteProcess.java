/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ExecuteProcess.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-03 12:01:07 $
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

package org.openoffice.setup.Util;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Vector;

public class ExecuteProcess {

    private ExecuteProcess() {
    }

    static public int executeProcessReturnValue(String[] command) {
        // usage of String arrays because of blanks in pathes
        int returnValue = 0;

        try {
            Process p = Runtime.getRuntime().exec(command);
            p.waitFor();
            returnValue = p.exitValue();
        } catch ( IOException ioe ) {
            System.err.println("IOError:" + ioe );
        } catch ( InterruptedException ie ) {
            System.err.println("Interrupted Exception:" + ie );
        }

        return returnValue;
    }

    static public int executeProcessReturnVector(String[] command, Vector returnVector, Vector returnErrorVector) {
        // usage of String arrays because of blanks in pathes
        int returnValue = -3;

        try {
            Process p = Runtime.getRuntime().exec(command);

            BufferedReader in = new BufferedReader(new InputStreamReader(p.getInputStream()));
            BufferedReader errorIn = new BufferedReader(new InputStreamReader(p.getErrorStream()));
            for ( String s; ( s = in.readLine()) != null; ) {
                returnVector.add(s);
            }
            for ( String t; ( t = errorIn.readLine()) != null; ) {
                returnErrorVector.add(t);
            }

            p.waitFor();
            returnValue = p.exitValue();

        } catch ( InterruptedException ioe ) {
            System.err.println("Interrupted Exception Error: " + ioe );
        } catch ( IOException ioe ) {
            System.err.println("IOError: " + ioe );
        }

        return returnValue;
    }

    static public int executeProcessReturnVectorEnv(String[] command, String[] envP, Vector returnVector, Vector returnErrorVector) {
        // usage of String arrays because of blanks in pathes
        int returnValue = -3;

        try {
            Process p = Runtime.getRuntime().exec(command, envP);

            // Solaris has to use the ErrorStream (do not log license texts), Linux the InputStream
            BufferedReader in = new BufferedReader(new InputStreamReader(p.getInputStream()));
            BufferedReader errorIn = new BufferedReader(new InputStreamReader(p.getErrorStream()));
            for ( String s; ( s = in.readLine()) != null; ) {
                returnVector.add(s);
            }
            for ( String t; ( t = errorIn.readLine()) != null; ) {
                returnErrorVector.add(t);
            }

            p.waitFor();
            returnValue = p.exitValue();

        } catch ( InterruptedException ioe ) {
            System.err.println("Interrupted Exception Error: " + ioe );
        } catch ( IOException ioe ) {
            System.err.println("IOError: " + ioe );
        }

        return returnValue;
    }

}
