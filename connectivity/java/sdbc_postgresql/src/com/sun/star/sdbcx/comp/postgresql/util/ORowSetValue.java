/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

package com.sun.star.sdbcx.comp.postgresql.util;

import java.io.ByteArrayOutputStream;
import java.io.UnsupportedEncodingException;

import com.sun.star.io.IOException;
import com.sun.star.io.XInputStream;
import com.sun.star.sdbc.DataType;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XBlob;
import com.sun.star.sdbc.XClob;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.Date;
import com.sun.star.util.DateTime;
import com.sun.star.util.Time;

public class ORowSetValue {
    private Object value;
    private int typeKind;
    private int flags;
    private static final int FLAG_NULL = 0b1000;
    private static final int FLAG_BOUND = 0b0100;
    private static final int FLAG_MODIFIED = 0b0010;
    private static final int FLAG_SIGNED = 0b0001;

    public ORowSetValue() {
        flags = FLAG_NULL | FLAG_BOUND | FLAG_SIGNED;
        typeKind = DataType.VARCHAR;
    }

    public ORowSetValue(boolean value) {
        this();
        setBoolean(value);
    }

    public ORowSetValue(Date value) {
        this();
        setDate(value);
    }

    public ORowSetValue(DateTime value) {
        this();
        setDateTime(value);
    }

    public ORowSetValue(double value) {
        this();
        setDouble(value);
    }

    public ORowSetValue(float value) {
        this();
        setFloat(value);
    }

    public ORowSetValue(byte value) {
        this();
        setInt8(value);
    }

    public ORowSetValue(short value) {
        this();
        setInt16(value);
    }

    public ORowSetValue(int value) {
        this();
        setInt32(value);
    }

    public ORowSetValue(long value) {
        this();
        setLong(value);
    }

    public ORowSetValue(byte[] value) {
        this();
        setSequence(value);
    }

    public ORowSetValue(String value) {
        this();
        setString(value);
    }

    public ORowSetValue(Time value) {
        this();
        setTime(value);
    }

    public boolean isNull() {
        return (flags & FLAG_NULL) != 0;
    }

    public void setNull() {
        free();
        flags |= FLAG_NULL;
    }

    public boolean isBound() {
        return (flags & FLAG_BOUND) != 0;
    }

    public void setBound(boolean isBound) {
        if (isBound) {
            flags |= FLAG_BOUND;
        } else {
            flags &= ~FLAG_BOUND;
        }
    }

    public boolean isModified() {
        return (flags & FLAG_MODIFIED) != 0;
    }

    public void setModified(boolean isModified) {
        flags |= FLAG_MODIFIED;
    }

    public boolean isSigned() {
        return (flags & FLAG_SIGNED) != 0;
    }

    public void setSigned() throws IOException, SQLException {
        setSigned(true);
    }

    public void setSigned(boolean isSigned) {
        if (isSigned) {
            flags |= FLAG_SIGNED;
        } else {
            flags &= ~FLAG_SIGNED;
        }
    }

