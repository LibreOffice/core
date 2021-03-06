/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include "xlconst.hxx"
#include "xestream.hxx"
#include "xlstream.hxx"
#include <salhelper/simplereferenceobject.hxx>
#include <rtl/ref.hxx>

// Base classes to export Excel records =======================================

/** Base class for all Excel records.

    Derive from this class to implement any functionality performed during
    saving the records - except really writing a record (i.e. write a list of
    records contained in the class). Derive from XclExpRecord (instead from
    this class) to write common records.
 */
class XclExpRecordBase : public salhelper::SimpleReferenceObject
{
public:
    XclExpRecordBase() {}

    // this class is stored both ref-counted and by value
    XclExpRecordBase(XclExpRecordBase const &)
        : salhelper::SimpleReferenceObject() {}
    XclExpRecordBase(XclExpRecordBase &&)
        : salhelper::SimpleReferenceObject() {}
    XclExpRecordBase& operator=(XclExpRecordBase const &)
        { return *this; }
    XclExpRecordBase& operator=(XclExpRecordBase &&) noexcept
        { return *this; }

    virtual             ~XclExpRecordBase();

    /** Overwrite this method to do any operation while saving the record. */
    virtual void        Save( XclExpStream& rStrm );
    virtual void        SaveXml( XclExpXmlStream& rStrm );
};

/*namespace std
{
    template<typename T,
            typename = typename std::enable_if<
                std::is_base_of<XclExpRecordBase,T>::value
              >::type>
    class shared_ptr
    {
        shared_ptr() {}
    };
};*/

class XclExpDelegatingRecord : public XclExpRecordBase
{
public:
                        XclExpDelegatingRecord( XclExpRecordBase* pRecord );
                        virtual ~XclExpDelegatingRecord() override;

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;
private:
    XclExpRecordBase*   mpRecord;
};

class XclExpXmlElementRecord : public XclExpRecordBase
{
public:
    explicit            XclExpXmlElementRecord(sal_Int32 nElement);
    virtual             ~XclExpXmlElementRecord() override;

protected:
    sal_Int32           mnElement;
};

class XclExpXmlStartElementRecord : public XclExpXmlElementRecord
{
public:
    explicit            XclExpXmlStartElementRecord(sal_Int32 nElement);
    virtual             ~XclExpXmlStartElementRecord() override;

    /** Starts the element nElement */
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;
};

class XclExpXmlEndElementRecord : public XclExpXmlElementRecord
{
public:
    explicit            XclExpXmlEndElementRecord(sal_Int32 nElement);
    virtual             ~XclExpXmlEndElementRecord() override;

    /** Ends the element nElement */
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;
};

class XclExpXmlStartSingleElementRecord : public XclExpXmlElementRecord
{
public:
    explicit            XclExpXmlStartSingleElementRecord(sal_Int32 nElement);
    virtual             ~XclExpXmlStartSingleElementRecord() override;

    /** Starts the single element nElement */
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;
};

class XclExpXmlEndSingleElementRecord : public XclExpRecordBase
{
public:
                        XclExpXmlEndSingleElementRecord();
    virtual             ~XclExpXmlEndSingleElementRecord() override;

    /** Ends the single element nElement */
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;
};

/** Base class for single records with any content.

    This class handles writing the record header. Derived classes only have to
    write the record body. Calculating the record size before saving optimizes
    the write process (the stream does not have to seek back and update the
    written record size). But it is not required to calculate a valid size
    (maybe it would be too complex or just impossible until the record is
    really written).
 */
class XclExpRecord : public XclExpRecordBase
{
public:
    /** @param nRecId  The record ID of this record. May be set later with SetRecId().
        @param nRecSize  The predicted record size. May be set later with SetRecSize(). */
    explicit            XclExpRecord(
                            sal_uInt16 nRecId = EXC_ID_UNKNOWN,
                            std::size_t nRecSize = 0 );

                        XclExpRecord(XclExpRecord const &) = default;

    virtual             ~XclExpRecord() override;

    /** Returns the current record ID. */
    sal_uInt16   GetRecId() const { return mnRecId; }
    /** Returns the current record size prediction. */
    std::size_t  GetRecSize() const { return mnRecSize; }

    /** Sets a new record ID. */
    void         SetRecId( sal_uInt16 nRecId ) { mnRecId = nRecId; }
    /** Sets a new record size prediction. */
    void         SetRecSize( std::size_t nRecSize ) { mnRecSize = nRecSize; }
    /** Adds a size value to the record size prediction. */
    void         AddRecSize( std::size_t nRecSize ) { mnRecSize += nRecSize; }
    /** Sets record ID and size with one call. */
    void                SetRecHeader( sal_uInt16 nRecId, std::size_t nRecSize );

