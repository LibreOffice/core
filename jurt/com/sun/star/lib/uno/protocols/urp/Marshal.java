/*************************************************************************
 *
 *  $RCSfile: Marshal.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kr $ $Date: 2000-09-28 11:43:18 $
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
import java.util.Hashtable;
import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.MappingException;


import java.io.ByteArrayOutputStream;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.IOException;

import java.lang.reflect.Array;
import java.lang.reflect.Field;
import java.lang.reflect.Modifier;


import com.sun.star.uno.Any;
import com.sun.star.uno.Enum;
import com.sun.star.uno.IBridge;
import com.sun.star.uno.IQueryInterface;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.Union;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import com.sun.star.lib.uno.environments.remote.IMarshal;
import com.sun.star.lib.uno.environments.remote.Protocol;
import com.sun.star.lib.uno.environments.remote.ThreadID;

import com.sun.star.lib.uno.typeinfo.MemberTypeInfo;

class Marshal implements IMarshal {
    /**
     * When set to true, enables various debugging output.
     */
    static public final boolean DEBUG = false;

    static class M_ThreadId {
        public byte  full[];
        public short cache;

        M_ThreadId() {}

        M_ThreadId(byte full[], short cache) {
            this.full  = full;
            this.cache = cache;
        }
    }

    static class M_InterfaceReference {
        public String full;
        public short cache;

        M_InterfaceReference() {}

          M_InterfaceReference(String full, short cache) {
            this.full  = full;
            this.cache = cache;
        }
    }


    private ByteArrayOutputStream _byteArrayOutputStream;
    private DataOutput            _dataOutput;
    private IBridge               _iBridge;
    private Cache                 _objectCache;
    private Cache                 _typeCache;
    private Cache                 _threadIdCache;

    Marshal(IBridge iBridge, short cacheSize) {
        _iBridge = iBridge;

        _byteArrayOutputStream = new ByteArrayOutputStream();
        _dataOutput = new DataOutputStream(_byteArrayOutputStream);

        _objectCache   = new Cache(cacheSize);
        _typeCache     = new Cache(cacheSize);
        _threadIdCache = new Cache(cacheSize);
    }

    void writeAny(Object object) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeAny:" + object);

        Type type = null;

        if(object == null)
            type = new Type(Void.class);

        else if(object instanceof Any) {
            Any any = (Any)object;

            type = new Type(any.getInterface());
            object = any.getObject();
        }
        else if(object instanceof XInterface)
            type = new Type(XInterface.class);

        else
            type = new Type(object.getClass());

        writeType(type);
        writeObject(type.getDescription(), object);
    }

    void writeBoolean(Boolean zBoolean) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeBoolean:" + zBoolean);

        _dataOutput.writeBoolean(zBoolean.booleanValue());
    }

    void writebyte(byte zbyte) throws Exception {
        int ibyte = zbyte & 0xff;

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writebyte:" + ibyte);

        _dataOutput.writeByte(ibyte);
    }

    void writebyteSequence(byte bytes[]) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writebyteSequence:" + bytes);

        writeCompressedInt(bytes.length);

        _dataOutput.write(bytes);
    }

    void writeByte(Byte zByte) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeByte:" + zByte);

        _dataOutput.writeByte(zByte.byteValue());
    }

    void writeCharacter(Character character) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeCharacter:" + character);

        _dataOutput.writeChar(character.charValue());
    }

    void writeDouble(Double zDouble) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeDouble:" + zDouble);

        _dataOutput.writeDouble(zDouble.doubleValue());
    }

    void writeEnum(Enum enum) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeEnum:" + enum + " " + enum.getValue());

        writeCompressedInt(enum.getValue());
    }

    void writeThrowable(Class zClass, Throwable throwable) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeThrowable:" + throwable);

        String message = throwable.getMessage();
        writeString((message == null) ? "" : message);

        if(java.lang.Exception.class.isAssignableFrom(zClass))
            writeStruct(zClass, throwable);

        else if(java.lang.RuntimeException.class.isAssignableFrom(zClass))
            writeStruct(zClass, throwable);

        else
            throw new Exception("urp.Marshal.writeThrowable - unsupported throwable:" + zClass);
    }

    void writeFloat(Float zFloat) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeFloat:" + zFloat);

        _dataOutput.writeFloat(zFloat.floatValue());
    }

    void writeInteger(Integer integer) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeInteger:" + integer);

        _dataOutput.writeInt(integer.intValue());
    }

    void writeint(int zint) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeint:" + zint);

        _dataOutput.writeInt(zint);
    }

    void writeLong(Long zLong) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeLong:" + zLong);

        _dataOutput.writeLong(zLong.longValue());
    }

    // we may optimize this method with a hashtable in the future
    public void writeObject(Class zClass, Object object) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeObject: <" + zClass + "> <" + object + ">");

        if(zClass == Any.class || zClass == Object.class) // write an any ?
            writeAny(object);

        else if(zClass.isArray() & zClass.getComponentType() == byte.class) // write a sequence ?
            writebyteSequence((byte [])object);

        else if(zClass.isArray()) // write a sequence ?
            writeSequence(zClass, object);

        else if(zClass == Void.class || zClass == void.class) // write nothing ?
            ; // nop

        else if(Enum.class.isAssignableFrom(zClass)) // write an enum ?
            writeEnum((Enum)object);

        else if(Union.class.isAssignableFrom(zClass)) // write a union ?
            writeUnion((Union)object);

        else if(Type.class.isAssignableFrom(zClass)) // write a type ?
            writeType((Type)object);

        else if(XInterface.class.isAssignableFrom(zClass)) // is it an interface ?
            writeReference(zClass, object);

        else if(object instanceof ThreadID) // is it a thread id ?
            writeThreadID((ThreadID)object);

        else if(zClass == boolean.class || zClass == Boolean.class)  // is it a boolean
            writeBoolean((Boolean)object);

        else if(zClass == char.class || zClass == Character.class) // is it a character ?)
            writeCharacter((Character)object);

        else if(zClass == byte.class || zClass == Byte.class) // is it a byte ?
            writeByte((Byte)object);

        else if(zClass == short.class || zClass == Short.class) // is it a short ?
            writeShort((Short)object);

        else if(zClass == int.class || zClass == Integer.class) // is it an integer ?
            writeInteger((Integer)object);

        else if(zClass == long.class || zClass == Long.class) // is it a long ?
            writeLong((Long)object);

        else if(zClass == float.class || zClass == Float.class) // is it a float ?
            writeFloat((Float)object);

        else if(zClass == double.class || zClass == Double.class) // is it a double ?
            writeDouble((Double)object);

        else if(zClass == String.class) // is it a String ?
            writeString((String)object);

        else if(Throwable.class.isAssignableFrom(zClass)) // is it an exception?
            writeThrowable(zClass, (Throwable)object);

        else // otherwise it must be a struct
            writeStruct(zClass, object);

    }

    static private final M_InterfaceReference null_M_InterfaceReference = new M_InterfaceReference("", (short)0xffff);

    void writeOid(String oid) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeOid:" + oid);

        M_InterfaceReference m_InterfaceReference = null;

        if(oid == null)
            m_InterfaceReference = null_M_InterfaceReference;
        else {
            boolean found[] = new boolean[1];
            short index = _objectCache.add(found, oid);

            m_InterfaceReference = new M_InterfaceReference(found[0] ? "" : oid, index);
        }

        writeObject(M_InterfaceReference.class, m_InterfaceReference);
    }

    void writeReference(Class zInterface, Object object) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeReference:" + zInterface + " " + object);

        // map the object to universe
        writeOid(object != null ? (String)_iBridge.mapInterfaceTo(object, zInterface) : null);
    }

    void writeSequence(Class zClass, Object object) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeSequence:" + zClass + " " + object);

        int size = Array.getLength(object);
        writeCompressedInt(size);

        zClass = zClass.getComponentType();
        for(int i = 0; i < size; ++ i)
            writeObject(zClass, Array.get(object, i));
    }

    void writeShort(Short zShort) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeShort:" + zShort);

        _dataOutput.writeShort(zShort.shortValue());
    }

    void writeshort(short zshort) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeshort:" + zshort);

        _dataOutput.writeShort(zshort);
    }

    void writeCompressedInt(int size) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeCompressedInt:" + size);

        if(size >= 255) {
            _dataOutput.writeByte((byte)0xff);
            _dataOutput.writeInt(size);
        }
        else
            _dataOutput.writeByte((byte)size);
    }

    void writeString(String string) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeString:" + string);

        byte bytes[] = string.getBytes("UTF8");

        writeCompressedInt(bytes.length);
        _dataOutput.write(bytes);
    }

    void writeStruct(Class zClass, Object object) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeStruct:" + zClass + " " + object);

        Field fields[] = zClass.getFields();
        for(int i = 0; i < fields.length; ++ i) {
            if((fields[i].getModifiers() & (Modifier.STATIC | Modifier.TRANSIENT)) == 0) { // neither static nor transient ?
                MemberTypeInfo memberTypeInfo = Protocol.__findMemberTypeInfo(zClass, fields[i].getName());

                // default the member type to the declared type
                Class zInterface = fields[i].getType();

                if(memberTypeInfo != null) { // do we have any type infos?
                    if(memberTypeInfo.isAny()) // is the member any any?
                        if(zInterface.isArray())
                            zInterface = Class.forName("[Lcom.sun.star.uno.Any;");
                        else
                            zInterface = Any.class;

                    else if(memberTypeInfo.isInterface()) { // is the member an interface ?
                        fields[i].getType().getFields();

                        Class xInterface = zInterface;

                        if(!XInterface.class.isAssignableFrom(fields[i].getType())) // is the member type not derived of XInterface ?
                            xInterface = XInterface.class; // ensure that we write at least an XInterface

                        if(zInterface.isArray())
                            zInterface = Class.forName("[L" + xInterface.getName() + ";");
                        else
                            zInterface = xInterface;
                    }
                }

                writeObject(zInterface, fields[i].get(object));
            }
        }
    }

    void writeThreadID(ThreadID threadID) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeThreadID:" + threadID);

        boolean found[] = new boolean[1];
        short index = _threadIdCache.add(found, threadID.getBytes());

        M_ThreadId m_ThreadId = new M_ThreadId(found[0] ? null : threadID.getBytes(), index);

        writeObject(M_ThreadId.class, m_ThreadId);
    }

    void writeType(Type type) throws Exception {
        TypeClass typeClass = type.getTypeClass();

        if(Type.isTypeClassSimple(typeClass))
            _dataOutput.writeByte((byte)typeClass.getValue()); // write the typeclass value
        else {
            boolean found[] = new boolean[1];
            short index = _typeCache.add(found, type.getTypeName());

            _dataOutput.writeByte((byte)(typeClass.getValue() | (found[0] ? 0x0 : 0x80))); // write the typeclass value

            _dataOutput.writeShort(index); // write the cache index

            if(!found[0]) // if not found in cache and the type is complex, write the type name
                writeString(type.getTypeName());
        }
    }

    void writeUnion(Union union) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeUnion:" + union);

        throw new Exception("Marshal.writeUnion is not implemented yet!!!");
    }

    public byte []reset() {
        byte result[] = _byteArrayOutputStream.toByteArray();
        _byteArrayOutputStream.reset();

        return result;
    }
}

