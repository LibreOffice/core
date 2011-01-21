/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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

package httpserv;

import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;

/**
 *
 * @author jochen
 */
public class Main {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        try {

            Option[] opts = new Option[2];
            opts[0] = new Option("--help", "-h", false);
            opts[1] = new Option("--accept", "-a", true);
            if (!parseOptions(args, opts)) {
                return;
            }
            HttpServer server = HttpServer.create(
                    new InetSocketAddress((Integer) opts[1].value), 0);
            server.createContext("/", new MyHandler());
            server.setExecutor(null);
            server.start();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }        // TODO code application logic here

    static boolean parseOptions(String[] args, Option[] inout_options) {
        if (args.length == 0) {
            printUsage();
            return false;
        }

        boolean bWrongArgs = true;
        Option currentOpt = null;

        for (String s : args) {
            // get the value for an option
            if (currentOpt != null && currentOpt.bHasValue) {
                //now we expect the value for the option
                //check the type
                try {
                    if (currentOpt.sLong.equals("--accept")) {
                        currentOpt.value = Integer.decode(s);
                    }
                } catch (Exception e ) {
                    printUsage();
                    return false;
                }
                currentOpt = null;
                continue;
            } else {
                currentOpt = null;
            }


            // get the option
            for (Option o : inout_options) {
                if (s.equals(o.sLong) || s.equals(o.sShort)) {
                    bWrongArgs = false;
                    //special handling for --help
                    if (o.sLong.equals("--help")) {
                        printUsage();
                        return false;
                    }
                    else
                    {
                        currentOpt = o;
                        if (!o.bHasValue) {
                            o.bSet = true;
                        }
                        break;
                    }
                }
            }
        }

        if (bWrongArgs) {
            printUsage();
            return false;
        }
        return true;
    }

    static void printUsage() {
        String usage = new String(
                "Usage: \n" +
                "java -jar httpserv [options] \n" +
                "\n" +
                "Options are: \n" +
                "-h --help \t this help \n" +
                "-a --accept port \t the port number to which this server listens \n");
        System.out.println(usage);
    }
}

class MyHandler implements HttpHandler {

    public void handle(HttpExchange xchange) throws IOException {
        try {
            //First get the path to the file
            File fileCurrent = new File(".");
            String sRequestPath = xchange.getRequestURI().getPath();
            System.out.println("requested: " + sRequestPath);
            File fileRequest = new File(new File(".").getCanonicalPath(), sRequestPath);
            if (!fileRequest.exists()) {
                throw new Exception("The file " + fileRequest.toString() + " does not exist!\n");
            }
            else if (fileRequest.isDirectory()) {
                throw new Exception(fileRequest.toString() + " is a directory!\n");
            }


            //Read the file into a byte array
            byte[] data = new byte[(int) fileRequest.length()];
            FileInputStream fr = new FileInputStream(fileRequest);
            int count = fr.read(data);

            //set the Content-type header
            Headers h = xchange.getResponseHeaders();
            String canonicalPath = fileRequest.getCanonicalPath();
            int lastIndex = canonicalPath.lastIndexOf(".");
            String fileExtension = canonicalPath.substring(lastIndex + 1);

            if (fileExtension.equalsIgnoreCase("crl"))
            {
                //h.set("Content-Type","application/x-pkcs7-crl");
                h.set("Content-Type","application/pkix-crl");
            }
            else if (fileExtension.equalsIgnoreCase("crt")
                    || fileExtension.equalsIgnoreCase("cer")
                    || fileExtension.equalsIgnoreCase("der"))
            {
                h.set("Content-Type", "application/x-x509-ca-cert");
            }

            //write out the requested file
            xchange.sendResponseHeaders(200, data.length);
            OutputStream os = xchange.getResponseBody();
            os.write(data);
            os.close();
            System.out.println("delivered: " + fileRequest.toString());

        } catch (Exception e) {
            xchange.sendResponseHeaders(404, e.getMessage().length());
            OutputStream os = xchange.getResponseBody();
            os.write(e.getMessage().getBytes());
            os.close();
            System.out.println("Error: " + e.getMessage());
        }
    }
}

class Option {

    Option(String _sLong, String _sShort, boolean _bHasValue) {
        sLong = _sLong;
        sShort = _sShort;
        bHasValue = _bHasValue;
    }
    String sLong;
    String sShort;
    boolean bHasValue;
    Object value;
    //indicates if this option was set if it does not need a value. Otherwise value
    //is set.
    boolean bSet;
}


