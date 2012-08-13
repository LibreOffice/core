/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_XERECORD_HXX
#define SC_XERECORD_HXX

#include "xlconst.hxx"
#include "xestream.hxx"
#include <boost/shared_ptr.hpp>

// Base classes to export Excel records =======================================

/** Base class for all Excel records.

    Derive from this class to implement any functionality performed during
    saving the records - except really writing a record (i.e. write a list of
    records contained in the class). Derive from XclExpRecord (instead from
    this class) to write common records.
 */
class XclExpRecordBase
{
public:
    virtual             ~XclExpRecordBase();

    /** Overwrite this method to do any operation while saving the record. */
    virtual void        Save( XclExpStream& rStrm );
    virtual void        SaveXml( XclExpXmlStream& rStrm );
};

// ----------------------------------------------------------------------------

class XclExpDelegatingRecord : public XclExpRecordBase
{
public:
                        XclExpDelegatingRecord( XclExpRecordBase* pRecord );
                        ~XclExpDelegatingRecord();

    virtual void        SaveXml( XclExpXmlStream& rStrm );
private:
    XclExpRecordBase*   mpRecord;
};

// ----------------------------------------------------------------------------

class XclExpXmlElementRecord : public XclExpRecordBase
{
public:
                        XclExpXmlElementRecord( sal_Int32 nElement, void (*pAttributes)( XclExpXmlStream& rStrm) = NULL );
    virtual             ~XclExpXmlElementRecord();

protected:
    sal_Int32           mnElement;
    void                (*mpAttributes)( XclExpXmlStream& rStrm );
};

// ----------------------------------------------------------------------------

class XclExpXmlStartElementRecord : public XclExpXmlElementRecord
{
public:
                        XclExpXmlStartElementRecord( sal_Int32 nElement, void (*pAttributes)( XclExpXmlStream& rStrm) = NULL );
    virtual             ~XclExpXmlStartElementRecord();

    /** Starts the element nElement */
    virtual void        SaveXml( XclExpXmlStream& rStrm );
};

// ----------------------------------------------------------------------------

class XclExpXmlEndElementRecord : public XclExpXmlElementRecord
{
public:
                        XclExpXmlEndElementRecord( sal_Int32 nElement );
    virtual             ~XclExpXmlEndElementRecord();

    /** Ends the element nElement */
    virtual void        SaveXml( XclExpXmlStream& rStrm );
};

// ----------------------------------------------------------------------------

class XclExpXmlStartSingleElementRecord : public XclExpXmlElementRecord
{
public:
                        XclExpXmlStartSingleElementRecord( sal_Int32 nElement, void (*pAttributes)( XclExpXmlStream& rStrm) = NULL );
    virtual             ~XclExpXmlStartSingleElementRecord();

    /** Starts the single element nElement */
    virtual void        SaveXml( XclExpXmlStream& rStrm );
};

// ----------------------------------------------------------------------------

class XclExpXmlEndSingleElementRecord : public XclExpRecordBase
{
public:
                        XclExpXmlEndSingleElementRecord();
    virtual             ~XclExpXmlEndSingleElementRecord();

    /** Ends the single element nElement */
    virtual void        SaveXml( XclExpXmlStream& rStrm );
};

// ----------------------------------------------------------------------------

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
                            sal_Size nRecSize = 0 );

    virtual             ~XclExpRecord();

    /** Returns the current record ID. */
    inline sal_uInt16   GetRecId() const { return mnRecId; }
    /** Returns the current record size prediction. */
    inline sal_Size     GetRecSize() const { return mnRecSize; }

    /** Sets a new record ID. */
    inline void         SetRecId( sal_uInt16 nRecId ) { mnRecId = nRecId; }
    /** Sets a new record size prediction. */
    inline void         SetRecSize( sal_Size nRecSize ) { mnRecSize = nRecSize; }
    /** Adds a size value to the record size prediction. */
    inline void         AddRecSize( sal_Size nRecSize ) { mnRecSize += nRecSize; }
    /** Sets record ID and size with one call. */
    void                SetRecHeader( sal_uInt16 nRecId, sal_Size nRecSize );

    /** Writes the record header and calls WriteBody(). */
    virtual void        Save( XclExpStream& rStrm );