    private boolean isStorageCompatible(int _eType1, int _eType2) {
        boolean bIsCompatible = true;

        if (_eType1 != _eType2) {
            switch (_eType1) {
            case DataType.CHAR:
            case DataType.VARCHAR:
            case DataType.DECIMAL:
            case DataType.NUMERIC:
            case DataType.LONGVARCHAR:
                bIsCompatible = (DataType.CHAR         == _eType2)
                            ||  (DataType.VARCHAR      == _eType2)
                            ||  (DataType.DECIMAL      == _eType2)
                            ||  (DataType.NUMERIC      == _eType2)
                            ||  (DataType.LONGVARCHAR  == _eType2);
                break;

            case DataType.DOUBLE:
            case DataType.REAL:
                bIsCompatible = (DataType.DOUBLE   == _eType2)
                            ||  (DataType.REAL     == _eType2);
                break;

            case DataType.BINARY:
            case DataType.VARBINARY:
            case DataType.LONGVARBINARY:
                bIsCompatible = (DataType.BINARY           == _eType2)
                            ||  (DataType.VARBINARY        == _eType2)
                            ||  (DataType.LONGVARBINARY    == _eType2);
                break;

            case DataType.INTEGER:
                bIsCompatible = (DataType.SMALLINT == _eType2)
                            ||  (DataType.TINYINT  == _eType2)
                            ||  (DataType.BIT      == _eType2)
                            ||  (DataType.BOOLEAN  == _eType2);
                break;
            case DataType.SMALLINT:
                bIsCompatible = (DataType.TINYINT  == _eType2)
                            ||  (DataType.BIT      == _eType2)
                            ||  (DataType.BOOLEAN  == _eType2);
                break;
            case DataType.TINYINT:
                bIsCompatible = (DataType.BIT      == _eType2)
                            ||  (DataType.BOOLEAN  == _eType2);
                break;

            case DataType.BLOB:
            case DataType.CLOB:
            case DataType.OBJECT:
                bIsCompatible = (DataType.BLOB     == _eType2)
                            ||  (DataType.CLOB     == _eType2)
                            ||  (DataType.OBJECT   == _eType2);
                break;

            default:
                bIsCompatible = false;
            }
        }
        return bIsCompatible;
    }

    public int getTypeKind() {
        return typeKind;
    }

    public void setTypeKind(int type) throws SQLException {
        if (!isNull() && !isStorageCompatible(type, typeKind)) {
            switch (type) {
            case DataType.VARCHAR:
            case DataType.CHAR:
            case DataType.DECIMAL:
            case DataType.NUMERIC:
            case DataType.LONGVARCHAR:
                setString(getString());
                break;
            case DataType.BIGINT:
                setLong(getLong());
                break;

            case DataType.FLOAT:
                setFloat(getFloat());
                break;
            case DataType.DOUBLE:
            case DataType.REAL:
                setDouble(getDouble());
                break;
            case DataType.TINYINT:
                setInt8(getInt8());
                break;
            case DataType.SMALLINT:
                setInt16(getInt16());
                break;
            case DataType.INTEGER:
                setInt32(getInt32());
                break;
            case DataType.BIT:
            case DataType.BOOLEAN:
                setBoolean(getBoolean());
                break;
            case DataType.DATE:
                setDate(getDate());
                break;
            case DataType.TIME:
                setTime(getTime());
                break;
            case DataType.TIMESTAMP:
                setDateTime(getDateTime());
                break;
            case DataType.BINARY:
            case DataType.VARBINARY:
            case DataType.LONGVARBINARY:
                setSequence(getSequence());
                break;
            case DataType.BLOB:
            case DataType.CLOB:
            case DataType.OBJECT:
            case DataType.OTHER:
                setAny(getAny());
                break;
            default:
                setAny(getAny());
                //OSL_ENSURE(0,"ORowSetValue:operator==(): UNSPUPPORTED TYPE!");
            }
        }
        typeKind = type;
    }

    private void free() {
        if (!isNull()) {
            value = null;
            flags |= FLAG_NULL;
        }
    }

    public Any getAny() {
        Any any = (Any)value;
        return new Any(any.getType(), any.getObject());
    }

    public boolean getBoolean() {
        boolean bRet = false;
        if (!isNull()) {
            switch (getTypeKind()) {
            case DataType.CHAR:
            case DataType.VARCHAR:
            case DataType.LONGVARCHAR:
                if (((String)value).equals("true")) {
                    bRet = true;
                } else if (((String)value).equals("false")) {
                    bRet = false;
                }
                // fall through
            case DataType.DECIMAL:
            case DataType.NUMERIC:
                bRet = DBTypeConversion.safeParseInt((String)value) != 0;
                break;
            case DataType.BIGINT:
                bRet = (long)value != 0;
                break;
            case DataType.FLOAT:
                bRet = (float)value != 0.0;
                break;
            case DataType.DOUBLE:
            case DataType.REAL:
                bRet = (double)value != 0.0;
                break;
            case DataType.DATE:
            case DataType.TIME:
            case DataType.TIMESTAMP:
            case DataType.BINARY:
            case DataType.VARBINARY:
            case DataType.LONGVARBINARY:
                break;
            case DataType.BIT:
            case DataType.BOOLEAN:
                bRet = (boolean)value;
                break;
            case DataType.TINYINT:
                bRet = (byte)value != 0;
                break;
            case DataType.SMALLINT:
                bRet = (short)value != 0;
                break;
            case DataType.INTEGER:
                bRet = (int)value != 0;
                break;
            default:
                try {
                    bRet = AnyConverter.toBoolean(value);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                }
                break;
            }
        }
        return bRet;
    }

