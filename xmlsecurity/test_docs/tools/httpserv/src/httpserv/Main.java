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
    }

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
        String usage =
                "Usage: \n" +
                "java -jar httpserv [options] \n" +
                "\n" +
                "Options are: \n" +
                "-h --help \t this help \n" +
                "-a --accept port \t the port number to which this server listens \n";
        System.out.println(usage);
    }
}

class MyHandler implements HttpHandler {

    public void handle(HttpExchange xchange) throws IOException {
        try {
            //First get the path to the file
            String sRequestPath = xchange.getRequestURI().getPath();
            System.out.println("requested: " + sRequestPath);
            File fileRequest = new File(new File(".").getCanonicalPath(), sRequestPath);
            if (!fileRequest.exists()) {
                throw new IOException("The file " + fileRequest + " does not exist!");
            }
            else if (fileRequest.isDirectory()) {
                throw new IOException(fileRequest + " is a directory!");
            }


            //Read the file into a byte array
            byte[] data = new byte[(int) fileRequest.length()];
            FileInputStream fr = new FileInputStream(fileRequest);
            fr.read(data);
            fr.close();

            //set the Content-type header
            Headers h = xchange.getResponseHeaders();
            String canonicalPath = fileRequest.getCanonicalPath();
            int lastIndex = canonicalPath.lastIndexOf(".");
            String fileExtension = canonicalPath.substring(lastIndex + 1);

            if (fileExtension.equalsIgnoreCase("crl"))
            {
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
}


