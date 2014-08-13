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

package com.sun.star.lib.uno.helper;
import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Object representation and parsing of Uno Urls,
 * which allow to locate a named Uno object in a
 * different process. An Uno Url consists of the
 * specification of a connection, protocol and
 * rootOid delimited with a ';'.
 * The syntax of an Uno Url is
 *
 * <code>
 * [uno:]connection-type,parameters;protocol-name,parameters;objectname";
 * </code>
 *
 * An example Uno Url will look like this:
 *
 * <code>
 * socket,host=localhost,port=2002;urp;StarOffice.ServiceManager
 * </code>
 *
 * For more information about Uno Url please consult
 * <a href="http://udk.openoffice.org/common/man/spec/uno-url.html">
 * http://udk.openoffice.org/common/man/spec/uno-url.html</a>
 *
 * Usage:
 *
 * <code>
 *   UnoUrl url = UnoUrl.parseUnoUrl("socket,host=localhost,port=2002;urp;StarOffice.ServiceManager");
 * </code>
 */
public class UnoUrl {

    private static final String FORMAT_ERROR =
        "syntax: [uno:]connection-type,parameters;protocol-name,parameters;objectname";

    private UnoUrlPart connection;
    private UnoUrlPart protocol;
    private String rootOid;

    private static final Pattern patternUnoUrl = Pattern.compile("^\\s*uno\\s*:"
            // connection-type,params;
            + "\\s*(socket|pipe)(\\s*,\\s*(\\w+)=([a-zA-Z0-9\\!$&'()*+-./:?@_~]|([\\%][a-fA-F0-9]{2}))+)*\\s*;"
            // protocol-name,params;
            + "\\s*(\\w+)(\\s*,\\s*(\\w+)=([a-zA-Z0-9\\!$&'()*+,-./:?=@_~]|([\\%][a-fA-F0-9]{2}))+)*\\s*;"
            // ObjectName
            + "\\s*([a-zA-Z0-9\\!$&'()*+,-./:?=@_~]+)");

    static private class UnoUrlPart {

        private String partTypeName;
        private HashMap<String,String> partParameters;
        private String uninterpretedParameterString;

        public UnoUrlPart(
            String uninterpretedParameterString,
            String partTypeName,
            HashMap<String,String> partParameters) {
            this.uninterpretedParameterString = uninterpretedParameterString;
            this.partTypeName = partTypeName;
            this.partParameters = partParameters;
        }

        public String getPartTypeName() {
            return partTypeName;
        }

        public HashMap<String,String> getPartParameters() {
            return partParameters;
        }

        public String getUninterpretedParameterString() {
            return uninterpretedParameterString;
        }

        public String getUninterpretedString() {
            StringBuffer buf = new StringBuffer(partTypeName);
            if (uninterpretedParameterString.length() > 0) {
                buf.append(',');
                buf.append(uninterpretedParameterString);
            }
            return buf.toString();
        }
    }

    private UnoUrl(
        UnoUrlPart connectionPart,
        UnoUrlPart protocolPart,
        String rootOid) {
        this.connection = connectionPart;
        this.protocol = protocolPart;
        this.rootOid = rootOid;
    }

    /**
     * Returns the name of the connection of this
     * Uno Url. Encoded characters are not allowed.
     *
     * @return The connection name as string.
     */
    public String getConnection() {
        return connection.getPartTypeName();
    }

    /**
     * Returns the name of the protocol of this
     * Uno Url. Encoded characters are not allowed.
     *
     * @return The protocol name as string.
     */
    public String getProtocol() {
        return protocol.getPartTypeName();
    }

    /**
     * Return the object name. Encoded character are
     * not allowed.
     *
     * @return The object name as String.
     */
    public String getRootOid() {
        return rootOid;
    }

    /**
     * Returns the protocol parameters as
     * a Hashmap with key/value pairs. Encoded
     * characters like '%41' are decoded.
     *
     * @return a HashMap with key/value pairs for protocol parameters.
     */
    public HashMap<String,String> getProtocolParameters() {
        return protocol.getPartParameters();
    }

    /**
     * Returns the connection parameters as
     * a Hashmap with key/value pairs. Encoded
     * characters like '%41' are decoded.
     *
     * @return a HashMap with key/value pairs for connection parameters.
     */
    public HashMap<String,String> getConnectionParameters() {
        return connection.getPartParameters();
    }

    /**
     * Returns the raw specification of the protocol
     * parameters. Encoded characters like '%41' are
     * not decoded.
     *
     * @return The uninterpreted protocol parameters as string.
     */
    public String getProtocolParametersAsString() {
        return protocol.getUninterpretedParameterString();
    }