    public Date getDate() throws SQLException {
        Date aValue = new Date();
        if (!isNull()) {
            switch (getTypeKind()) {
            case DataType.CHAR:
            case DataType.VARCHAR:
            case DataType.LONGVARCHAR:
                aValue = DBTypeConversion.toDate(getString());
                break;
            case DataType.DECIMAL:
            case DataType.NUMERIC:
            case DataType.FLOAT:
            case DataType.DOUBLE:
            case DataType.REAL:
                aValue = DBTypeConversion.toDate(getDouble());
                break;
            case DataType.DATE:
                Date date    = (Date)value;
                aValue.Day   = date.Day;
                aValue.Month = date.Month;
                aValue.Year  = date.Year;
                break;
            case DataType.TIMESTAMP:
                DateTime dateTime = (DateTime)value;
                aValue.Day        = dateTime.Day;
                aValue.Month      = dateTime.Month;
                aValue.Year       = dateTime.Year;
                break;
            case DataType.BIT:
            case DataType.BOOLEAN:
            case DataType.TINYINT:
            case DataType.SMALLINT:
            case DataType.INTEGER:
            case DataType.BIGINT:
                aValue = DBTypeConversion.toDate((double)getLong());
                break;

            case DataType.BLOB:
            case DataType.CLOB:
            case DataType.OBJECT:
            default:
                //OSL_ENSURE( false, "ORowSetValue::getDate: cannot retrieve the data!" );
                // NO break!

            case DataType.BINARY:
            case DataType.VARBINARY:
            case DataType.LONGVARBINARY:
            case DataType.TIME:
                aValue = DBTypeConversion.toDate(0.0);
                break;
            }
        }
        return aValue;

    }

    public DateTime getDateTime() throws SQLException {
        DateTime aValue = new DateTime();
        if (!isNull()) {
            switch (getTypeKind()) {
            case DataType.CHAR:
            case DataType.VARCHAR:
            case DataType.LONGVARCHAR:
                aValue = DBTypeConversion.toDateTime(getString());
                break;
            case DataType.DECIMAL:
            case DataType.NUMERIC:
            case DataType.FLOAT:
            case DataType.DOUBLE:
            case DataType.REAL:
                aValue = DBTypeConversion.toDateTime(getDouble());
                break;
            case DataType.DATE:
                Date date       = (Date)value;
                aValue.Day      = date.Day;
                aValue.Month    = date.Month;
                aValue.Year     = date.Year;
                break;
            case DataType.TIME:
                Time time               = (Time)value;
                aValue.HundredthSeconds = time.HundredthSeconds;
                aValue.Seconds          = time.Seconds;
                aValue.Minutes          = time.Minutes;
                aValue.Hours            = time.Hours;
                break;
            case DataType.TIMESTAMP:
                DateTime dateTime       = (DateTime)value;
                aValue.Year             = dateTime.Year;
                aValue.Month            = dateTime.Month;
                aValue.Day              = dateTime.Day;
                aValue.Hours            = dateTime.Hours;
                aValue.Minutes          = dateTime.Minutes;
                aValue.Seconds          = dateTime.Seconds;
                aValue.HundredthSeconds = dateTime.HundredthSeconds;
                break;
            default:
                try {
                    DateTime any            = AnyConverter.toObject(DateTime.class, value);
                    aValue.Year             = any.Year;
                    aValue.Month            = any.Month;
                    aValue.Day              = any.Day;
                    aValue.Hours            = any.Hours;
                    aValue.Minutes          = any.Minutes;
                    aValue.Seconds          = any.Seconds;
                    aValue.HundredthSeconds = any.HundredthSeconds;
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                } catch (ClassCastException classCastException) {
                }
                break;
            }
        }
        return aValue;
    }

