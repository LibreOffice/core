/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: RegistryKey.java,v $
 * $Revision: 1.3 $
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

package com.sun.star.tools.uno;


import java.util.Enumeration;
import java.util.Hashtable;


import com.sun.star.registry.InvalidRegistryException;
import com.sun.star.registry.InvalidValueException;
import com.sun.star.registry.RegistryKeyType;
import com.sun.star.registry.RegistryValueType;
import com.sun.star.registry.XRegistryKey;


/**
 * This is a dummy registry implementation,
 * which only implmenets the needed methods.
 */
class RegistryKey implements XRegistryKey {
    protected RegistryValueType _registryValueType = RegistryValueType.NOT_DEFINED;

    protected String    _name;
    protected Hashtable _keys = new Hashtable();

    protected int    _long;
    protected int    _long_list[];
    protected String _ascii;
    protected String _ascii_list[];
    protected String _string;
    protected String _string_list[];
    protected byte   _binary[];

    public RegistryKey(String name) {
        _name = name;
    }


    // XRegistryKey Attributes
    public String getKeyName() throws com.sun.star.uno.RuntimeException {
        return _name;
    }

    // XRegistryKey Methods
    public boolean isReadOnly() throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        return false;
    }

    public boolean isValid() throws com.sun.star.uno.RuntimeException {
        return true;
    }

    public RegistryKeyType getKeyType( /*IN*/String rKeyName ) throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        return RegistryKeyType.KEY;
    }

    public RegistryValueType getValueType() throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        return _registryValueType;
    }

    public int getLongValue() throws InvalidRegistryException, InvalidValueException, com.sun.star.uno.RuntimeException {
        if(_registryValueType != RegistryValueType.LONG)
            throw new InvalidValueException("long");

        return _long;
    }

    public void setLongValue( /*IN*/int value ) throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        _registryValueType = RegistryValueType.LONG;

        _long = value;
    }

    public int[] getLongListValue() throws InvalidRegistryException, InvalidValueException, com.sun.star.uno.RuntimeException {
        if(_registryValueType != RegistryValueType.LONGLIST)
            throw new InvalidValueException("longlist");

        return _long_list;
    }

    public void setLongListValue( /*IN*/int[] seqValue ) throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        _registryValueType = RegistryValueType.LONGLIST;

        _long_list = seqValue;
    }

    public String getAsciiValue() throws InvalidRegistryException, InvalidValueException, com.sun.star.uno.RuntimeException {
        if(_registryValueType != RegistryValueType.ASCII)
            throw new InvalidValueException("ascii");

        return _ascii;
    }

    public void setAsciiValue( /*IN*/String value ) throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        _registryValueType = RegistryValueType.ASCII;

        _ascii = value;
    }

    public String[] getAsciiListValue() throws InvalidRegistryException, InvalidValueException, com.sun.star.uno.RuntimeException  {
        if(_registryValueType != RegistryValueType.ASCIILIST)
            throw new InvalidValueException("asciilist");

        return _ascii_list;
    }

    public void setAsciiListValue( /*IN*/String[] seqValue ) throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        _registryValueType = RegistryValueType.ASCIILIST;

        _ascii_list = seqValue;
    }

    public String getStringValue() throws InvalidRegistryException, InvalidValueException, com.sun.star.uno.RuntimeException  {
        if(_registryValueType != RegistryValueType.STRING)
            throw new InvalidValueException("string");

        return _string;
    }

    public void setStringValue( /*IN*/String value ) throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        _registryValueType = RegistryValueType.STRING;

        _string = value;
    }

    public String[] getStringListValue() throws InvalidRegistryException, InvalidValueException, com.sun.star.uno.RuntimeException {
        if(_registryValueType != RegistryValueType.STRINGLIST)
            throw new InvalidValueException("string_list");

        return _string_list;
    }

    public void setStringListValue( /*IN*/String[] seqValue ) throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        _registryValueType = RegistryValueType.STRINGLIST;

        _string_list = seqValue;
    }

    public byte[] getBinaryValue() throws InvalidRegistryException, InvalidValueException, com.sun.star.uno.RuntimeException {
        if(_registryValueType != RegistryValueType.BINARY)
            throw new InvalidValueException("longlist");

        return _binary;
    }

    public void setBinaryValue( /*IN*/byte[] value ) throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        _registryValueType = RegistryValueType.BINARY;

        _binary = value;
    }

    public XRegistryKey openKey( /*IN*/String aKeyName ) throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        return (XRegistryKey)_keys.get(aKeyName);
    }

    public XRegistryKey createKey( /*IN*/String aKeyName ) throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        XRegistryKey xRegistryKey = openKey(aKeyName);

        if(xRegistryKey == null) {
            xRegistryKey = new RegistryKey(aKeyName);
            _keys.put(aKeyName, xRegistryKey);
        }

        return xRegistryKey;
    }

    public void closeKey() throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
    }

    public void deleteKey( /*IN*/String rKeyName ) throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        _keys.remove(rKeyName);
    }

    public synchronized XRegistryKey[] openKeys() throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        RegistryKey registryKeys[] = new RegistryKey[_keys.size()];

        Enumeration elements = _keys.elements();
        int i = 0;
        while(elements.hasMoreElements()) {
            registryKeys[i ++] = (RegistryKey)elements.nextElement();
        }

        return registryKeys;
    }

    public synchronized String[] getKeyNames() throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        String strings[] = new String[_keys.size()];

        Enumeration elements = _keys.keys();
        int i = 0;
        while(elements.hasMoreElements()) {
            strings[i ++] = (String)elements.nextElement();
        }

        return strings;
    }

    public boolean createLink( /*IN*/String aLinkName, /*IN*/String aLinkTarget ) throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        return false;
    }

    public void deleteLink( /*IN*/String rLinkName ) throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
    }

    public String getLinkTarget( /*IN*/String rLinkName ) throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        return null;
    }

    public String getResolvedName( /*IN*/String aKeyName ) throws InvalidRegistryException, com.sun.star.uno.RuntimeException {
        return null;
    }
}
