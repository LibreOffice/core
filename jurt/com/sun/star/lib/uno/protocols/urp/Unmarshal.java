/*************************************************************************
 *
 *  $RCSfile: Unmarshal.java,v $
 *
 *  $Revision: 1.11 $
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
import java.io.DataInput;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;

import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;


import com.sun.star.bridge.XInstanceProvider;

import com.sun.star.uno.Any;
import com.sun.star.uno.Enum;
import com.sun.star.uno.IBridge;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.Union;
import com.sun.star.uno.XInterface;

import com.sun.star.lib.uno.environments.remote.IUnmarshal;
import com.sun.star.lib.uno.environments.remote.Protocol;
import com.sun.star.lib.uno.environments.remote.ThreadId;

import com.sun.star.uno.ITypeDescription;
import com.sun.star.uno.IFieldDescription;

import com.sun.star.lib.uno.typedesc.TypeDescription;

class Unmarshal implements IUnmarshal {
    /**
     * When set to true, enables various debugging output.
     */
    static private final boolean DEBUG = false;

    private InputStream _inputStream;
    private DataInput   _dataInput;
    private IBridge     _iBridge;
    private Object      _objectCache[];
    private ITypeDescription        _iTypeDescriptionCache[];
    private ThreadId    _threadIdCache[];

    Unmarshal(IBridge iBridge, short cacheSize) {
        _iBridge         = iBridge;

        _objectCache          = new Object[cacheSize];
        _iTypeDescriptionCache = new ITypeDescription[cacheSize];
        _threadIdCache        = new ThreadId[cacheSize];
        _inputStream          = new ByteArrayInputStream(new byte[0]);
        _dataInput            = new DataInputStream(_inputStream);
    }

    Object readAny() {
        ITypeDescription t = readTypeDescription();
        switch (t.getTypeClass().getValue()) {
        case TypeClass.VOID_value:
            return Any.VOID;
        case TypeClass.CHAR_value:
            return readCharacter();
        case TypeClass.BOOLEAN_value:
            return readBoolean();
        case TypeClass.BYTE_value:
            return readByte();
        case TypeClass.SHORT_value:
            return readShort();
        case TypeClass.UNSIGNED_SHORT_value:
            return new Any(Type.UNSIGNED_SHORT, readShort());
        case TypeClass.LONG_value:
            return readInteger();
        case TypeClass.UNSIGNED_LONG_value:
            return new Any(Type.UNSIGNED_LONG, readInteger());
        case TypeClass.HYPER_value:
            return readLong();
        case TypeClass.UNSIGNED_HYPER_value:
            return new Any(Type.UNSIGNED_HYPER, readLong());
        case TypeClass.FLOAT_value:
            return readFloat();
        case TypeClass.DOUBLE_value:
            return readDouble();
        case TypeClass.STRING_value:
            return readString();
        case TypeClass.TYPE_value:
            return new Type(readTypeDescription());
        case TypeClass.ENUM_value:
            return readEnum(t);
        case TypeClass.STRUCT_value:
            return readStruct(t);
        case TypeClass.EXCEPTION_value:
            return readThrowable(t);
        case TypeClass.SEQUENCE_value:
            Object os = readSequence(t);
            ITypeDescription tc = t.getComponentType();
            while (tc.getTypeClass() == TypeClass.SEQUENCE) {
                tc = tc.getComponentType();
            }
            switch (tc.getTypeClass().getValue()) {
            case TypeClass.UNSIGNED_SHORT_value:
            case TypeClass.UNSIGNED_LONG_value:
            case TypeClass.UNSIGNED_HYPER_value:
                return new Any(new Type(t), os);
            default:
                return os;
            }
        case TypeClass.INTERFACE_value:
            Object oi = readReference(t);
            return t.getZClass() == XInterface.class ? oi
                : new Any(new Type(t), oi);
        default:
            throw new com.sun.star.uno.RuntimeException(
                "Reading Any with bad type " + t.getTypeClass());
        }
    }

    boolean readboolean() {
        boolean bool;

        try {
            bool = _dataInput.readBoolean();
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readboolean - unexpected:" + iOException);
        }

        return bool;
    }

    Boolean readBoolean() {
        Boolean result =  new Boolean(readboolean());

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readBoolean:" + result);

        return result;
    }

    Byte readByte() {
        Byte result = new Byte(readbyte());

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readByte:" + result);

        return result;
    }

    byte readbyte() {
        try {
            byte result = (byte)(_dataInput.readByte() & 0xff);

            if(DEBUG) System.err.println("##### " + getClass().getName() + ".readbyte:" + (result & 0xff));

            return result;
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readByte - unexpected:" + iOException);
        }
    }

    void read(byte bytes[]) {
        try {
            _inputStream.read(bytes);

            if(DEBUG) System.err.println("##### " + getClass().getName() + ".read:" + bytes);
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".read - unexpected:" + iOException);
        }
    }

    byte []readbyteSequence() {
        int size = readCompressedInt();
        byte bytes[] = new byte[size];

        read(bytes);

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readbyteSequence:" + bytes);

        return bytes;
    }

    char readchar() {
        try {
            char zchar = _dataInput.readChar();

            if(DEBUG) System.err.println("##### " + getClass().getName() + ".readChar:" + zchar);

            return zchar;
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readChar - unexpected:" + iOException);
        }
    }

    Character readCharacter() {
        Character result = new Character(readchar());

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readChar:" + result);

        return result;
    }

    double readdouble() {
        try {
            double zdouble = _dataInput.readDouble();

            if(DEBUG) System.err.println("##### " + getClass().getName() + ".readDouble:" + zdouble);

            return zdouble;
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readDouble - unexpected:" + iOException);
        }
    }

    Double readDouble() {
        Double result = new Double(readdouble());

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readDouble:" + result);

        return result;
    }

    Enum readEnum(ITypeDescription iTypeDescription) {
        try {
            Integer index = readInteger();

            Method fromInt = iTypeDescription.getZClass().getMethod("fromInt", new Class[] {int.class});
            Enum result = (Enum)fromInt.invoke(null, new Object[]{index});

            if(DEBUG) System.err.println("##### " + getClass().getName() + ".readEnum:" + result);

            return result;
        }
        catch(NoSuchMethodException noSuchMethodException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readEnum - unexpected:" + noSuchMethodException);
        }
        catch(InvocationTargetException invocationTargetException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readEnum - unexpected:" + invocationTargetException);
        }
        catch(IllegalAccessException illegalAccessException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readEnum - unexpected:" + illegalAccessException);
        }
    }

    Throwable readThrowable(ITypeDescription iTypeDescription) {
        try {
            String message = readString();

            Constructor constructor = iTypeDescription.getZClass().getConstructor(new Class[]{String.class});
            Throwable throwable = (Throwable)constructor.newInstance(new Object[]{message});

            readStruct(iTypeDescription, throwable);

            if(DEBUG) System.err.println("##### " + getClass().getName() + ".readThrowable:" + throwable);

            return throwable;
        }
        catch(NoSuchMethodException noSuchMethodException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readThrowable - unexpected:" + noSuchMethodException);
        }
        catch(InvocationTargetException invocationTargetException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readThrowable - unexpected:" + invocationTargetException);
        }
        catch(IllegalAccessException illegalAccessException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readThrowable - unexpected:" + illegalAccessException);
        }
        catch(InstantiationException instantiationException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readThrowable - unexpected:" + instantiationException);
        }
    }

    float readfloat() {
        try {
            float zfloat = _dataInput.readFloat();

            if(DEBUG) System.err.println("##### " + getClass().getName() + ".readFloat:" + zfloat);

            return zfloat;
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readfloat - unexpected:" + iOException);
        }
    }

    Float readFloat() {
        Float result = new Float(readfloat());

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readFloat:" + result);

        return result;
    }

    int readint() {
        try {
            int zint = _dataInput.readInt();

            if(DEBUG) System.err.println("##### " + getClass().getName() + ".readint:" + zint);

            return zint;
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readint - unexpected:" + iOException);
        }
    }

    Integer readInteger() {
        Integer result = new Integer(readint());

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readInteger:" + result);

        return result;
    }

    long readlong() {
        try {
            long zlong = _dataInput.readLong();

            if(DEBUG) System.err.println("##### " + getClass().getName() + ".readlong:" + zlong);

            return zlong;
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readlong - unexpected:" + iOException);
        }
    }

    Long readLong() {
        Long result = new Long(readlong());

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readLong:" + result);

        return result;
    }

    public Object readObject(ITypeDescription iTypeDescription) {
        Object result = null;

        switch(iTypeDescription.getTypeClass().getValue()) {
        case TypeClass.ANY_value:       result = readAny();           break; // read an any?
        case TypeClass.SEQUENCE_value:
        case TypeClass.ARRAY_value:     result = readSequence(iTypeDescription);  break;  // read a sequence ?
        case TypeClass.VOID_value:                                    break; // nop  // read nothing ?
        case TypeClass.ENUM_value:      result = readEnum(iTypeDescription);      break;  // read an enum ?
        case TypeClass.UNION_value:     result = readUnion(iTypeDescription);     break;  // read a union ?
        case TypeClass.TYPE_value:      result = new Type(readTypeDescription());          break;  // read a type ?
        case TypeClass.INTERFACE_value: result = readReference(iTypeDescription); break;  // read an interface ?
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
        case TypeClass.EXCEPTION_value: result = readThrowable(iTypeDescription); break;  // is it an exception?
        case TypeClass.STRUCT_value:
            if(iTypeDescription.getZClass() == ThreadId.class) // read a thread id ?
                result = readThreadId();

            else   // otherwise read a struct
                result = readStruct(iTypeDescription);

            break;

        default:
            throw new com.sun.star.uno.RuntimeException("unknown typeClass:" + iTypeDescription.getTypeClass());
        }

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readObject:" + iTypeDescription + " >" + result + "<");

        return result;
    }

    String readOid() {
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

    Object readReference(ITypeDescription iTypeDescription) {
        Object oid = readOid();;

        // the result is a null ref, in case cache and oid are invalid
        Object result = null;

        // map the object from universe
        if(oid != null)
            result = _iBridge.mapInterfaceFrom(oid, new Type(iTypeDescription));

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readReference:" + iTypeDescription + " " + result);

        return result;
    }

    Object readSequence(ITypeDescription iTypeDescription) {
        Object result = null;
        if(iTypeDescription.getTypeClass() == TypeClass.BYTE) // read a byte sequence ?
              result = readbyteSequence();

        else {
            int size = readCompressedInt();

            iTypeDescription = iTypeDescription.getComponentType();

            if(iTypeDescription.getTypeClass() == TypeClass.ANY) // take special care of any array (cause anys are mapped to objects)
                result = Array.newInstance(Object.class, size);
            else
                result = Array.newInstance(iTypeDescription.getZClass(), size);

            for(int i = 0; i < size; ++ i)
                Array.set(result, i, readObject(iTypeDescription));
        }

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readSequence:" + result);

        return result;
    }

    Short readShort() {
        Short result = new Short(readshort());

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readShort:" + result);

        return result;
    }

    short readshort() {
        try {
            short result = _dataInput.readShort();

            if(DEBUG) System.err.println("##### " + getClass().getName() + ".readshort:" + result);

            return result;
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readshort - unexpected:" + iOException);
        }
    }

    int readCompressedInt() {
        int result = readbyte() & 0xff;

        if(result == 255) // if 255 then there follows a complete int
            result = readint();

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readCompressedInt:" + result);

        return result;
    }

    String readString() {
        try {
            int utflen = readCompressedInt(); // the size of the string

            byte bytes[] = new byte[utflen];
            read(bytes);

            return new String(bytes, "UTF8");
        }
        catch(UnsupportedEncodingException unsupportedEncodingException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readString - unexpected:" + unsupportedEncodingException);
        }
    }

    void readStruct(ITypeDescription iTypeDescription, Object object) {
        IFieldDescription iFieldDescriptions[] = iTypeDescription.getFieldDescriptions();

          for(int i = 0; i < iFieldDescriptions.length; ++ i) {
            try {
                iFieldDescriptions[i].getField().set(object, readObject(iFieldDescriptions[i].getTypeDescription()));
            }
            catch(IllegalAccessException illegalAccessException) {
                throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readStruct - unexpected:" + illegalAccessException);
            }
        }

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readStruct:" + object);
    }

    Object readStruct(ITypeDescription iTypeDescription) {
        try {
            Object object = iTypeDescription.getZClass().newInstance();

            readStruct(iTypeDescription, object);

            return object;
        }
        catch(IllegalAccessException illegalAccessException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readStruct - unexpected:" + illegalAccessException);
        }
        catch(InstantiationException instantiationException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readStruct - unexpected:" + instantiationException);
        }
    }

    ThreadId readThreadId() {
        Marshal.M_ThreadId m_threadId = (Marshal.M_ThreadId)readObject(Marshal.__M_ThreadIdTypeDescription);

        ThreadId threadId = null;

        if(m_threadId.cache != (short)0xffff) { // is the cache entry valid?
            if(m_threadId.full.length != 0)
                _threadIdCache[m_threadId.cache] = new ThreadId(m_threadId.full);

            threadId = _threadIdCache[m_threadId.cache];
        }
        else if(m_threadId.full.length != 0)
            threadId = new ThreadId(m_threadId.full);

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readThreadID:" + _threadIdCache[m_threadId.cache]);

        return threadId;
    }


    int readunsignedbyte() {
        try {
            int result = (byte)(_dataInput.readUnsignedByte() & 0xff);

            if(DEBUG) System.err.println("##### " + getClass().getName() + ".readunsignedbyte:" + result);

            return result;
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readunsignedbyte - unexpected:" + iOException);
        }
    }

    ITypeDescription readTypeDescription() {
        int typeClassValue = readunsignedbyte() & 0xff;

        TypeClass typeClass = TypeClass.fromInt(typeClassValue & 0x7f);
        ITypeDescription iTypeDescription = null;

        if(TypeDescription.isTypeClassSimple(typeClass)) // is it a simple type?
            iTypeDescription = TypeDescription.getTypeDescription(typeClass);

        else {
            try {
                short index = readshort(); // the cache index

                if(index != (short)0xffff) { // shall we update the cache?
                    if((typeClassValue & 0x80) != 0) {// update the cache?
                        _iTypeDescriptionCache[index] = TypeDescription.getTypeDescription(readString());
                    }
                    iTypeDescription = _iTypeDescriptionCache[index];
                }
                else
                    iTypeDescription = TypeDescription.getTypeDescription(readString());
            }
            catch(ClassNotFoundException classNotFoundException) {
                throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readTypeDescription - unexpected:" + classNotFoundException);
            }
        }

        if(DEBUG) System.err.println("##### " + getClass().getName() + ".readTypeDescription:" + iTypeDescription);

        return iTypeDescription;
    }

    Union readUnion(ITypeDescription iTypeDescription) {
        throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".readUnion - not implemented!!!");
    }

    void reset(byte bytes[]) {
        _inputStream = new ByteArrayInputStream(bytes);
        _dataInput = new DataInputStream(_inputStream);
    }

    int bytesLeft() {
        try {
            return _inputStream.available();
        }
        catch(IOException iOException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".bytesLeft - unexpected:" + iOException);
        }
    }
}