    public double getDouble() {
        double nRet = 0.0;
        if (!isNull()) {
            switch (getTypeKind()) {
            case DataType.CHAR:
            case DataType.VARCHAR:
            case DataType.DECIMAL:
            case DataType.NUMERIC:
            case DataType.LONGVARCHAR:
                nRet = DBTypeConversion.safeParseDouble((String)value);
                break;
            case DataType.BIGINT:
                nRet = isSigned() ? (long)value : DBTypeConversion.unsignedLongToDouble((long)value);
                break;
            case DataType.FLOAT:
                nRet = (float)value;
                break;
            case DataType.DOUBLE:
            case DataType.REAL:
                nRet = (double)value;
                break;
            case DataType.DATE:
                nRet = DBTypeConversion.toDouble((Date)value);
                break;
            case DataType.TIME:
                nRet = DBTypeConversion.toDouble((Time)value);
                break;
            case DataType.TIMESTAMP:
                nRet = DBTypeConversion.toDouble((DateTime)value);
                break;
            case DataType.BINARY:
            case DataType.VARBINARY:
            case DataType.LONGVARBINARY:
            case DataType.BLOB:
            case DataType.CLOB:
                //OSL_ASSERT(!"getDouble() for this type is not allowed!");
                break;
            case DataType.BIT:
            case DataType.BOOLEAN:
                nRet = (boolean)value ? 1 : 0;
                break;
            case DataType.TINYINT:
                nRet = isSigned() ? (byte)value : 0xff & (byte)value;
                break;
            case DataType.SMALLINT:
                nRet = isSigned() ? (short)value : 0xffff & (short)value;
                break;
            case DataType.INTEGER:
                nRet = isSigned() ? (int)value : 0xffffFFFFL & (int)value;
                break;
            default:
                try {
                    nRet = AnyConverter.toDouble(value);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                }
                break;
            }
        }
        return nRet;
    }

    public float getFloat() {
        float nRet = 0.0f;
        if (!isNull()) {
            switch (getTypeKind()) {
            case DataType.CHAR:
            case DataType.VARCHAR:
            case DataType.DECIMAL:
            case DataType.NUMERIC:
            case DataType.LONGVARCHAR:
                nRet = DBTypeConversion.safeParseFloat((String)value);
                break;
            case DataType.BIGINT:
                nRet = isSigned() ? (long)value : DBTypeConversion.unsignedLongToFloat((long)value);
                break;
            case DataType.FLOAT:
                nRet = (float)value;
                break;
            case DataType.DOUBLE:
            case DataType.REAL:
                nRet = (float)(double)value;
                break;
            case DataType.DATE:
                nRet = (float)DBTypeConversion.toDouble((Date)value);
                break;
            case DataType.TIME:
                nRet = (float)DBTypeConversion.toDouble((Time)value);
                break;
            case DataType.TIMESTAMP:
                nRet = (float)DBTypeConversion.toDouble((DateTime)value);
                break;
            case DataType.BINARY:
            case DataType.VARBINARY:
            case DataType.LONGVARBINARY:
            case DataType.BLOB:
            case DataType.CLOB:
                //OSL_ASSERT(!"getDouble() for this type is not allowed!");
                break;
            case DataType.BIT:
            case DataType.BOOLEAN:
                nRet = (boolean)value ? 1 : 0;
                break;
            case DataType.TINYINT:
                nRet = isSigned() ? (byte)value : 0xff & (byte)value;
                break;
            case DataType.SMALLINT:
                nRet = isSigned() ? (short)value : 0xffff & (short)value;
                break;
            case DataType.INTEGER:
                nRet = isSigned() ? (int)value : 0xffffFFFFL & (int)value;
                break;
            default:
                try {
                    nRet = AnyConverter.toFloat(value);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                }
                break;
            }
        }
        return nRet;
    }

