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



#ifndef _VOS_STREAM_HXX_
#define _VOS_STREAM_HXX_

#   include <vos/types.hxx>
#   include <vos/object.hxx>
#   include <vos/istream.hxx>

namespace vos
{

/** Adds seeking capabilities to IStream
*/
class IPositionableStream : public vos::IStream
{

public:

    typedef sal_Int32 Offset;

public:

    ///
    virtual sal_Bool SAL_CALL seekTo(Offset position) const = 0;

    ///
    virtual sal_Bool SAL_CALL seekRelative(Offset change) const = 0;

    ///
    virtual sal_Bool SAL_CALL seekToEnd() const = 0;

    ///
    virtual sal_Bool SAL_CALL changeSize(sal_uInt32 new_size) = 0;

    ///
    virtual sal_uInt32 SAL_CALL getSize() const = 0;


    ///
    virtual Offset SAL_CALL getOffset() const = 0;


protected:
    IPositionableStream() { }
    virtual ~IPositionableStream() { }

};


/** Implements IPositionableStream
*/
class OStream : public vos::OObject,
                public vos::IPositionableStream
{
    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OStream, vos));

public:

    ///
    OStream(IPositionableStream& rStream);

    ///
    virtual ~OStream ();

    // ----------------- Read operations -------------------------

    ///
    virtual sal_Int32 SAL_CALL read(void* pbuffer, sal_uInt32 n) const;

    ///
    sal_Int32 SAL_CALL read(IPositionableStream::Offset offset,
                 void* pbuffer,
                 sal_uInt32 n) const;

    ///
    inline sal_Bool SAL_CALL read(sal_Int32& value) const;

    ///
    inline sal_Bool SAL_CALL read(sal_Int16& value) const;

    ///
    inline sal_Bool SAL_CALL read(sal_Char& value) const;

    ///
    inline sal_Bool SAL_CALL read(sal_uInt8& value) const;

    // ----------------- Write operations ------------------------

    ///
    virtual sal_Int32 SAL_CALL write(const void* pbuffer, sal_uInt32 n);

    ///
    sal_Int32 SAL_CALL write(IPositionableStream::Offset offset,
                  const void* pbuffer,
                  sal_uInt32 n);
    ///
    inline sal_Bool SAL_CALL write(sal_Int32 value);

    ///
    inline sal_Bool SAL_CALL write(sal_Int16 value);

    ///
    inline sal_Bool SAL_CALL write(sal_Char value);

    ///
    inline sal_Bool SAL_CALL write(sal_uInt8 value);

    ///
    sal_Bool SAL_CALL append(void* pbuffer, sal_uInt32 n); // Write at the end of the Stream.

    // ------------- Positioning and sizing operations ----------

    ///
    inline sal_Bool SAL_CALL seekToBegin() const;

    // ----------------- Stream interface ------------------------

    ///
    virtual sal_Bool SAL_CALL seekTo(IPositionableStream::Offset pos) const;

    ///
    virtual sal_Bool SAL_CALL seekToEnd() const;

    ///
    virtual sal_Bool SAL_CALL seekRelative(IPositionableStream::Offset change) const;

    ///
    virtual sal_Bool SAL_CALL changeSize(sal_uInt32 new_size);

    ///
    virtual sal_uInt32 SAL_CALL getSize() const;

    ///
    virtual sal_Bool SAL_CALL isEof() const;

    ///
    virtual IPositionableStream::Offset SAL_CALL getOffset() const;

protected:
    IPositionableStream& m_rStream;

    // ----------------- Stream operators ------------------------

    friend const OStream& operator>> (OStream& rStream, sal_Int32& value);
    friend const OStream& operator>> (OStream& rStream, sal_Int16& value);
    friend const OStream& operator>> (OStream& rStream, sal_uInt8& value);
    friend const OStream& operator>> (OStream& rStream, sal_Char& value);
    friend OStream& operator<< (OStream& rStream, sal_Int32 value);
    friend OStream& operator<< (OStream& rStream, sal_Int16 value);
    friend OStream& operator<< (OStream& rStream, sal_uInt8 value);
    friend OStream& operator<< (OStream& rStream, sal_Char value);
};

inline sal_Bool OStream::read(sal_Int32& value) const
{
    return (read(&value, sizeof(value)) == sizeof(value));
}

inline sal_Bool OStream::read(sal_Int16& value) const
{
    return (read(&value, sizeof(value)) == sizeof(value));
}

inline sal_Bool OStream::read(sal_Char& value) const
{
    return (read(&value, sizeof(value)) == sizeof(value));
}

inline sal_Bool OStream::read(sal_uInt8& value) const
{
    return (read(&value, sizeof(value)) == sizeof(value));
}

inline sal_Bool OStream::write(sal_Int32 value)
{
    return (write(&value, sizeof(value)) == sizeof(value));
}

inline sal_Bool OStream::write(sal_Int16 value)
{
    return (write(&value, sizeof(value)) == sizeof(value));
}

inline sal_Bool OStream::write(sal_Char value)
{
    return (write(&value, sizeof(value)) == sizeof(value));
}

inline sal_Bool OStream::write(sal_uInt8 value)
{
    return (write((sal_uInt8*)&value, sizeof(value)) == sizeof(value));
}

inline sal_Bool OStream::seekToBegin() const
{
    return (seekTo(0L));
}

inline const OStream& operator>> (OStream& rStream, sal_Int32& value)
{
    rStream.read(value);

    return (rStream);
}

inline const OStream& operator>> (OStream& rStream, sal_Int16& value)
{
    rStream.read(value);

    return (rStream);
}

inline const OStream& operator>> (OStream& rStream, sal_uInt8& value)
{
    rStream.read(value);

    return (rStream);
}

inline const OStream& operator>> (OStream& rStream, sal_Char& value)
{
    rStream.read(value);

    return (rStream);
}

inline OStream& operator<< (OStream& rStream, sal_Int32 value)
{
    rStream.write(value);

    return (rStream);
}

inline OStream& operator<< (OStream& rStream, sal_Int16 value)
{
    rStream.write(value);

    return (rStream);
}

inline OStream& operator<< (OStream& rStream, sal_uInt8 value)
{
    rStream.write(value);

    return (rStream);
}

inline OStream& operator<< (OStream& rStream, sal_Char value)
{
    rStream.write(value);

    return (rStream);
}

}

#endif // _VOS_STREAM_HXX_


