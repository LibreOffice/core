/*************************************************************************
 *
 *  $RCSfile: Marshal.java,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 12:33:00 $
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


import java.io.ByteArrayOutputStream;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;

import java.lang.reflect.Array;
import java.lang.reflect.Field;
import java.lang.reflect.Modifier;


import com.sun.star.uno.Any;
import com.sun.star.uno.Enum;
import com.sun.star.uno.IBridge;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.Union;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import com.sun.star.lib.uno.environments.remote.IMarshal;
import com.sun.star.lib.uno.environments.remote.Protocol;
import com.sun.star.lib.uno.environments.remote.ThreadId;

import com.sun.star.uno.ITypeDescription;
import com.sun.star.uno.IFieldDescription;

import com.sun.star.lib.uno.typedesc.TypeDescription;

import com.sun.star.lib.uno.typeinfo.MemberTypeInfo;

class Marshal implements IMarshal {
    /**
     * When set to true, enables various debugging output.
     */
    static private final boolean DEBUG = false;

    static public final ITypeDescription __xInterfaceTypeDescription           = TypeDescription.getTypeDescription(XInterface.class);
    static public final ITypeDescription __M_InterfaceReferenceTypeDescription = TypeDescription.getTypeDescription(M_InterfaceReference.class);
    static public final ITypeDescription __M_ThreadIdTypeDescription           = TypeDescription.getTypeDescription(M_ThreadId.class);

    static private final M_InterfaceReference __null_M_InterfaceReference = new M_InterfaceReference("", (short)0xffff);

    static final Class __anyArray;

    static {
        try {
            __anyArray = Class.forName("[Lcom.sun.star.uno.Any;");
        }
        catch(ClassNotFoundException classNotFoundException) {
            throw new com.sun.star.uno.RuntimeException("urp.Marshal.<cinit> - unexpected:" + classNotFoundException);
        }
    }

    static public class M_ThreadId {
        public byte  full[];
        public short cache;

        M_ThreadId() {}

        M_ThreadId(byte full[], short cache) {
            this.full  = full;
            this.cache = cache;
        }
        public static final com.sun.star.lib.uno.typeinfo.TypeInfo UNOTYPEINFO[] = {
            new com.sun.star.lib.uno.typeinfo.MemberTypeInfo( "full", 0, 0 ),
            new com.sun.star.lib.uno.typeinfo.MemberTypeInfo( "cache", 1, 0 ) };
    }

    static public class M_InterfaceReference {
        public String full;
        public short cache;

        M_InterfaceReference() {}

          M_InterfaceReference(String full, short cache) {
            this.full  = full;
            this.cache = cache;
        }
        public static final com.sun.star.lib.uno.typeinfo.TypeInfo UNOTYPEINFO[] = {
            new com.sun.star.lib.uno.typeinfo.MemberTypeInfo( "full", 0, 0 ),
            new com.sun.star.lib.uno.typeinfo.MemberTypeInfo( "cache", 1, 0 ) };
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

    void writeAny(Object object) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeAny:" + object);

        ITypeDescription iTypeDescription = null;

        if(object == null)
            iTypeDescription = __xInterfaceTypeDescription;

        else if(object instanceof Any) {
            Any any = (Any)object;

            try {
                iTypeDescription = TypeDescription.getTypeDescription(any.getType());
            }
            catch(ClassNotFoundException classNotFoundException) {
                throw new com.sun.star.uno.RuntimeException(getClass().getName() + " - unexpected:" + classNotFoundException);
            }

            object = any.getObject();
        }
        else if(object instanceof XInterface)
            iTypeDescription = __xInterfaceTypeDescription;

        else
            iTypeDescription = TypeDescription.getTypeDescription(object.getClass());

        writeTypeDescrption(iTypeDescription);
        writeObject(iTypeDescription, object);
    }

    void writeboolean(boolean bool) {
        try {
            _dataOutput.writeBoolean(bool);
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".writeboolean - unexpected: " + iOException);
        }
    }

    void writeBoolean(Boolean zBoolean) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeBoolean:" + zBoolean);

        writeboolean(zBoolean.booleanValue());
    }

    void writebyte(byte zbyte) {
        int ibyte = zbyte & 0xff;

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writebyte:" + ibyte);

        try {
            _dataOutput.writeByte(ibyte);
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".writebyte - unexpected: " + iOException);
        }
    }

    void write(byte bytes[]) {
        try {
            _dataOutput.write(bytes);
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".write - unexpected: " + iOException);
        }
    }

    void writebyteSequence(byte bytes[]) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writebyteSequence:" + bytes);

        int size = 0;
        if(bytes == null)
            System.err.println("WARNING! writing null sequence as empty sequence");

        else
            size = bytes.length;

        writeCompressedInt(size);

        if(size != 0)
            write(bytes);
    }

    void writeByte(Byte zByte) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeByte:" + zByte);

        writebyte(zByte.byteValue());
    }

    void writechar(char zchar) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writechar:" + zchar);

        try {
            _dataOutput.writeChar(zchar);
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".writechar - unexpected: " + iOException);
        }
    }

    void writeCharacter(Character character) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeCharacter:" + character);

        writechar(character.charValue());
    }

    void writedouble(double zdouble) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writedouble:" + zdouble);

        try {
            _dataOutput.writeDouble(zdouble);
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".writedouble - unexpected: " + iOException);
        }
    }

    void writeDouble(Double zDouble) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeDouble:" + zDouble);

        writedouble(zDouble.doubleValue());
    }

    void writeEnum(Enum enum) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeEnum:" + enum + " " + enum.getValue());

        writeint(enum.getValue());
    }

    void writeThrowable(ITypeDescription iTypeDescription, Throwable throwable) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeThrowable:" + throwable);

        String message = throwable.getMessage();
        writeString((message == null) ? "" : message);

        writeStruct(iTypeDescription, throwable);
    }

    void writefloat(float zfloat) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writefloat:" + zfloat);

        try {
            _dataOutput.writeFloat(zfloat);
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".writefloat - unexpected: " + iOException);
        }
    }

    void writeFloat(Float zFloat) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeFloat:" + zFloat);

        writefloat(zFloat.floatValue());
    }

    void writeInteger(Integer integer) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeInteger:" + integer);

        writeint(integer.intValue());
    }

    void writeint(int zint) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeint:" + zint);

        try {
            _dataOutput.writeInt(zint);
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".writeint - unexpected: " + iOException);
        }
    }

    void writelong(long zlong) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writelong:" + zlong);

        try {
            _dataOutput.writeLong(zlong);
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".writelong - unexpected: " + iOException);
        }
    }

    void writeLong(Long zLong) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeLong:" + zLong);

        writelong(zLong.longValue());
    }

    public void writeObject(ITypeDescription iTypeDescription, Object object) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeObject: <" + iTypeDescription + "> <" + object + ">");

        switch(iTypeDescription.getTypeClass().getValue()) {
        case TypeClass.ANY_value:       writeAny(object);                                     break; // write an any ?
        case TypeClass.SEQUENCE_value:
        case TypeClass.ARRAY_value:     writeSequence(iTypeDescription, object);               break; // write a sequence ?
        case TypeClass.VOID_value:                                                            break; // write nothing ?
        case TypeClass.ENUM_value:      writeEnum((Enum)object);                              break; // write an enum ?
        case TypeClass.UNION_value:     writeUnion((Union)object);                            break; // write a union ?
        case TypeClass.TYPE_value:
            try {
                writeTypeDescrption(TypeDescription.getTypeDescription((Type)object));
            }
            catch(ClassNotFoundException classNotFoundException) {
                throw new com.sun.star.uno.RuntimeException(getClass().getName() + " - unexpected:" + classNotFoundException);
            }
            break; // write a type ?

        case TypeClass.INTERFACE_value: writeReference(iTypeDescription, object);              break; // is it an interface ?
        case TypeClass.BOOLEAN_value:   writeBoolean((Boolean)object);                        break; // is it a boolean
        case TypeClass.CHAR_value:      writeCharacter((Character)object);                    break; // is it a character ?
        case TypeClass.BYTE_value:      writeByte((Byte)object);                              break; // is it a byte ?
        case TypeClass.SHORT_value:
        case TypeClass.UNSIGNED_SHORT_value:
            writeShort((Short) object);
            break;
        case TypeClass.LONG_value:
        case TypeClass.UNSIGNED_LONG_value:
            writeInteger((Integer) object);
            break;
        case TypeClass.HYPER_value:
        case TypeClass.UNSIGNED_HYPER_value:
            writeLong((Long) object);
            break;
        case TypeClass.FLOAT_value:     writeFloat((Float)object);                            break; // is it a float ?
        case TypeClass.DOUBLE_value:    writeDouble((Double)object);                          break; // is it a double ?
        case TypeClass.STRING_value:    writeString((String)object);                          break; // is it a String ?
        case TypeClass.STRUCT_value:
            if(object instanceof ThreadId) // is it a thread id ?
                writeThreadId((ThreadId)object);
            else // is it a struct ?
                writeStruct(iTypeDescription, object);

            break;
        case TypeClass.EXCEPTION_value: writeThrowable(iTypeDescription, (Throwable)object);   break; // is it an exception?

        default:
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".writeObject - unknown typeClass:" + iTypeDescription.getTypeClass().getValue());
        }
    }

    void writeOid(String oid) {
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

    void writeReference(ITypeDescription iTypeDescription, Object object) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeReference:" + iTypeDescription + " " + object);

        // map the object to universe
        writeOid(object != null ? (String)_iBridge.mapInterfaceTo(object, new Type(iTypeDescription)) : null);
    }

    void writeSequence(ITypeDescription iTypeDescription, Object object) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeSequence:" + iTypeDescription + " " + object);

        iTypeDescription = iTypeDescription.getComponentType();
        if(iTypeDescription.getTypeClass() == TypeClass.BYTE) // write a byte sequence ?
            writebyteSequence((byte [])object);

        else {
            int size = 0;
            if(object == null)
                System.err.println("WARNING! writing null sequence as empty sequence");
            else
                size = Array.getLength(object);

            writeCompressedInt(size);

            for(int i = 0; i < size; ++ i)
                writeObject(iTypeDescription, Array.get(object, i));
        }
    }

    void writeShort(Short zShort) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeShort:" + zShort);

        writeshort(zShort.shortValue());
    }

    void writeshort(short zshort) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeshort:" + zshort);

        try {
            _dataOutput.writeShort(zshort);
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".writeshort - unexpected: " + iOException);
        }
    }

    void writeCompressedInt(int size) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeCompressedInt:" + size);

        if(size >= 255) {
            writebyte((byte)0xff);
            writeint(size);
        }
        else
            writebyte((byte)size);
    }

    void writeString(String string) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeString:" + string);

        try {
            byte bytes[] = string.getBytes("UTF8");

            writeCompressedInt(bytes.length);
            write(bytes);
        }
        catch(UnsupportedEncodingException unsupportedEncodingException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".writeString - unexpected:" + unsupportedEncodingException);
        }
    }

    void writeStruct(ITypeDescription iTypeDescription, Object object) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeStruct:" + iTypeDescription + " " + object);

        IFieldDescription iFieldDescriptions[] = iTypeDescription.getFieldDescriptions();
        for(int i = 0; i < iFieldDescriptions.length; ++ i) {
            try {
                writeObject(iFieldDescriptions[i].getTypeDescription(), iFieldDescriptions[i].getField().get(object));
            }
            catch(IllegalAccessException illegalAccessException) {
                throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".writeStruct - unexpected:" + illegalAccessException);
            }
        }
    }

    void writeThreadId(ThreadId threadId) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeThreadID:" + threadId);

        boolean found[] = new boolean[1];
        short index;
        if(_useCaches)
            index = _threadIdCache.add(found, threadId.getBytes());
        else
            index = (short)0xffff;

        M_ThreadId m_ThreadId = new M_ThreadId(found[0] ? null : threadId.getBytes(), index);

        writeObject(__M_ThreadIdTypeDescription, m_ThreadId);
    }

    void writeTypeDescrption(ITypeDescription iTypeDescription) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeTypeDescrption:" + iTypeDescription);

        TypeClass typeClass = iTypeDescription.getTypeClass();

        if(TypeDescription.isTypeClassSimple(typeClass))
            writebyte((byte)typeClass.getValue()); // write the typeclass value

        else {
            boolean found[] = new boolean[1];
            short index;

            if(_useCaches)
                index = _typeCache.add(found, iTypeDescription.getTypeName());
            else
                index = (short)0xffff;

            writebyte((byte)(typeClass.getValue() | (found[0] ? 0x0 : 0x80))); // write the typeclass value

            writeshort(index); // write the cache index

            if(!found[0]) // if not found in cache and the type is complex, write the type name
                writeString(iTypeDescription.getTypeName());
        }
    }

    void writeUnion(Union union) {
        if(DEBUG) System.err.println("##### " + getClass().getName() + ".writeUnion:" + union);

        throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".writeUnion is not implemented yet!!!");
    }

    public byte []reset() {
        byte result[] = _byteArrayOutputStream.toByteArray();
        _byteArrayOutputStream.reset();

        return result;
    }
}