    public byte getInt8() {
        byte nRet = 0;
        if (!isNull()) {
            switch (getTypeKind()) {
            case DataType.CHAR:
            case DataType.VARCHAR:
            case DataType.DECIMAL:
            case DataType.NUMERIC:
            case DataType.LONGVARCHAR:
                nRet = (byte)DBTypeConversion.safeParseInt((String)value);
                break;
            case DataType.BIGINT:
                nRet = (byte)(long)value;
                break;
            case DataType.FLOAT:
                nRet = (byte)(float)value;
                break;
            case DataType.DOUBLE:
            case DataType.REAL:
                nRet = (byte)(double)value;
                break;
            case DataType.DATE:
            case DataType.TIME:
            case DataType.TIMESTAMP:
            case DataType.BINARY:
            case DataType.VARBINARY:
            case DataType.LONGVARBINARY:
            case DataType.BLOB:
            case DataType.CLOB:
                break;
            case DataType.BIT:
            case DataType.BOOLEAN:
                nRet = (byte)((boolean)value ? 1 : 0);
                break;
            case DataType.TINYINT:
                nRet = (byte)value;
                break;
            case DataType.SMALLINT:
                nRet = (byte)(short)value;
                break;
            case DataType.INTEGER:
                nRet = (byte)(int)value;
                break;
            default:
                try {
                    nRet = AnyConverter.toByte(value);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                }
                break;
            }
        }
        return nRet;
    }

    public short getInt16() {
        short nRet = 0;
        if (!isNull()) {
            switch (getTypeKind()) {
            case DataType.CHAR:
            case DataType.VARCHAR:
            case DataType.DECIMAL:
            case DataType.NUMERIC:
            case DataType.LONGVARCHAR:
                nRet = (short)DBTypeConversion.safeParseInt((String)value);
                break;
            case DataType.BIGINT:
                nRet = (short)(long)value;
                break;
            case DataType.FLOAT:
                nRet = (short)(float)value;
                break;
            case DataType.DOUBLE:
            case DataType.REAL:
                nRet = (short)(double)value;
                break;
            case DataType.DATE:
            case DataType.TIME:
            case DataType.TIMESTAMP:
            case DataType.BINARY:
            case DataType.VARBINARY:
            case DataType.LONGVARBINARY:
            case DataType.BLOB:
            case DataType.CLOB:
                break;
            case DataType.BIT:
            case DataType.BOOLEAN:
                nRet = (short)((boolean)value ? 1 : 0);
                break;
            case DataType.TINYINT:
                nRet = (short)(isSigned() ? (byte)value : 0xff & (byte)value);
                break;
            case DataType.SMALLINT:
                nRet = (short)value;
                break;
            case DataType.INTEGER:
                nRet = (short)(int)value;
                break;
            default:
                try {
                    nRet = AnyConverter.toShort(value);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                }
                break;
            }
        }
        return nRet;
    }

    public int getInt32() {
        int nRet = 0;
        if (!isNull()) {
            switch (getTypeKind()) {
            case DataType.CHAR:
            case DataType.VARCHAR:
            case DataType.DECIMAL:
            case DataType.NUMERIC:
            case DataType.LONGVARCHAR:
                nRet = DBTypeConversion.safeParseInt((String)value);
                break;
            case DataType.BIGINT:
                nRet = (int)(long)value;
                break;
            case DataType.FLOAT:
                nRet = (int)(float)value;
                break;
            case DataType.DOUBLE:
            case DataType.REAL:
                nRet = (int)(double)value;
                break;
            case DataType.DATE:
                nRet = DBTypeConversion.toDays((Date)value);
                break;
            case DataType.TIME:
            case DataType.TIMESTAMP:
            case DataType.BINARY:
            case DataType.VARBINARY:
            case DataType.LONGVARBINARY:
            case DataType.BLOB:
            case DataType.CLOB:
                break;
            case DataType.BIT:
            case DataType.BOOLEAN:
                nRet = (boolean)value ? 1 : 0;
                break;
            case DataType.TINYINT:
                nRet = isSigned() ? (byte)value : 0xff & (byte)value;
                break;
            case DataType.SMALLINT:
                nRet = isSigned() ? (short)value : 0xffff & (short)value;
                break;
            case DataType.INTEGER:
                nRet = (int)value;
                break;
            default:
                try {
                    nRet = AnyConverter.toInt(value);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                }
                break;
            }
        }
        return nRet;
    }

