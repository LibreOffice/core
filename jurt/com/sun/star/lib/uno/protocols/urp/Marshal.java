/*************************************************************************
 *
 *  $RCSfile: Marshal.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kr $ $Date: 2001-03-06 17:09:08 $
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

import com.sun.star.lib.uno.typedesc.TypeDescription;

import com.sun.star.lib.uno.typeinfo.MemberTypeInfo;

class Marshal implements IMarshal {
    /**
     * When set to true, enables various debugging output.
     */
    static public final boolean DEBUG = false;

    static public final TypeDescription __xInterfaceTypeDescription           = TypeDescription.getTypeDescription(XInterface.class);
    static public final TypeDescription __M_InterfaceReferenceTypeDescription = TypeDescription.getTypeDescription(M_InterfaceReference.class);
    static public final TypeDescription __M_ThreadIdTypeDescription           = TypeDescription.getTypeDescription(M_ThreadId.class);

    static private final M_InterfaceReference __null_M_InterfaceReference = new M_InterfaceReference("", (short)0xffff);



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
    private boolean               _useCaches = true;

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

        TypeDescription typeDescription = null;

        if(object == null)
            typeDescription = TypeDescription.__void_TypeDescription;

        else if(object instanceof Any) {
            Any any = (Any)object;

            typeDescription = (TypeDescription)any.getType().getTypeDescription();
            object = any.getObject();
        }
        else if(object instanceof XInterface)
            typeDescription = __xInterfaceTypeDescription;

        else
            typeDescription = TypeDescription.getTypeDescription(object.getClass());

        writeTypeDescrption(typeDescription);
        writeObject(typeDescription, object);
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

        int size = 0;
        if(bytes == null)
            System.err.println("WARNING! writing null sequence as empty sequence");

        else
            size = bytes.length;

        writeCompressedInt(size);

        if(size != 0)
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

        writeint(enum.getValue());
    }

    void writeThrowable(TypeDescription typeDescription, Throwable throwable) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeThrowable:" + throwable);

        String message = throwable.getMessage();
        writeString((message == null) ? "" : message);

        writeStruct(typeDescription, throwable);
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

    public void writeObject(TypeDescription typeDescription, Object object) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeObject: <" + typeDescription + "> <" + object + ">");

        switch(typeDescription.getTypeClass().getValue()) {
        case TypeClass.ANY_value:       writeAny(object);                                     break; // write an any ?
        case TypeClass.SEQUENCE_value:
        case TypeClass.ARRAY_value:     writeSequence(typeDescription, object);               break; // write a sequence ?
        case TypeClass.VOID_value:                                                            break; // write nothing ?
        case TypeClass.ENUM_value:      writeEnum((Enum)object);                              break; // write an enum ?
        case TypeClass.UNION_value:     writeUnion((Union)object);                            break; // write a union ?
        case TypeClass.TYPE_value:      writeTypeDescrption((TypeDescription)((Type)object).getTypeDescription()); break; // write a type ?
        case TypeClass.INTERFACE_value: writeReference(typeDescription, object);              break; // is it an interface ?
        case TypeClass.BOOLEAN_value:   writeBoolean((Boolean)object);                        break; // is it a boolean
        case TypeClass.CHAR_value:      writeCharacter((Character)object);                    break; // is it a character ?
        case TypeClass.BYTE_value:      writeByte((Byte)object);                              break; // is it a byte ?
        case TypeClass.SHORT_value:     writeShort((Short)object);                            break; // is it a short ?
        case TypeClass.LONG_value:      writeInteger((Integer)object);                        break; // is it an integer ?
        case TypeClass.HYPER_value:     writeLong((Long)object);                              break; // is it a long ?
        case TypeClass.FLOAT_value:     writeFloat((Float)object);                            break; // is it a float ?
        case TypeClass.DOUBLE_value:    writeDouble((Double)object);                          break; // is it a double ?
        case TypeClass.STRING_value:    writeString((String)object);                          break; // is it a String ?
        case TypeClass.STRUCT_value:
            if(object instanceof ThreadID) // is it a thread id ?
                writeThreadID((ThreadID)object);
            else // is it a struct ?
                writeStruct(typeDescription, object);

            break;
        case TypeClass.EXCEPTION_value: writeThrowable(typeDescription, (Throwable)object);   break; // is it an exception?

        default:
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".writeObject - unknown typeClass:" + typeDescription.getTypeClass().getValue());
        }
    }

    void writeOid(String oid) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeOid:" + oid);

        M_InterfaceReference m_InterfaceReference = null;

        if(oid == null)
            m_InterfaceReference = __null_M_InterfaceReference;
        else {
            boolean found[] = new boolean[1];
            short index;
            if(_useCaches)
                index = _objectCache.add(found, oid);
            else
                index = (short)0xffff;

            m_InterfaceReference = new M_InterfaceReference(found[0] ? "" : oid, index);
        }

        writeObject(__M_InterfaceReferenceTypeDescription, m_InterfaceReference);
    }

    void writeReference(TypeDescription typeDescription, Object object) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeReference:" + typeDescription + " " + object);

        // map the object to universe
        writeOid(object != null ? (String)_iBridge.mapInterfaceTo(object, new Type(typeDescription)) : null);
    }

    void writeSequence(TypeDescription typeDescription, Object object) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeSequence:" + typeDescription + " " + object);

        if(typeDescription.getTypeClass() == TypeClass.BYTE) // write a byte sequence ?
            writebyteSequence((byte [])object);

        else {
            int size = 0;
            if(object == null)
                System.err.println("WARNING! writing null sequence as empty sequence");
            else
                size = Array.getLength(object);

            writeCompressedInt(size);

            typeDescription = typeDescription.getComponentType();
            for(int i = 0; i < size; ++ i)
                writeObject(typeDescription, Array.get(object, i));
        }
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

    void writeStruct(TypeDescription typeDescription, Object object) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeStruct:" + typeDescription + " " + object);

        Field fields[] = typeDescription.getFields();
        for(int i = 0; i < fields.length; ++ i) {
            if((fields[i].getModifiers() & (Modifier.STATIC | Modifier.TRANSIENT)) == 0) { // neither static nor transient ?
                MemberTypeInfo memberTypeInfo = typeDescription.getMemberTypeInfo(fields[i].getName());

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

                writeObject(TypeDescription.getTypeDescription(zInterface), fields[i].get(object));
            }
        }
    }

    void writeThreadID(ThreadID threadID) throws Exception {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeThreadID:" + threadID);

        boolean found[] = new boolean[1];
        short index;
        if(_useCaches)
            index = _threadIdCache.add(found, threadID.getBytes());
        else
            index = (short)0xffff;

        M_ThreadId m_ThreadId = new M_ThreadId(found[0] ? null : threadID.getBytes(), index);

        writeObject(__M_ThreadIdTypeDescription, m_ThreadId);
    }

    void writeTypeDescrption(TypeDescription typeDescription) throws Exception {
        TypeClass typeClass = typeDescription.getTypeClass();

        if(TypeDescription.isTypeClassSimple(typeClass))
            _dataOutput.writeByte((byte)typeClass.getValue()); // write the typeclass value
        else {
            boolean found[] = new boolean[1];
            short index;

            if(_useCaches)
                index = _typeCache.add(found, typeDescription.getTypeName());
            else
                index = (short)0xffff;

            _dataOutput.writeByte((byte)(typeClass.getValue() | (found[0] ? 0x0 : 0x80))); // write the typeclass value

            _dataOutput.writeShort(index); // write the cache index

            if(!found[0]) // if not found in cache and the type is complex, write the type name
                writeString(typeDescription.getTypeName());
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

