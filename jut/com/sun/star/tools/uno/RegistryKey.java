/*************************************************************************
 *
 *  $RCSfile: RegistryKey.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: kr $ $Date: 2000-09-27 09:30:27 $
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