    public long getLong() {
        long nRet = 0;
        if (!isNull()) {
            switch (getTypeKind()) {
            case DataType.CHAR:
            case DataType.VARCHAR:
            case DataType.DECIMAL:
            case DataType.NUMERIC:
            case DataType.LONGVARCHAR:
                nRet = DBTypeConversion.safeParseLong((String)value);
                break;
            case DataType.BIGINT:
                nRet = (long)value;
                break;
            case DataType.FLOAT:
                nRet = (long)(float)value;
                break;
            case DataType.DOUBLE:
            case DataType.REAL:
                nRet = (long)(double)value;
                break;
            case DataType.DATE:
                nRet = DBTypeConversion.toDays((Date)value);
                break;
            case DataType.TIME:
            case DataType.TIMESTAMP:
            case DataType.BINARY:
            case DataType.VARBINARY:
            case DataType.LONGVARBINARY:
            case DataType.BLOB:
            case DataType.CLOB:
                break;
            case DataType.BIT:
            case DataType.BOOLEAN:
                nRet = (boolean)value ? 1 : 0;
                break;
            case DataType.TINYINT:
                nRet = isSigned() ? (byte)value : 0xff & (byte)value;
                break;
            case DataType.SMALLINT:
                nRet = isSigned() ? (short)value : 0xffff & (short)value;
                break;
            case DataType.INTEGER:
                nRet = isSigned() ? (int)value : 0xffffFFFFL & (int)value;
                break;
            default:
                try {
                    nRet = AnyConverter.toInt(value);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                }
                break;
            }
        }
        return nRet;
    }

    public byte[] getSequence() throws SQLException {
        byte[] aSeq = new byte[0];
        if (!isNull()) {
            switch (getTypeKind()) {
            case DataType.OBJECT:
            case DataType.CLOB:
            case DataType.BLOB:
                XInputStream xStream = null;
                if (value != null) {
                    XBlob blob = UnoRuntime.queryInterface(XBlob.class, value);
                    if (blob != null) {
                        xStream = blob.getBinaryStream();
                    } else {
                        XClob clob = UnoRuntime.queryInterface(XClob.class, value);
                        if (clob != null) {
                            xStream = clob.getCharacterStream();
                        }
                    }
                    if (xStream != null) {
                        try {
                            try {
                                final int bytesToRead = 65535;
                                byte[][] aReadSeq = new byte[1][];
                                ByteArrayOutputStream baos = new ByteArrayOutputStream();
                                int read;
                                do {
                                    read = xStream.readBytes(aReadSeq, bytesToRead);
                                    baos.write(aReadSeq[0], 0, read);
                                } while (read == bytesToRead);
                                aSeq = baos.toByteArray();
                            } finally {
                                xStream.closeInput();
                            }
                        } catch (IOException ioException) {
                            throw new SQLException(ioException.getMessage());
                        }
                    }
                }
                break;
            case DataType.VARCHAR:
            case DataType.LONGVARCHAR:
                try {
                    aSeq = ((String)value).getBytes("UTF-16");
                } catch (UnsupportedEncodingException unsupportedEncodingException) {
                }
                break;
            case DataType.BINARY:
            case DataType.VARBINARY:
            case DataType.LONGVARBINARY:
                aSeq = ((byte[])value).clone();
                break;
            default:
                try {
                    aSeq = ((byte[])AnyConverter.toArray(value)).clone();
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                } catch (ClassCastException classCastException) {
                }
                break;
            }
        }
        return aSeq;
    }