protected:
    /** Writes the body of the record (without record header).
        @descr  Usually this method will be overwritten by derived classes. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    sal_Size            mnRecSize;      /// The predicted record size.
    sal_uInt16          mnRecId;        /// The record ID.
};

// ----------------------------------------------------------------------------

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

// ============================================================================

/** A record with a single value of type Type.
    @descr  Requires operator<<( XclExpStream&, const Type& ). */
template< typename Type >
class XclExpValueRecord : public XclExpRecord
{
public:
    /** @param nRecId  The record ID of this record.
        @param rValue  The value for the record body.
        @param nSize  Record size. Uses sizeof( Type ), if this parameter is omitted. */
    inline explicit     XclExpValueRecord( sal_uInt16 nRecId, const Type& rValue, sal_Size nSize = sizeof( Type ) ) :
                            XclExpRecord( nRecId, nSize ), maValue( rValue ), mnAttribute( -1 ) {}

    /** Returns the value of the record. */
    inline const Type&  GetValue() const { return maValue; }
    /** Sets a new record value. */
    inline void         SetValue( const Type& rValue ) { maValue = rValue; }

    /** Sets the OOXML attribute this record corresponds to */
    XclExpValueRecord*  SetAttribute( sal_Int32 nId );

    /** Write the OOXML attribute and its value */
    void                SaveXml( XclExpXmlStream& rStrm );

private:
    /** Writes the body of the record. */
    inline virtual void WriteBody( XclExpStream& rStrm ) { rStrm << maValue; }
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
    rStrm.WriteAttributes(
        mnAttribute,    rtl::OString::valueOf( (sal_Int32) maValue ).getStr(),
        FSEND );
}

template<>
void XclExpValueRecord<double>::SaveXml( XclExpXmlStream& rStrm );

template< typename Type >
XclExpValueRecord< Type >* XclExpValueRecord< Type >::SetAttribute( sal_Int32 nId )
{
    mnAttribute = nId;
    return this;
}

// ----------------------------------------------------------------------------

/** A record containing an unsigned 16-bit value. */
typedef XclExpValueRecord< sal_uInt16 >     XclExpUInt16Record;

/** A record containing an unsigned 32-bit value. */
typedef XclExpValueRecord< sal_uInt32 >     XclExpUInt32Record;

/** A record containing a double value. */
typedef XclExpValueRecord< double >         XclExpDoubleRecord;

// ----------------------------------------------------------------------------

/** Record which contains a Boolean value.
    @descr  The value is stored as 16-bit value: 0x0000 = sal_False, 0x0001 = TRUE. */
class XclExpBoolRecord : public XclExpRecord
{
public:
    /** @param nRecId  The record ID of this record.
        @param nValue  The value for the record body. */
    inline explicit     XclExpBoolRecord( sal_uInt16 nRecId, bool bValue, sal_Int32 nAttribute = -1 ) :
                            XclExpRecord( nRecId, 2 ), mbValue( bValue ), mnAttribute( nAttribute ) {}

    /** Returns the Boolean value of the record. */
    inline bool         GetBool() const { return mbValue; }
    /** Sets a new Boolean record value. */
    inline void         SetBool( bool bValue ) { mbValue = bValue; }

    virtual void        SaveXml( XclExpXmlStream& rStrm );

private:
    /** Writes the body of the record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    bool                mbValue;        /// The record data.
    sal_Int32           mnAttribute;    /// The attribute to generate within SaveXml()
};

// ----------------------------------------------------------------------------

/** Record which exports a memory data array. */
class XclExpDummyRecord : public XclExpRecord
{
public:
    /** @param nRecId  The record ID of this record.
        @param pRecData  Pointer to the data array representing the record body.
        @param nRecSize  Size of the data array. */
    explicit            XclExpDummyRecord(
                            sal_uInt16 nRecId, const void* pRecData, sal_Size nRecSize );

