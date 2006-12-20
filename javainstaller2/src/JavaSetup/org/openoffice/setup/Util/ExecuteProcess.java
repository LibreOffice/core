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