    public String getString() throws SQLException {
        String aRet = "";
        if (!isNull()) {
            switch (getTypeKind()) {
            case DataType.CHAR:
            case DataType.VARCHAR:
            case DataType.DECIMAL:
            case DataType.NUMERIC:
            case DataType.LONGVARCHAR:
                aRet = (String)value;
                break;
            case DataType.BIGINT:
                aRet = isSigned() ? Long.toString((long)value) : Long.toUnsignedString((long)value);
                break;
            case DataType.FLOAT:
                aRet = ((Float)value).toString();
                break;
            case DataType.DOUBLE:
            case DataType.REAL:
                aRet = ((Double)value).toString();
                break;
            case DataType.DATE:
                aRet = DBTypeConversion.toDateString((Date)value);
                break;
            case DataType.TIME:
                aRet = DBTypeConversion.toTimeString((Time)value);
                break;
            case DataType.TIMESTAMP:
                aRet = DBTypeConversion.toDateTimeString((DateTime)value);
                break;
            case DataType.BINARY:
            case DataType.VARBINARY:
            case DataType.LONGVARBINARY:
                {
                    StringBuilder sVal = new StringBuilder("0x");
                    byte[] sSeq = getSequence();
                    for (byte b : sSeq) {
                        sVal.append(String.format("%02x", Byte.toUnsignedInt(b)));
                    }
                    aRet = sVal.toString();
                }
                break;
            case DataType.BIT:
            case DataType.BOOLEAN:
                aRet = ((Boolean)value).toString();
                break;
            case DataType.TINYINT:
                aRet = isSigned() ? Integer.toString((byte)value) : Integer.toUnsignedString(0xff & (byte)value);
                break;
            case DataType.SMALLINT:
                aRet = isSigned() ? Integer.toString((short)value) : Integer.toUnsignedString(0xffff & (short)value);
                break;
            case DataType.INTEGER:
                aRet = isSigned() ? Integer.toString((int)value) : Integer.toUnsignedString((int)value);
                break;
            case DataType.CLOB:
                if (AnyConverter.isObject(value)) {
                    try {
                        XClob clob = AnyConverter.toObject(XClob.class, value);
                        if (clob != null) {
                            aRet = clob.getSubString(1, (int)clob.length());
                        }
                    } catch (ClassCastException classCastException) {
                    } catch (com.sun.star.lang.IllegalArgumentException e) {
                    }
                }
                break;
            default:
                try {
                    aRet = AnyConverter.toString(value);
                } catch (com.sun.star.lang.IllegalArgumentException e) {
                }
                break;
            }
        }
        return aRet;
    }

    public Time getTime() throws SQLException {
        Time aValue = new Time();
        if (!isNull()) {
            switch (getTypeKind()) {
                case DataType.CHAR:
                case DataType.VARCHAR:
                case DataType.LONGVARCHAR:
                    aValue = DBTypeConversion.toTime(getString());
                    break;
                case DataType.DECIMAL:
                case DataType.NUMERIC:
                    aValue = DBTypeConversion.toTime(getDouble());
                    break;
                case DataType.FLOAT:
                case DataType.DOUBLE:
                case DataType.REAL:
                    aValue = DBTypeConversion.toTime(getDouble());
                    break;
                case DataType.TIMESTAMP:
                    DateTime pDateTime      = (DateTime)value;
                    aValue.HundredthSeconds = pDateTime.HundredthSeconds;
                    aValue.Seconds          = pDateTime.Seconds;
                    aValue.Minutes          = pDateTime.Minutes;
                    aValue.Hours            = pDateTime.Hours;
                    break;
                case DataType.TIME:
                    Time time               = (Time)value;
                    aValue.Hours            = time.Hours;
                    aValue.Minutes          = time.Minutes;
                    aValue.Seconds          = time.Seconds;
                    aValue.HundredthSeconds = time.HundredthSeconds;
                    break;
                default:
                    try {
                        aValue = AnyConverter.toObject(Time.class, value);
                    } catch (com.sun.star.lang.IllegalArgumentException e) {
                    } catch (ClassCastException classCastException) {
                    }
                    break;
            }
        }
        return aValue;
    }

    public void setAny(Any value) {
        flags &= ~FLAG_NULL;
        this.value = new Any(value.getType(), value.getObject());
        typeKind = DataType.OBJECT;
    }

    public void setBoolean(boolean value) {
        flags &= ~FLAG_NULL;
        this.value = value;
        typeKind = DataType.BIT;
    }