    /** Writes the record header and calls WriteBody(). */
    virtual void        Save( XclExpStream& rStrm ) override;

protected:
    /** Writes the body of the record (without record header).
        @descr  Usually this method will be overwritten by derived classes. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    std::size_t         mnRecSize;      /// The predicted record size.
    sal_uInt16          mnRecId;        /// The record ID.
};

/** A record without body. Only the record ID and the size 0 will be written. */
class XclExpEmptyRecord : public XclExpRecord
{
public:
    /** @param nRecId  The record ID of this record. */
    inline explicit     XclExpEmptyRecord( sal_uInt16 nRecId );
};

inline XclExpEmptyRecord::XclExpEmptyRecord( sal_uInt16 nRecId ) :
    XclExpRecord( nRecId, 0 )
{
}

/** A record with a single value of type Type.
    @descr  Requires operator<<( XclExpStream&, const Type& ). */
template< typename Type >
class XclExpValueRecord : public XclExpRecord
{
public:
    /** @param nRecId  The record ID of this record.
        @param rValue  The value for the record body.
        @param nSize  Record size. Uses sizeof( Type ), if this parameter is omitted. */
    explicit     XclExpValueRecord( sal_uInt16 nRecId, const Type& rValue, std::size_t nSize = sizeof( Type ) ) :
                            XclExpRecord( nRecId, nSize ), maValue( rValue ), mnAttribute( -1 ) {}

    /** Returns the value of the record. */
    const Type&  GetValue() const { return maValue; }
    /** Sets a new record value. */
    void         SetValue( const Type& rValue ) { maValue = rValue; }

    /** Sets the OOXML attribute this record corresponds to */
    XclExpValueRecord*  SetAttribute( sal_Int32 nId );

    /** Write the OOXML attribute and its value */
    void                SaveXml( XclExpXmlStream& rStrm ) override;

private:
    /** Writes the body of the record. */
    virtual void WriteBody( XclExpStream& rStrm ) override { rStrm << maValue; }
    // inlining prevents warning in wntmsci10

private:
    Type                maValue;        /// The record data.
    sal_Int32           mnAttribute;    /// The OOXML attribute Id
};

template< typename Type >
void XclExpValueRecord< Type >::SaveXml( XclExpXmlStream& rStrm )
{
    if( mnAttribute == -1 )
        return;
    rStrm.WriteAttributes(mnAttribute, OUString::number(maValue));
}

template<>
void XclExpValueRecord<double>::SaveXml( XclExpXmlStream& rStrm );

template< typename Type >
XclExpValueRecord< Type >* XclExpValueRecord< Type >::SetAttribute( sal_Int32 nId )
{
    mnAttribute = nId;
    return this;
}

/** A record containing an unsigned 16-bit value. */
typedef XclExpValueRecord< sal_uInt16 >     XclExpUInt16Record;

/** A record containing a double value. */
typedef XclExpValueRecord< double >         XclExpDoubleRecord;

/** Record which contains a Boolean value.
    @descr  The value is stored as 16-bit value: 0x0000 = sal_False, 0x0001 = TRUE. */
class XclExpBoolRecord : public XclExpRecord
{
public:
    /** @param nRecId  The record ID of this record.
        @param nValue  The value for the record body. */
    explicit     XclExpBoolRecord( sal_uInt16 nRecId, bool bValue, sal_Int32 nAttribute = -1 ) :
                            XclExpRecord( nRecId, 2 ), mbValue( bValue ), mnAttribute( nAttribute ) {}

    /** Returns the Boolean value of the record. */
    bool         GetBool() const { return mbValue; }

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    /** Writes the body of the record. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    bool                mbValue;        /// The record data.
    sal_Int32           mnAttribute;    /// The attribute to generate within SaveXml()
};

/** Record which exports a memory data array. */
class XclExpDummyRecord : public XclExpRecord
{
public:
    /** @param nRecId  The record ID of this record.
        @param pRecData  Pointer to the data array representing the record body.
        @param nRecSize  Size of the data array. */
    explicit            XclExpDummyRecord(
                            sal_uInt16 nRecId, const void* pRecData, std::size_t nRecSize );