    /** Sets a data array. */
    void                SetData( const void* pRecData, sal_Size nRecSize );

private:
    /** Writes the body of the record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    const void*         mpData;         /// The record data.
};

// Future records =============================================================

class XclExpFutureRecord : public XclExpRecord
{
public:
    explicit            XclExpFutureRecord( XclFutureRecType eRecType,
                            sal_uInt16 nRecId, sal_Size nRecSize = 0 );

    /** Writes the extended record header and calls WriteBody(). */
    virtual void        Save( XclExpStream& rStrm );

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
    typedef boost::shared_ptr< RecType > RecordRefType;

    inline bool         IsEmpty() const { return maRecs.empty(); }
    inline size_t       GetSize() const { return maRecs.size(); }

    /** Returns true, if the passed index points to an exiting record. */
    inline bool         HasRecord( size_t nPos ) const
                            { return nPos < maRecs.size(); }
    /** Returns reference to an existing record or empty reference on error. */
    inline RecordRefType GetRecord( size_t nPos ) const
                            { return (nPos < maRecs.size()) ? maRecs[ nPos ] : RecordRefType(); }
    /** Returns reference to the first existing record or empty reference, if list is empty. */
    inline RecordRefType GetFirstRecord() const
                            { return maRecs.empty() ? RecordRefType() : maRecs.front(); }
    /** Returns reference to the last existing record or empty reference, if list is empty. */
    inline RecordRefType GetLastRecord() const
                            { return maRecs.empty() ? RecordRefType() : maRecs.back(); }

    /** Inserts a record at the specified position into the list. */
    inline void         InsertRecord( RecordRefType xRec, size_t nPos )
                            { if( xRec.get() ) maRecs.insert( maRecs.begin() + ::std::min( nPos, maRecs.size() ), xRec ); }
    /** Appends a record to the list. */
    inline void         AppendRecord( RecordRefType xRec )
                            { if( xRec.get() ) maRecs.push_back( xRec ); }
    /** Replaces the record at the specified position from the list with the passed record. */
    inline void         ReplaceRecord( RecordRefType xRec, size_t nPos )
                            { RemoveRecord( nPos ); InsertRecord( xRec, nPos ); }

    /** Inserts a newly created record at the specified position into the list. */
    inline void         InsertNewRecord( RecType* pRec, size_t nPos )
                            { if( pRec ) InsertRecord( RecordRefType( pRec ), nPos ); }
    /** Appends a newly created record to the list. */
    inline void         AppendNewRecord( RecType* pRec )
                            { if( pRec ) AppendRecord( RecordRefType( pRec ) ); }
    /** Replaces the record at the specified position from the list with the passed newly created record. */
    inline void         ReplaceNewRecord( RecType* pRec, size_t nPos )
                            { RemoveRecord( nPos ); InsertNewRecord( pRec, nPos ); }

    /** Removes the record at the specified position from the list. */
    inline void         RemoveRecord( size_t nPos )
                            { if( nPos < maRecs.size() ) maRecs.erase( maRecs.begin() + nPos ); }
    /** Removes all records from the list. */
    inline void         RemoveAllRecords() { maRecs.clear(); }

    /** Writes the complete record list. */
    inline virtual void Save( XclExpStream& rStrm )
    {
        // inlining prevents warning in wntmsci10
        for( typename RecordVec::iterator aIt = maRecs.begin(), aEnd = maRecs.end(); aIt != aEnd; ++aIt )
            (*aIt)->Save( rStrm );
    }

    inline virtual void SaveXml( XclExpXmlStream& rStrm )
    {
        // inlining prevents warning in wntmsci10
        for( typename RecordVec::iterator aIt = maRecs.begin(), aEnd = maRecs.end(); aIt != aEnd; ++aIt )
            (*aIt)->SaveXml( rStrm );
    }

private:
    typedef ::std::vector< RecordRefType > RecordVec;
    RecordVec           maRecs;
};

// ============================================================================

/** Represents a complete substream of records enclosed into a pair of BOF/EOF records. */
class XclExpSubStream : public XclExpRecordList<>
{
public:
    explicit            XclExpSubStream( sal_uInt16 nSubStrmType );

    /** Writes the complete substream, including leading BOF and trailing EOF. */
    virtual void        Save( XclExpStream& rStrm );

private:
    sal_uInt16          mnSubStrmType;  /// Substream type, stored in leading BOF record.
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
