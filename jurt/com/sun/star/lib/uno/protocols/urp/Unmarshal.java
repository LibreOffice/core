/*************************************************************************
 *
 *  $RCSfile: Unmarshal.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kr $ $Date: 2000-09-28 16:53:16 $
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
package com.sun.star.lib.uno.protocols.urp;


import java.io.ByteArrayInputStream;
import java.io.DataInput;
import java.io.DataInputStream;
import java.io.InputStream;

import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;


import com.sun.star.bridge.XInstanceProvider;

import com.sun.star.uno.Any;
import com.sun.star.uno.Enum;
import com.sun.star.uno.IBridge;
import com.sun.star.uno.IQueryInterface;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.Union;
import com.sun.star.uno.XInterface;

import com.sun.star.lib.uno.environments.remote.IUnmarshal;
import com.sun.star.lib.uno.environments.remote.Protocol;
import com.sun.star.lib.uno.environments.remote.ThreadID;

import com.sun.star.lib.uno.typeinfo.MemberTypeInfo;

class Unmarshal implements IUnmarshal {
    /**
     * When set to true, enables various debugging output.
     */
    static public final boolean DEBUG = false;

    private InputStream _inputStream;
    private DataInput   _dataInput;
    private IBridge     _iBridge;
    private Object      _objectCache[];
    private Type        _typeCache[];
    private ThreadID    _threadIdCache[];

    Unmarshal(IBridge iBridge, short cacheSize) {
        _iBridge         = iBridge;

        _objectCache   = new Object[cacheSize];
        _typeCache     = new Type[cacheSize];
        _threadIdCache = new ThreadID[cacheSize];
        _inputStream   = new ByteArrayInputStream(new byte[0]);
        _dataInput     = new DataInputStream(_inputStream);
    }

    Object readAny() throws Exception {
        Type type = readType();
        Object object = readObject(type.getDescription());

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readAny:" + object);

        return object;
    }

    Boolean readBoolean() throws Exception {
        Boolean result =  new Boolean(_dataInput.readBoolean());

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readBoolean:" + result);

        return result;
    }

    Byte readByte() throws Exception {
        Byte result = new Byte(_dataInput.readByte());

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readByte:" + result);

        return result;
    }

    byte readbyte() throws Exception {
          byte result = (byte)(_dataInput.readByte() & 0xff);

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readbyte:" + (result & 0xff));

        return result;
    }

    byte []readbyteSequence() throws Exception {
        int size = readCompressedInt();
        byte bytes[] = new byte[size];

        _inputStream.read(bytes);

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readbyteSequence:" + bytes);

        return bytes;
    }

    Character readCharacter() throws Exception {
        Character result = new Character(_dataInput.readChar());

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readChar:" + result);

        return result;
    }

    Double readDouble() throws Exception {
        Double result = new Double(_dataInput.readDouble());

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readDouble:" + result);

        return result;
    }

    Enum readEnum(Class zClass) throws Exception {
        Integer index = readInteger();

        Method fromInt = zClass.getMethod("fromInt", new Class[] {int.class});
        Enum result = (Enum)fromInt.invoke(null, new Object[]{index});

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readEnum:" + result);

        return result;
    }

    Throwable readThrowable(Class zClass) throws Exception {
        String message = readString();

        Constructor constructor = zClass.getConstructor(new Class[]{String.class});
        Throwable throwable = (Throwable)constructor.newInstance(new Object[]{message});

        if(java.lang.Exception.class.isAssignableFrom(zClass))
            readStruct(zClass, throwable);

        else if(java.lang.RuntimeException.class.isAssignableFrom(zClass))
            readStruct(zClass, throwable);

        else
            throw new Exception("urp.Unmarshal.readThrowable - unsupported throwable:" + zClass);

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readThrowable:" + throwable);

        return throwable;
    }

    Float readFloat() throws Exception {
        Float result = new Float(_dataInput.readFloat());

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readFloat:" + result);

        return result;
    }

    Integer readInteger() throws Exception {
        Integer result = new Integer(_dataInput.readInt());

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readInteger:" + result);

        return result;
    }

    Long readLong() throws Exception {
        Long result = new Long(_dataInput.readLong());

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readLong:" + result);

        return result;
    }

    public Object readObject(Class zClass) throws Exception {
        Object result = null;

        if(zClass == Any.class || zClass == Object.class) // read an any?
            result = readAny();

        else if(zClass.isArray() && zClass.getComponentType() == byte.class) // read a sequence ?
            result = readbyteSequence();

        else if(zClass.isArray()) // read a sequence ?
            result = readSequence(zClass);

        else if(zClass == Void.class || zClass == void.class) // read nothing ?
            ; // nop

        else if(Enum.class.isAssignableFrom(zClass)) // read an enum ?
            result = readEnum(zClass);

        else if(Union.class.isAssignableFrom(zClass)) // read a union ?
            result = readUnion(zClass);

        else if(zClass == Type.class) // read a type ?
            result = readType();

        else if(XInterface.class.isAssignableFrom(zClass)) // read an interface ?
            result = readReference(zClass);

        else if(zClass == ThreadID.class) // read a thread id ?
            result = readThreadID();

        else if(zClass == boolean.class || zClass == Boolean.class)  // is it a boolean
            result = readBoolean();

        else if(zClass == char.class || zClass == Character.class) // is it a character ?)
            result = readCharacter();

        else if(zClass == byte.class || zClass == Byte.class) // is it a byte ?
            result = readByte();

        else if(zClass == short.class || zClass == Short.class) // is it a short ?
            result = readShort();

        else if(zClass == int.class || zClass == Integer.class) // is it an integer ?
            result = readInteger();

        else if(zClass == long.class || zClass == Long.class) // is it a long ?
            result = readLong();

        else if(zClass == float.class || zClass == Float.class) // is it a float ?
            result = readFloat();

        else if(zClass == double.class || zClass == Double.class) // is it a double ?
            result = readDouble();

        else if(zClass == String.class) // is it a String ?
            result = readString();

        else if(Throwable.class.isAssignableFrom(zClass)) // is it an exception?
            result = readThrowable(zClass);

        else // otherwise read a struct
            result = readStruct(zClass);

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readObject:" + zClass + " " + result);

        return result;
    }

    String readOid() throws Exception {
        Marshal.M_InterfaceReference m_InterfaceReference = (Marshal.M_InterfaceReference)readObject(Marshal.M_InterfaceReference.class);

        String oid = null;

        if(m_InterfaceReference.cache != (short)0xffff) { // is the cache entry valid ?
            if(m_InterfaceReference.full.length() > 0)  // update the cache?
                _objectCache[m_InterfaceReference.cache] = m_InterfaceReference.full;

            oid = (String)_objectCache[m_InterfaceReference.cache];
        }
        else if(m_InterfaceReference.full.length() > 0) // is the oid entry valid ?
            oid = m_InterfaceReference.full;

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readOid:" + oid);

        return oid;
    }

    Object readReference(Class zInterface) throws Exception {
        Object oid = readOid();;

        // the result is a null ref, in case cache and oid are invalid
        Object result = null;

        // map the object from universe
        if(oid != null)
            result = _iBridge.mapInterfaceFrom(oid, zInterface);

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readReference:" + zInterface + " " + result);

        return result;
    }

    Object readSequence(Class zClass) throws Exception {
        int size = readCompressedInt();
        Object result = null;

        zClass = zClass.getComponentType();

        if(zClass == Any.class) // take special care of any array (cause anys are mapped to objects)
            result = Array.newInstance(Object.class, size);
        else
            result = Array.newInstance(zClass, size);

        for(int i = 0; i < size; ++ i)
            Array.set(result, i, readObject(zClass));

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readSequence:" + result);

        return result;
    }

    Short readShort() throws Exception {
        Short result = new Short(_dataInput.readShort());

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readShort:" + result);

        return result;
    }

    short readshort() throws Exception {
        short result = _dataInput.readShort();

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readshort:" + result);

        return result;
    }

    int readCompressedInt() throws Exception {
        int result = _dataInput.readByte() & 0xff;

        if(result == 255) // if 255 then there follows a complete int
            result = _dataInput.readInt();

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readCompressedInt:" + result);

        return result;
    }

    String readString() throws Exception {
        int utflen = readCompressedInt(); // the size of the string

        byte bytes[] = new byte[utflen];
        _inputStream.read(bytes);

        return new String(bytes, "UTF8");
    }

    void readStruct(Class zClass, Object object) throws Exception {
        Field fields[] = zClass.getFields();

        for(int i = 0; i < fields.length; ++ i) {
            if((fields[i].getModifiers() & (Modifier.STATIC | Modifier.TRANSIENT)) == 0) { // neither static nor transient ?
                MemberTypeInfo memberTypeInfo = Protocol.__findMemberTypeInfo(zClass, fields[i].getName());

                // default the member type to the declared type
                Class zInterface = fields[i].getType();

                if(memberTypeInfo != null) {
                    if(memberTypeInfo.isAny()) // is the member an any?
                        if(zInterface.isArray())
                            zInterface = Class.forName("[Lcom.sun.star.uno.Any;");
                        else
                            zInterface = Any.class;

                    else if(memberTypeInfo.isInterface()) { // is the member an interface ?
                        Class xInterface = zInterface;

                        if(!XInterface.class.isAssignableFrom(fields[i].getType())) // is the member type not derived of XInterface ?
                            xInterface = XInterface.class; // ensure that we get at least an XInterface

                        if(zInterface.isArray())
                            zInterface = Class.forName("[L" + xInterface.getName() + ";");
                        else
                            zInterface = xInterface;
                    }
                }

                fields[i].set(object, readObject(zInterface));
            }
        }

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readStruct:" + object);
    }

    Object readStruct(Class zClass) throws Exception {
        Object object = zClass.newInstance();

        readStruct(zClass, object);

        return object;
    }

    ThreadID readThreadID() throws Exception {
        Marshal.M_ThreadId m_threadId = (Marshal.M_ThreadId)readObject(Marshal.M_ThreadId.class);

        ThreadID threadId = null;

        if(m_threadId.cache != (short)0xffff) { // is the cache entry valid?
            if(m_threadId.full.length != 0)
                _threadIdCache[m_threadId.cache] = new ThreadID(m_threadId.full);

            threadId = _threadIdCache[m_threadId.cache];
        }
        else if(m_threadId.full.length != 0)
            threadId = new ThreadID(m_threadId.full);

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readThreadID:" + _threadIdCache[m_threadId.cache]);

        return threadId;
    }

    Type readType() throws Exception {
        int typeClassValue = _dataInput.readUnsignedByte() & 0xff;

        TypeClass typeClass = TypeClass.fromInt(typeClassValue & 0x7f);
        Type type = null;

        if(Type.isTypeClassSimple(typeClass)) // is it a simple type?
            type = new Type(typeClass);

        else {
            short index = _dataInput.readShort(); // the cache index

            if(index != (short)0xffff) { // shall we update the cache?
                if((typeClassValue & 0x80) != 0) // update the cache?
                    _typeCache[index] = new Type(typeClass, readString());

                type = _typeCache[index];
            }
            else
                type = new Type(typeClass, readString());
        }

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readType:" + type);

        return type;
    }

    Union readUnion(Class zClass) throws Exception {
        throw new Exception("Unmarshal.readUnion - not implemented!!!");
    }

    void reset(byte bytes[]) {
        _inputStream = new ByteArrayInputStream(bytes);
        _dataInput = new DataInputStream(_inputStream);
    }

    int bytesLeft() throws Exception {
        return _inputStream.available();
    }
}