    /** Sets a data array. */
    void                SetData( const void* pRecData, std::size_t nRecSize );

private:
    /** Writes the body of the record. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    const void*         mpData;         /// The record data.
};

// Future records =============================================================

class XclExpFutureRecord : public XclExpRecord
{
public:
    explicit            XclExpFutureRecord( XclFutureRecType eRecType,
                            sal_uInt16 nRecId, std::size_t nRecSize );

    /** Writes the extended record header and calls WriteBody(). */
    virtual void        Save( XclExpStream& rStrm ) override;

private:
    XclFutureRecType    meRecType;
};

// List of records ============================================================

/** A list of Excel record objects.

    Provides saving the compete list. This class is derived from
    XclExpRecordBase, so it can be used as record in another record list.
    Requires RecType::Save( XclExpStream& ).
 */
template< typename RecType = XclExpRecordBase >
class XclExpRecordList : public XclExpRecordBase
{
public:
    typedef rtl::Reference< RecType > RecordRefType;

    bool         IsEmpty() const { return maRecs.empty(); }
    size_t       GetSize() const { return maRecs.size(); }

    /** Returns true, if the passed index points to an exiting record. */
    bool         HasRecord( size_t nPos ) const
                            { return nPos < maRecs.size(); }
    /** Returns reference to an existing record or empty reference on error. */
    RecType*     GetRecord( size_t nPos ) const
                            { return nPos < maRecs.size() ? maRecs[ nPos ].get() : nullptr; }
    /** Returns reference to the first existing record or empty reference, if list is empty. */
    RecType*     GetFirstRecord() const
                            { return maRecs.empty() ? nullptr : maRecs.front().get(); }
    /** Returns reference to the last existing record or empty reference, if list is empty. */
    RecType*     GetLastRecord() const
                            { return maRecs.empty() ? nullptr : maRecs.back().get(); }

    /** Inserts a record at the specified position into the list. */
    void         InsertRecord( RecType* pRec, size_t nPos )
                            { assert(pRec); maRecs.insert( maRecs.begin() + ::std::min( nPos, maRecs.size() ), pRec ); }
    void         InsertRecord( RecordRefType pRec, size_t nPos )
                            { assert(pRec); maRecs.insert( maRecs.begin() + ::std::min( nPos, maRecs.size() ), std::move(pRec) ); }
    /** Appends a record to the list. */
    void         AppendRecord( RecType* pRec )
                            { if (pRec) maRecs.push_back( pRec ); }
    void         AppendRecord( const RecordRefType& xRec )
                            { if (xRec) maRecs.push_back( xRec.get() ); }
    void         AppendRecord( RecordRefType xRec ) &&
                            { if (xRec) maRecs.push_back( std::move(xRec) ); }
    /** Replaces the record at the specified position from the list with the passed record. */
    void         ReplaceRecord( RecType* pRec, size_t nPos )
                            { if (pRec) maRecs[nPos] = pRec; else RemoveRecord( nPos ); }
    void         ReplaceRecord( RecordRefType const & xRec, size_t nPos )
                            { ReplaceRecord(xRec.get(), nPos); }

    /** Appends a newly created record to the list. */
    void         AppendNewRecord( RecType* pRec )
                            { assert(pRec); maRecs.push_back( pRec ); }
    void         AppendNewRecord( RecordRefType const & xRec )
                            { AppendNewRecord(xRec.get()); }
    void         AppendNewRecord( RecordRefType xRec ) &&
                            { assert(xRec); maRecs.append(std::move(xRec)); }

    /** Removes the record at the specified position from the list. */
    void         RemoveRecord( size_t nPos )
                            { maRecs.erase( maRecs.begin() + nPos ); }
    /** Removes all records from the list. */
    void         RemoveAllRecords() { maRecs.clear(); }

    /** Writes the complete record list. */
    virtual void Save( XclExpStream& rStrm ) override
    {
        // inlining prevents warning in wntmsci10
        for( typename RecordVec::iterator aIt = maRecs.begin(), aEnd = maRecs.end(); aIt != aEnd; ++aIt )
            (*aIt)->Save( rStrm );
    }

    virtual void SaveXml( XclExpXmlStream& rStrm ) override
    {
        // inlining prevents warning in wntmsci10
        for( typename RecordVec::iterator aIt = maRecs.begin(), aEnd = maRecs.end(); aIt != aEnd; ++aIt )
            (*aIt)->SaveXml( rStrm );
    }

private:
    typedef ::std::vector< RecordRefType > RecordVec;
    RecordVec           maRecs;
};

/** Represents a complete substream of records enclosed into a pair of BOF/EOF records. */
class XclExpSubStream : public XclExpRecordList<>
{
public:
    explicit            XclExpSubStream( sal_uInt16 nSubStrmType );

    /** Writes the complete substream, including leading BOF and trailing EOF. */
    virtual void        Save( XclExpStream& rStrm ) override;

private:
    sal_uInt16          mnSubStrmType;  /// Substream type, stored in leading BOF record.
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