    /**
     * Returns the raw specification of the connection
     * parameters. Encoded characters like '%41' are
     * not decoded.
     *
     * @return The uninterpreted connection parameters as string.
     */
    public String getConnectionParametersAsString() {
        return connection.getUninterpretedParameterString();
    }

    /**
     * Returns the raw specification of the protocol
     * name and parameters. Encoded characters like '%41' are
     * not decoded.
     *
     * @return The uninterpreted protocol name and parameters as string.
     */
    public String getProtocolAndParametersAsString() {
        return protocol.getUninterpretedString();
    }

    /**
     * Returns the raw specification of the connection
     * name and parameters. Encoded characters like '%41' are
     * not decoded.
     *
     * @return The uninterpreted connection name and parameters as string.
     */
    public String getConnectionAndParametersAsString() {
        return connection.getUninterpretedString();
    }

    private static String decodeUTF8(String s)
        throws com.sun.star.lang.IllegalArgumentException {

        try {
            if (s.contains("%")) {
                int length = s.length();
                ByteBuffer bb = ByteBuffer.allocate(length);
                for (int i = 0; i < length; i++) {
                    int ch = s.charAt(i);

                    if (ch == '%') {
                        try {
                            ch = Integer.parseInt(s.substring(i+1,i+3),16);
                        } catch (NumberFormatException e) {
                            throw new com.sun.star.lang.IllegalArgumentException(e.toString());
                        }
                        if (ch < 0)
                            throw new com.sun.star.lang.IllegalArgumentException(
                                "Illegal hex characters in escape (%) pattern - negative value");
                        i+=2;
                    }

                    bb.put((byte) (ch & 0xFF));
                }

                byte[] bytes = new byte[bb.position()];
                System.arraycopy(bb.array(), 0, bytes, 0, bytes.length);
                return new String(bytes, "UTF-8");

            } else {

                return new String(s.getBytes(), "UTF-8");

            }
        } catch (UnsupportedEncodingException e) {
            throw new com.sun.star.lang.IllegalArgumentException(
                "Couldn't convert parameter string to UTF-8 string:" + e.getMessage());
        }
    }

    private static HashMap<String,String> buildParamHashMap(String paramString)
        throws com.sun.star.lang.IllegalArgumentException {
        HashMap<String,String> params = new HashMap<String,String>();
        if (paramString.length() > 0){
            for (String param : paramString.split(",")) {
                String[] paramParts = param.split("=",2);
                params.put(paramParts[0].trim(), decodeUTF8(paramParts[1].trim()));
            }
        }
        return params;
    }

    /**
     * Parses the given Uno Url and returns
     * an in memory object representation.
     *
     * @param unoUrl The given uno URl as string.
     * @return Object representation of class UnoUrl.
     * @throws IllegalArgumentException if Url cannot be parsed.
     */
    public static UnoUrl parseUnoUrl(String unoUrl)
        throws com.sun.star.lang.IllegalArgumentException {

        Matcher matcherUnoUrl = patternUnoUrl.matcher(unoUrl);
        if (!matcherUnoUrl.matches()) {
            throw new com.sun.star.lang.IllegalArgumentException("'" + unoUrl + "' is an invalid Uno Url. " + FORMAT_ERROR);
        }

        String[] parts = unoUrl.substring(unoUrl.indexOf(':') + 1).split(";");
        int index;

        String connectionPartName;
        String connectionPartArgs;
        index = parts[0].indexOf(",");
        if (index != -1) {
            connectionPartName = parts[0].substring(0, index).trim();
            connectionPartArgs = parts[0].substring(index + 1).trim();
        } else {
            connectionPartName = parts[0].trim();
            connectionPartArgs = "";
        }
        HashMap<String,String> connectionParams = buildParamHashMap(connectionPartArgs);
        UnoUrlPart connectionPart = new UnoUrlPart(connectionPartArgs, connectionPartName, connectionParams);

        String protocolPartName;
        String protocolPartArgs;
        index = parts[1].indexOf(",");
        if (index != -1) {
            protocolPartName = parts[1].substring(0, index).trim();
            protocolPartArgs = parts[1].substring(index + 1).trim();
        } else {
            protocolPartName = parts[1].trim();
            protocolPartArgs = "";
        }
        HashMap<String,String> protocolParams = buildParamHashMap(protocolPartArgs);
        UnoUrlPart protocolPart = new UnoUrlPart(protocolPartArgs, protocolPartName, protocolParams);

        String rootOid = parts[2].trim();

        return new UnoUrl(connectionPart, protocolPart, rootOid);

    }

}
