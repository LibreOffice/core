/*************************************************************************
 *
 *  $RCSfile: stream.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:13 $
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

#ifndef _VOS_STREAM_HXX_
#define _VOS_STREAM_HXX_

#ifndef _VOS_TYPES_HXX_
#   include <vos/types.hxx>
#endif
#ifndef _VOS_OBJECT_HXX_
#   include <vos/object.hxx>
#endif
#ifndef _VOS_ISTREAM_HXX_
#   include <vos/istream.hxx>
#endif

#ifdef _USE_NAMESPACE
namespace vos
{
#endif


/** Adds seeking capabilities to IStream
*/
class IPositionableStream : public NAMESPACE_VOS(IStream)
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
class OStream : public NAMESPACE_VOS(OObject),
                public NAMESPACE_VOS(IPositionableStream)
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

#ifdef _USE_NAMESPACE
}
#endif

#endif // _VOS_STREAM_HXX_


