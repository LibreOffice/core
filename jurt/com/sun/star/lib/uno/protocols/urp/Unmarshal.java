/*************************************************************************
 *
 *  $RCSfile: Unmarshal.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kr $ $Date: 2001-03-14 10:16:07 $
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

import com.sun.star.lib.uno.typedesc.TypeDescription;

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
    private TypeDescription        _typeDescriptionCache[];
    private ThreadID    _threadIdCache[];

    Unmarshal(IBridge iBridge, short cacheSize) {
        _iBridge         = iBridge;

        _objectCache          = new Object[cacheSize];
        _typeDescriptionCache = new TypeDescription[cacheSize];
        _threadIdCache        = new ThreadID[cacheSize];
        _inputStream          = new ByteArrayInputStream(new byte[0]);
        _dataInput            = new DataInputStream(_inputStream);
    }

    Object readAny() throws Exception {
        TypeDescription typeDescription = readTypeDescription();
        Object object = readObject(typeDescription);

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

    Enum readEnum(TypeDescription typeDescription) throws Exception {
        Integer index = readInteger();

        Method fromInt = typeDescription.getZClass().getMethod("fromInt", new Class[] {int.class});
        Enum result = (Enum)fromInt.invoke(null, new Object[]{index});

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readEnum:" + result);

        return result;
    }

    Throwable readThrowable(TypeDescription typeDescription) throws Exception {
        String message = readString();

        Constructor constructor = typeDescription.getZClass().getConstructor(new Class[]{String.class});
        Throwable throwable = (Throwable)constructor.newInstance(new Object[]{message});

        readStruct(typeDescription, throwable);

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

    public Object readObject(TypeDescription typeDescription) throws Exception {
        Object result = null;

        switch(typeDescription.getTypeClass().getValue()) {
        case TypeClass.ANY_value:       result = readAny();           break; // read an any?
        case TypeClass.SEQUENCE_value:
        case TypeClass.ARRAY_value:     result = readSequence(typeDescription);  break;  // read a sequence ?
        case TypeClass.VOID_value:                                    break; // nop  // read nothing ?
        case TypeClass.ENUM_value:      result = readEnum(typeDescription);      break;  // read an enum ?
        case TypeClass.UNION_value:     result = readUnion(typeDescription);     break;  // read a union ?
        case TypeClass.TYPE_value:      result = new Type(readTypeDescription());          break;  // read a type ?
        case TypeClass.INTERFACE_value: result = readReference(typeDescription); break;  // read an interface ?
        case TypeClass.BOOLEAN_value:   result = readBoolean();       break;  // is it a boolean
        case TypeClass.CHAR_value:      result = readCharacter();     break;  // is it a character ?)
        case TypeClass.BYTE_value:      result = readByte();          break; // is it a byte ?
        case TypeClass.SHORT_value:
        case TypeClass.UNSIGNED_SHORT_value: result = readShort();         break;  // is it a short ?
        case TypeClass.LONG_value:
        case TypeClass.UNSIGNED_LONG_value: result = readInteger();       break;  // is it an integer ?
        case TypeClass.HYPER_value:
        case TypeClass.UNSIGNED_HYPER_value: result = readLong();          break;  // is it a long ?
        case TypeClass.FLOAT_value:     result = readFloat();         break;  // is it a float ?
        case TypeClass.DOUBLE_value:    result = readDouble();        break;  // is it a double ?
        case TypeClass.STRING_value:    result = readString();        break;  // is it a String ?
        case TypeClass.EXCEPTION_value: result = readThrowable(typeDescription); break;  // is it an exception?
        case TypeClass.STRUCT_value:
            if(typeDescription.getZClass() == ThreadID.class) // read a thread id ?
                result = readThreadID();
            else   // otherwise read a struct
                result = readStruct(typeDescription);

            break;

        default:
            throw new com.sun.star.uno.RuntimeException("unknown typeClass:" + typeDescription.getTypeClass());
        }

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readObject:" + typeDescription + " " + result);

        return result;
    }

    String readOid() throws Exception {
        Marshal.M_InterfaceReference m_InterfaceReference = (Marshal.M_InterfaceReference)readObject(Marshal.__M_InterfaceReferenceTypeDescription);

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

    Object readReference(TypeDescription typeDescription) throws Exception {
        Object oid = readOid();;

        // the result is a null ref, in case cache and oid are invalid
        Object result = null;

        // map the object from universe
        if(oid != null)
            result = _iBridge.mapInterfaceFrom(oid, new Type(typeDescription));

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readReference:" + typeDescription + " " + result);

        return result;
    }

    Object readSequence(TypeDescription typeDescription) throws Exception {
        Object result = null;
        if(typeDescription.getTypeClass() == TypeClass.BYTE) // read a byte sequence ?
              result = readbyteSequence();

        else {
            int size = readCompressedInt();

            typeDescription = typeDescription.getComponentType();

            if(typeDescription.getTypeClass() == TypeClass.ANY) // take special care of any array (cause anys are mapped to objects)
                result = Array.newInstance(Object.class, size);
            else
                result = Array.newInstance(typeDescription.getZClass(), size);

            for(int i = 0; i < size; ++ i)
                Array.set(result, i, readObject(typeDescription));
        }

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

    void readStruct(TypeDescription typeDescription, Object object) throws Exception {
        Field fields[] = typeDescription.getFields();

        for(int i = 0; i < fields.length; ++ i) {
            if((fields[i].getModifiers() & (Modifier.STATIC | Modifier.TRANSIENT)) == 0) { // neither static nor transient ?
                MemberTypeInfo memberTypeInfo = typeDescription.getMemberTypeInfo(fields[i].getName());

                // default the member type to the declared type
                Class zInterface = fields[i].getType();

                if(memberTypeInfo != null) {
                    if(memberTypeInfo.isAny()) // is the member an any?
                        if(zInterface.isArray())
                            zInterface = Class.forName("[Lcom.sun.star.uno.Any;");
                        else
                            zInterface = Any.class;

                    else if(memberTypeInfo.isInterface()) { // is the member an interface ?
                        // if the field type is an array
                        // we have to unrole it,
                        // to ensure that the component type
                        // is at least an XInterface.
                        Class xInterface = zInterface;

                        int array_deepness = 0;
                        while(xInterface.isArray()) {
                            xInterface = xInterface.getComponentType();
                            ++ array_deepness;
                        }

                        if(!XInterface.class.isAssignableFrom(xInterface)) // is the member type not derived of XInterface ?
                            xInterface = XInterface.class; // ensure that we get at least an XInterface

                        String array_prefix = "";
                        while(array_deepness > 0) {
                            array_prefix += "[";

                            -- array_deepness;
                        }

                        if(array_prefix.length() != 0)
                            xInterface = Class.forName(array_prefix + "L" + xInterface.getName() + ";");

                        zInterface = xInterface;
                    }
                }

                fields[i].set(object, readObject(TypeDescription.getTypeDescription(zInterface)));
            }
        }

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readStruct:" + object);
    }

    Object readStruct(TypeDescription typeDescription) throws Exception {
        Object object = typeDescription.getZClass().newInstance();

        readStruct(typeDescription, object);

        return object;
    }

    ThreadID readThreadID() throws Exception {
        Marshal.M_ThreadId m_threadId = (Marshal.M_ThreadId)readObject(Marshal.__M_ThreadIdTypeDescription);

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

    TypeDescription readTypeDescription() throws Exception {
        int typeClassValue = _dataInput.readUnsignedByte() & 0xff;

        TypeClass typeClass = TypeClass.fromInt(typeClassValue & 0x7f);
        TypeDescription typeDescription = null;

        if(TypeDescription.isTypeClassSimple(typeClass)) // is it a simple type?
            typeDescription = TypeDescription.getTypeDescription(typeClass);

        else {
            short index = _dataInput.readShort(); // the cache index

            if(index != (short)0xffff) { // shall we update the cache?
                if((typeClassValue & 0x80) != 0) // update the cache?
                    _typeDescriptionCache[index] = TypeDescription.getTypeDescription(readString());
//                      _typeDescriptionCache[index] = TypeDescription.getType(typeClass, readString());

                typeDescription = _typeDescriptionCache[index];
            }
            else
                typeDescription = TypeDescription.getTypeDescription(readString());
//                  type = TypeDescription.getType(typeClass, readString());
        }

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readType:" + typeDescription);

        return typeDescription;
    }

    Union readUnion(TypeDescription typeDescription) throws Exception {
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
