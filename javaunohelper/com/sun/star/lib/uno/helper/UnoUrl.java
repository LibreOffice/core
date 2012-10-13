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
import java.util.HashMap;
import java.util.ArrayList;

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

    private static final String VALUE_CHAR_SET = "!$&'()*+-./:?@_~";
    private static final String OID_CHAR_SET = VALUE_CHAR_SET + ",=";

    private UnoUrlPart connection;
    private UnoUrlPart protocol;
    private String rootOid;

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

    private static int hexToInt(int ch)
        throws com.sun.star.lang.IllegalArgumentException {
        int c = Character.toLowerCase((char) ch);
        boolean isDigit = ('0' <= c && c <= '9');
        boolean isValidChar = ('a' <= c && c <= 'f') || isDigit;

        if (!isValidChar)
            throw new com.sun.star.lang.IllegalArgumentException(
                "Invalid UTF-8 hex byte '" + c + "'.");

        return isDigit ? ch - '0' : 10 + ((char) c - 'a') & 0xF;
    }

    private static String decodeUTF8(String s)
        throws com.sun.star.lang.IllegalArgumentException {
        ArrayList<Integer> v = new ArrayList<Integer>();

        for (int i = 0; i < s.length(); i++) {
            int ch = s.charAt(i);

            if (ch == '%') {
                int hb = hexToInt(s.charAt(++i));
                int lb = hexToInt(s.charAt(++i));
                ch = (hb << 4) | lb;
            }

            v.add(new Integer(ch));
        }

        int size = v.size();
        byte[] bytes = new byte[size];
        for (int i = 0; i < size; i++) {
            Integer anInt = v.get(i);
            bytes[i] = (byte) (anInt.intValue() & 0xFF);
        }

        try {
            return new String(bytes, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw new com.sun.star.lang.IllegalArgumentException(
                "Couldn't convert parameter string to UTF-8 string:" + e.getMessage());
        }
    }

    private static HashMap<String,String> buildParamHashMap(String paramString)
        throws com.sun.star.lang.IllegalArgumentException {
        HashMap<String,String> params = new HashMap<String,String>();

        int pos = 0;

        while (true) {
            char c = ',';
            String aKey = "";
            String aValue = "";

            while ((pos < paramString.length())
                && ((c = paramString.charAt(pos++)) != '=')) {
                aKey += c;
            }

            while ((pos < paramString.length())
                && ((c = paramString.charAt(pos++)) != ',')
                && c != ';') {
                aValue += c;
            }

            if ((aKey.length() > 0) && (aValue.length() > 0)) {

                if (!isAlphaNumeric(aKey)) {
                    throw new com.sun.star.lang.IllegalArgumentException(
                        "The parameter key '"
                            + aKey
                            + "' may only consist of alpha numeric ASCII characters.");
                }

                if (!isValidString(aValue, VALUE_CHAR_SET + "%")) {
                    throw new com.sun.star.lang.IllegalArgumentException(
                        "The parameter value for key '" + aKey + "' contains illegal characters.");
                }

                params.put(aKey, decodeUTF8(aValue));
            }

            if ((pos >= paramString.length()) || (c != ','))
                break;

        }

        return params;
    }

    private static UnoUrlPart parseUnoUrlPart(String thePart)
        throws com.sun.star.lang.IllegalArgumentException {
        String partName = thePart;
        String theParamPart = "";
        int index = thePart.indexOf(",");
        if (index != -1) {
            partName = thePart.substring(0, index).trim();
            theParamPart = thePart.substring(index + 1).trim();
        }

        if (!isAlphaNumeric(partName)) {
            throw new com.sun.star.lang.IllegalArgumentException(
                "The part name '"
                    + partName
                    + "' may only consist of alpha numeric ASCII characters.");
        }

        HashMap<String,String> params = buildParamHashMap(theParamPart);

        return new UnoUrlPart(theParamPart, partName, params);
    }

    private static boolean isAlphaNumeric(String s) {
        return isValidString(s, null);
    }

    private static boolean isValidString(String identifier, String validCharSet) {

        int len = identifier.length();

        for (int i = 0; i < len; i++) {

            int ch = identifier.charAt(i);

            boolean isValidChar =
                ('A' <= ch && ch <= 'Z')
                    || ('a' <= ch && ch <= 'z')
                    || ('0' <= ch && ch <= '9');

            if (!isValidChar && (validCharSet != null)) {
                isValidChar = (validCharSet.indexOf(ch) != -1);
            }

            if (!isValidChar)
                return false;
        }

        return true;
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

        String url = unoUrl;

        int index = url.indexOf(':');
        if (index != -1) {
            String unoStr = url.substring(0, index).trim();
            if (!"uno".equals(unoStr)) {
                throw new com.sun.star.lang.IllegalArgumentException(
                    "Uno Urls must start with 'uno:'. " + FORMAT_ERROR);
            }
        }

        url = url.substring(index + 1).trim();

        index = url.indexOf(';');
        if (index == -1) {
            throw new com.sun.star.lang.IllegalArgumentException("'"+unoUrl+"' is an invalid Uno Url. " + FORMAT_ERROR);
        }

        String connection = url.substring(0, index).trim();
        url = url.substring(index + 1).trim();

        UnoUrlPart connectionPart = parseUnoUrlPart(connection);

        index = url.indexOf(';');
        if (index == -1) {
            throw new com.sun.star.lang.IllegalArgumentException("'"+unoUrl+"' is an invalid Uno Url. " + FORMAT_ERROR);
        }

        String protocol = url.substring(0, index).trim();
        url = url.substring(index + 1).trim();

        UnoUrlPart protocolPart = parseUnoUrlPart(protocol);

        String rootOid = url.trim();
        if (!isValidString(rootOid, OID_CHAR_SET)) {
            throw new com.sun.star.lang.IllegalArgumentException(
                "Root OID '"+ rootOid + "' contains illegal characters.");
        }

        return new UnoUrl(connectionPart, protocolPart, rootOid);

    }

}