    public void setDate(Date date) {
        flags &= ~FLAG_NULL;
        this.value = new Date(date.Day, date.Month, date.Year);
        typeKind = DataType.DATE;
    }

    public void setDateTime(DateTime value) {
        flags &= ~FLAG_NULL;
        this.value = new DateTime(value.HundredthSeconds, value.Seconds, value.Minutes, value.Hours,
                value.Day, value.Minutes, value.Year);
        typeKind = DataType.TIMESTAMP;
    }

    public void setDouble(double value) {
        flags &= ~FLAG_NULL;
        this.value = value;
        typeKind = DataType.DOUBLE;
    }

    public void setFloat(float value) {
        flags &= ~FLAG_NULL;
        this.value = value;
        typeKind = DataType.FLOAT;
    }

    public void setInt8(byte value) {
        flags &= ~FLAG_NULL;
        this.value = value;
        typeKind = DataType.TINYINT;
    }

    public void setInt16(short value) {
        flags &= ~FLAG_NULL;
        this.value = value;
        typeKind = DataType.SMALLINT;
    }

    public void setInt32(int value) {
        flags &= ~FLAG_NULL;
        this.value = value;
        typeKind = DataType.INTEGER;
    }

    public void setLong(long value) {
        flags &= ~FLAG_NULL;
        this.value = value;
        typeKind = DataType.BIGINT;
    }

    public void setSequence(byte[] value) {
        flags &= ~FLAG_NULL;
        this.value = value.clone();
        typeKind = DataType.LONGVARBINARY;
    }

    public void setString(String value) {
        flags &= ~FLAG_NULL;
        this.value = value;
        typeKind = DataType.VARCHAR;
    }

    public void setTime(Time value) {
        flags &= ~FLAG_NULL;
        this.value = new Time(value.Hours, value.Minutes, value.Seconds, value.HundredthSeconds);
        typeKind = DataType.TIME;
    }

    public Object makeAny() {
        Object rValue = new Any(Type.VOID, null);
        if(isBound() && !isNull()) {
            switch (getTypeKind()) {
            case DataType.CHAR:
            case DataType.VARCHAR:
            case DataType.DECIMAL:
            case DataType.NUMERIC:
            case DataType.LONGVARCHAR:
                rValue = value;
                break;
            case DataType.BIGINT:
                rValue = value;
                break;
            case DataType.FLOAT:
                rValue = value;
                break;
            case DataType.DOUBLE:
            case DataType.REAL:
                rValue = value;
                break;
            case DataType.DATE:
                Date date = (Date)value;
                Date dateOut = new Date();
                dateOut.Day = date.Day;
                dateOut.Month = date.Month;
                dateOut.Year = date.Year;
                rValue = dateOut;
                break;
            case DataType.TIME:
                Time time = (Time)value;
                Time timeOut = new Time();
                timeOut.Hours = time.Hours;
                timeOut.Minutes = time.Minutes;
                timeOut.Seconds = time.Seconds;
                timeOut.HundredthSeconds = time.HundredthSeconds;
                rValue = timeOut;
                break;
            case DataType.TIMESTAMP:
                DateTime dateTime = (DateTime)value;
                DateTime dateTimeOut = new DateTime(dateTime.HundredthSeconds, dateTime.Seconds, dateTime.Minutes, dateTime.Hours,
                        dateTime.Day, dateTime.Minutes, dateTime.Year);
                rValue = dateTimeOut;
                break;
            case DataType.BINARY:
            case DataType.VARBINARY:
            case DataType.LONGVARBINARY:
                rValue = ((byte[])value).clone();
                break;
            case DataType.BLOB:
            case DataType.CLOB:
            case DataType.OBJECT:
            case DataType.OTHER:
                rValue = getAny();
                break;
            case DataType.BIT:
            case DataType.BOOLEAN:
                rValue = (boolean)value;
                break;
            case DataType.TINYINT:
                rValue = (byte)value;
                break;
            case DataType.SMALLINT:
                rValue = (short)value;
                break;
            case DataType.INTEGER:
                rValue = (int)value;
                break;
            default:
                rValue = getAny();
                break;
            }
        }
        return rValue;
    }
}

