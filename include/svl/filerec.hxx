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

#ifndef INCLUDED_SVL_FILEREC_HXX
#define INCLUDED_SVL_FILEREC_HXX

#include <sal/config.h>

#include <sal/log.hxx>
#include <svl/svldllapi.h>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <osl/diagnose.h>

#include <vector>

#define SFX_REC_PRETAG_EXT              sal_uInt8(0x00) // Pre-Tag for Extended-Records
#define SFX_REC_PRETAG_EOR              sal_uInt8(0xFF) // Pre-Tag for End-Of-Records

#define SFX_REC_TYPE_SINGLE             sal_uInt8(0x01) // Single-Content-Record
#define SFX_REC_TYPE_FIXSIZE            sal_uInt8(0x02) // Fix-Size-Multi-Content-Record
#define SFX_REC_TYPE_VARSIZE_RELOC      sal_uInt8(0x03) // variable Rec-Size
#define SFX_REC_TYPE_VARSIZE            sal_uInt8(0x04) // old (not movable)
#define SFX_REC_TYPE_MIXTAGS_RELOC      sal_uInt8(0x07) // Mixed Tag Content-Record
#define SFX_REC_TYPE_MIXTAGS            sal_uInt8(0x08) // old (not movable)

#define SFX_REC_HEADERSIZE_MINI     4   // size of the Mini-Record-Header
#define SFX_REC_HEADERSIZE_SINGLE   4   // additional HEADERSIZE_MINI => 8
#define SFX_REC_HEADERSIZE_MULTI    6   // additional HEADERSIZE_SINGLE => 14


// General file format: documented at class SfxMiniRecordReader below

/** Writes simple records in a stream
 *
 * An instance of this class can write a simple record into a stream. It identifies itself
 * with a sal_uInt8 and stores its own size. This allows it to be skipped with old versions or
 * readers if they do not know the record type (= tag). No version number will be stored.
 *
 * One can either provide the size or the latter will be automatically calculated based on the
 * difference of Tell() before and after streaming the content.
 *
 * @par File format
 *
 *     1*              sal_uInt8    Content-Tag (!= 0)
 *     1*              3-sal_uInt8  OffsetToEndOfRec in Bytes
 *     SizeOfContent*  sal_uInt8    Content
 *
 * @par Example
 * @code
 * {
 *     SfxMiniRecordWriter aRecord( pStream, MY_TAG_X );
 *     *aRecord << aMember1;
 *     *aRecord << aMember2;
 * }
 * @endcode
 *
 * @note To ensure up- and downwards compatibility, new versions need to include
 * the data of the older ones and are only allowed to add data afterwards.
 * @see SfxMiniRecordReader
 */
class SVL_DLLPUBLIC SfxMiniRecordWriter
{
protected:
    SvStream*       _pStream;   // <SvStream> with the record
    sal_uInt32      _nStartPos; // starting position of the total record in the stream
    bool            _bHeaderOk; /* TRUE, if header already written */
    sal_uInt8       _nPreTag;   // 'pre-Tag' to write to header

public:
    inline          SfxMiniRecordWriter( SvStream *pStream, sal_uInt8 nTag );
    inline          ~SfxMiniRecordWriter();

    inline SvStream& operator*() const;

    sal_uInt32      Close( bool bSeekToEndOfRec = true );

private:
    SfxMiniRecordWriter( const SfxMiniRecordWriter& ) SAL_DELETED_FUNCTION;
    SfxMiniRecordWriter& operator=(const SfxMiniRecordWriter&) SAL_DELETED_FUNCTION;
};

/** Reads simple record from a stream
 *
 * An instance of this class allows to read a simple record from a stream that was written by
 * SfxMiniRecordWriter. It is also possible to skip a record, even without knowing its internal
 * format.
 *
 * @par Example
 * @code
 * {
 *      SfxMiniRecordReader aRecord( pStream );
 *      switch ( aRecord.GetTag() )
 *      {
 *          case MY_TAG_X:
 *              *aRecord >> aMember1;
 *              *aRecord >> aMember2;
 *              break;
 *
 *          ...
 *      }
 * }
 * @endcode
 *
 * @par General file format
 *
 *  Each record begins with one byte, the so called 'Pre-Tag'.
 *
 *  If this 'Pre-Tag' == 0x00, then the record is a extended record,
 *  whose type is further determined by another byte at position 5:
 *
 *      0x01:       SfxSingleRecord
 *      0x02:       SfxMultiFixRecord
 *      0x03+0x04:  SfxMultiVarRecord
 *      0x07+0x08:  SfxMultiMixRecord
 *      (All other possible record types are reserved.)
 *
 *  Normally, if only for performance reasons, the file formats are
 *  constructed in such a way, that when loading the record type
 *  is predetermined. So the record type serves mainly for checks
 *  and for file viewers that do not know the exact file format.
 *
 *  For that purpose 'SfxMiniRecordReader' provides a static method
 *  'ScanRecordType()', with which it is possible to find out what
 *  Record type can be found in the stream that was passed.
 *
 *  A 'Pre-Tag' with value 0xFF is reserved for a terminator.
 *  Terminators are used to stop looking for a particular record,
 *  i.e. if it was not found until then the search will not be continued.
 *
 *  For all other values of the 'Pre-Tag' (so 0x01 to 0xFE) the record
 *  is one that is compatible with SW3, called 'SfxMiniRecord' here,
 *  and therefore it can be read with an <SfxMiniRecordReader>.
 *
 *  If the record starts with 0x44 it could be a Drawing-Engine-Record.
 *  This is the case if the following 3 bytes spell 'RMD' or 'RVW'
 *  (which together with  'D'==0x44 form an abbreviation for 'DRaw-MoDel'
 *  or 'DRaw-VieW'). Records of this type cannot be read by the classes
 *  represented here, nor interpreted in any way. Only the
 *  'ScanRecordType()' method can recognise it - but further processing
 *  is impossible.
 *
 *  The 3 bytes in position 2 to 4 normally contain the size of the
 *  record without the pre-tag and the size field itself,
 *  so the remaining size after the 4 byte header.
 *
 *  Structure of the Mini-Records:
 *
 *                           1 sal_uInt8         Pre-Tag
 *                           3 sal_uInt8         OffsetToEndOfRec
 *       OffsetToEndOfRec*   1 sal_uInt8         Content
 *
 *  For Extended-Records the 4 byte header is followed by an extended header,
 *  which contains first the record type, than a version number
 *  and a tag, which indicates the kind of content.
 *
 *  Structure of the extended record:
 *
 *                            1 sal_uInt8         Pre-Tag (==0x00)
 *                            3 sal_uInt8         OffsetToEndOfRec
 *        OffsetToEndOfRec*   1 sal_uInt8         Content
 *                            1 sal_uInt8         Record-Type
 *                            1 sal_uInt8         Version
 *                            2 sal_uInt8         Tag
 *        ContentSize*        1 sal_uInt8         Content
 *
 *       (ContentSize = OffsetToEndOfRec - 8)
 *
 *  @note
 *  The reason for the structure of the record is as follows:
 *
 *  The SW record type came first, and so had to be kept 1:1.
 *  Fortunately some record tags had not been used (like 0x00 and 0xFF).
 *  <BR>
 *  =>  1st byte 0x00 can be used to identify extended records
 *  <BR>
 *  =>  1st byte 0xFF can be used for special purposes.
 *
 *  Whatever record type is present, it should be possible to recognise
 *  the type, read the header and skip the record without having to
 *  seek back or read superfluous data.
 *  <BR>
 *  =>  Bytes 2-4 are interpreted as the offset to the end of the record
 *  whatever the record, so that the total record size is equal
 *  to sizeof(sal_uInt32) + OffsetToEndOfRec
 *
 *  The records should be easy to parse and constructed uniformly
 *  <BR>
 *  =>  They build on each, for instance the SfxMiniRecord is contained
 *  in all others
 *
 *  It should be possible to distinguish the record from Drawing Engine
 *  ones. These start with 'DRMD' und 'DRVW'.
 *  <BR>
 *  =>  Mini-Records with Pre-Tag 'D' can only be up to 4MB in size,
 *  to avoid confusion.
 *
 *  @par Extensions
 *  Plans are to extend the file format in such a way that the high nibble
 *  of the record type has special duties. For instance it is planned
 *  to mark Record-Contents als 'consisting only of Records'. That way
 *  a file viewer could automatically parse these structures without
 *  risking encountering data that looks like records, but actually is
 *  flat data. Those further extensions are prepared to the extent
 *  that in type comparisons the high nibble is not taken into account.
 *
 * @see SfxMiniRecordWriter
 */
class SVL_DLLPUBLIC SfxMiniRecordReader
{
protected:
    SvStream*           _pStream;   //  <SvStream> to read from
    sal_uInt32          _nEofRec;   //  Position direcly after the record
    bool                _bSkipped;  //  TRUE: the record was skipped explicitly
    sal_uInt8           _nPreTag;   //  Pre-Tag read from the heather

                        // three phase constructor for sub-classes
    SfxMiniRecordReader()
        : _pStream(NULL)
        , _nEofRec(0)
        , _bSkipped(false)
        , _nPreTag(0)
    {
    }
    void                 Construct_Impl( SvStream *pStream, sal_uInt8 nTag )
                        {
                            _pStream = pStream;
                            _bSkipped = false;
                            _nPreTag = nTag;
                        }
    inline bool         SetHeader_Impl( sal_uInt32 nHeader );

                        // mark as invalid and seek back
    void                SetInvalid_Impl( sal_uInt32 nRecordStartPos )
                        {
                            _nPreTag = SFX_REC_PRETAG_EOR;
                            _pStream->Seek( nRecordStartPos );
                        }

public:
    SfxMiniRecordReader( SvStream *pStream, sal_uInt8 nTag );
    inline              ~SfxMiniRecordReader();

    inline SvStream&    operator*() const;

    inline void         Skip();

private:
    SfxMiniRecordReader( const SfxMiniRecordReader& ) SAL_DELETED_FUNCTION;
    SfxMiniRecordReader& operator=(const SfxMiniRecordReader&) SAL_DELETED_FUNCTION;
};

/**
 *
 *  With instances of this class a record ban be written to a stream,
 *  whose only contents is identified by a sal_uInt16 tag and a
 *  sal_uInt8 version number. Also the length of the record is stored
 *  so that older versions or readers that do not known the
 *  record type (tag) can skip it.
 *
 *  The size can be given directly or calculated automatically from
 *  the difference between the tell() return values before and
 *  after streaming the conntents.
 *
 *  To allow for forward and backward compatibility, newer versions
 *  of the data must always include the older versions completely,
 *  it is only allowed to append new data!
 *
 *  @par File Format
 *
 *      1*              sal_uInt8       Pre-Tag (!= 0)
 *      1*              3-sal_uInt8     OffsetToEndOfRec in bytes
 *      1*              sal_uInt8       Record-Type (==SFX_REC_TYPE_SINGLE)
 *      1*              sal_uInt8       Content-Version
 *      1*              sal_uInt16      Content-Tag
 *      SizeOfContent*  sal_uInt8       Content
 */
class SVL_DLLPUBLIC SfxSingleRecordWriter: public SfxMiniRecordWriter
{
protected:
                    SfxSingleRecordWriter( sal_uInt8 nRecordType,
                                           SvStream *pStream,
                                           sal_uInt16 nTag, sal_uInt8 nCurVer );

public:
    sal_uInt32          Close( bool bSeekToEndOfRec = true );
};

/**
 *
 *  With instances of this class simple records can be read from a stream,
 *  that were written with class <SfxSingleRecordWriter>.
 *
 *  It is also possible to skip the record without knowing the internal format.
*/
class SVL_DLLPUBLIC SfxSingleRecordReader: public SfxMiniRecordReader
{
protected:
    sal_uInt16              _nRecordTag;    // type of the complete contents
    sal_uInt8               _nRecordVer;    // version of the complete contents
    sal_uInt8               _nRecordType;   // Record Type from the header

    // Three phase constructor for derived classes
    SfxSingleRecordReader()
        : _nRecordTag(0)
        , _nRecordVer(0)
        , _nRecordType(0)
    {
    }
    void                Construct_Impl( SvStream *pStream )
                        {
                            SfxMiniRecordReader::Construct_Impl(
                                    pStream, SFX_REC_PRETAG_EXT );
                        }
    bool                FindHeader_Impl( sal_uInt16 nTypes, sal_uInt16 nTag );
};

/**
 *
 *  Instances of this class can be used to write a record to a stream,
 *  which stores its own length so that it can be skipped by
 *  older versions and readers that do not known the record type (tag).
 *
 *  It contains multiple contents of the same type (tag) and the same
 *  version, which have been identified once and for all in the
 *  header of the record. All contents have a length which is
 *  known in advance and identical.
 *
 *  To be able to guarantee forward and backwards compatibility,
 *  newer versions of the that must always completely contain
 *  the old version, so it is only allowed to append data!
 *  Obviously, only the data of the individual contents are meant,
 *  the number of contents is naturally variable, and should be
 *  treated as such by the reading application.
 *
 *  @par File format
 *
 *        1*                  sal_uInt8       Pre-Tag (==0)
 *        1*                  3-sal_uInt8     OffsetToEndOfRec in bytes
 *        1*                  sal_uInt8       Record-Type (==SFX_REC_TYPE_FIXSIZE)
 *        1*                  sal_uInt8       Content-Version
 *        1*                  sal_uInt16      Content-Tag
 *        1*                  sal_uInt16      NumberOfContents
 *        1*                  sal_uInt32      SizeOfEachContent
 *        NumberOfContents*   (
 *        SizeOfEachContent   sal_uInt8       Content
 *                            )
 *
 * @par Example
 * @code
 *    {
 *        SfxMultiFixRecordWriter aRecord( pStream, MY_TAG_X, MY_VERSION );
 *        for ( sal_uInt16 n = 0; n < Count(); ++n )
 *        {
 *            aRecord.NewContent();
 *            *aRecord << aMember1[n];
 *            *aRecord << aMember2[n];
 *        }
 *    }
 * @endcode
 */
class SVL_DLLPUBLIC SfxMultiFixRecordWriter: public SfxSingleRecordWriter
{
protected:
    sal_uInt32          _nContentStartPos;  /*  start position of respective
                                            content - only with DBG_UTIL
                                            and for subclasses */
    sal_uInt32          _nContentSize;      //  size of each content
    sal_uInt16          _nContentCount;     //  number of contents

                    SfxMultiFixRecordWriter( sal_uInt8 nRecordType,
                                             SvStream *pStream,
                                             sal_uInt16 nTag,
                                             sal_uInt8 nCurVer );

public:
    inline          ~SfxMultiFixRecordWriter();

    sal_uInt32          Close( bool bSeekToEndOfRec = true );
};

/** write record with multiple content items
 *
 * Write a record into a stream that stores its own size. This allows it to be
 * skipped with old versions or readers if they do not know the record type (= tag).
 *
 * It contains multiple content items of the same tag and version, that are both
 * stored in the header of the record. The size of each content will be calculated
 * automatically and stored so that single content items can be skipped without
 * having to read them.
 *
 * @par File Format
 *
 *      1*                  sal_uInt8       Pre-Tag (==0)
 *      1*                  3-sal_uInt8     OffsetToEndOfRec in Bytes
 *      1*                  sal_uInt8       Record-Type (==SFX_FILETYPE_TYPE_VARSIZE)
 *      1*                  sal_uInt8       Content-Version
 *      1*                  sal_uInt16      Content-Tag
 *      1*                  sal_uInt16      NumberOfContents
 *      1*                  sal_uInt32      OffsetToOfsTable
 *      NumberOfContents*   (
 *      ContentSize*        sal_uInt8       Content
 *                          )
 *      NumberOfContents*   sal_uInt32      ContentOfs (shifted each <<8)
 *
 * @par Example
 * @code
 *  {
 *      SfxMultiVarRecordWriter aRecord( pStream, MY_TAG_X, MY_VERSION );
 *      for ( sal_uInt16 n = 0; n < Count(); ++n )
 *      {
 *          aRecord.NewContent();
 *          *aRecord << aMember1[n];
 *          *aRecord << aMember2[n];
 *      }
 *  }
 * @endcode
 *
 * @note To ensure up- and downwards compatibility, new versions need to include
 * the data of the older ones and are only allowed to add data afterwards.
 */
class SVL_DLLPUBLIC SfxMultiVarRecordWriter: public SfxMultiFixRecordWriter
{
protected:
    std::vector<sal_uInt32> _aContentOfs;
    sal_uInt16              _nContentVer;   // only for SfxMultiMixRecordWriter

                        SfxMultiVarRecordWriter( sal_uInt8 nRecordType,
                                                 SvStream *pStream,
                                                 sal_uInt16 nRecordTag,
                                                 sal_uInt8 nRecordVer );

    void                FlushContent_Impl();

public:
                        SfxMultiVarRecordWriter( SvStream *pStream,
                                                 sal_uInt16 nRecordTag,
                                                 sal_uInt8 nRecordVer );
    virtual             ~SfxMultiVarRecordWriter();

    void                NewContent();

    sal_uInt32          Close( bool bSeekToEndOfRec = true );
};

/** write record with multiple content items with identical size
 *
 * Write a record into a stream that stores its own size. This allows it to be
 * skipped with old versions or readers if they do not know the record type (= tag).
 *
 * It contains multiple content items of the same tag and version, that are both
 * stored in the header of the record. All content items have a known identical
 * size.
 *
 * @par File Format
 *
 *     1*                  sal_uInt8       Pre-Tag (==0)
 *     1*                  3-sal_uInt8     OffsetToEndOfRec in Bytes
 *     1*                  sal_uInt8       record type (==SFX_REC_TYPE_MIXTAGS)
 *     1*                  sal_uInt8       content version
 *     1*                  sal_uInt16      record tag
 *     1*                  sal_uInt16      NumberOfContents
 *     1*                  sal_uInt32      OffsetToOfsTable
 *     NumberOfContents*   (
 *     1*                  sal_uInt16      content tag
 *     ContentSize*        sal_uInt8       content
 *                         )
 *     NumberOfContents*   sal_uInt32      ( ContentOfs << 8 + Version )
 *
 * @note To ensure up- and downwards compatibility, new versions need to include
 * the data of the older ones and are only allowed to add data afterwards.
 */
class SVL_DLLPUBLIC SfxMultiMixRecordWriter: public SfxMultiVarRecordWriter
{
public:
    inline              SfxMultiMixRecordWriter( SvStream *pStream,
                                                 sal_uInt16 nRecordTag,
                                                 sal_uInt8 nRecordVer );

    void                NewContent( sal_uInt16 nTag, sal_uInt8 nVersion );
};

/** Read multiple content items of an existing record
 *
 * Instances of this class allow to read multiple content items of a record
 * that was written with
 * - SfxMultiFixRecordWriter
 * - SfxMultiVarRecordWriter
 * - SfxMultiMixRecordWriter
 *
 * It is possible to skip single content or the whole record without knowing
 * its internal format.
 *
 * @par Example
 * @code
 * {
 *      SfxMultiRecordReader aRecord( pStream );
 *      for ( sal_uInt16 nRecNo = 0; aRecord.GetContent(); ++nRecNo )
 *      {
 *          switch ( aRecord.GetTag() )
 *          {
 *              case MY_TAG_X:
 *                  X *pObj = new X;
 *                  *aRecord >> pObj.>aMember1;
 *                  if ( aRecord.HasVersion(2) )
 *                      *aRecord >> pObj->aMember2;
 *                  Append( pObj );
 *                  break;
 *
 *              ...
 *          }
 *      }
 *  }
 * @endcode
 */
class SVL_DLLPUBLIC SfxMultiRecordReader: public SfxSingleRecordReader
{
    sal_uInt32          _nStartPos;     //  start position of this record
    sal_uInt32*         _pContentOfs;   //  offsets of the start positions
    sal_uInt32          _nContentSize;  //  size of each record or table position
    sal_uInt16          _nContentCount; //  number of content items
    sal_uInt16          _nContentNo;    /*  the index of the current content
                                            contains the next content's index
                                            for GetContent() */
    sal_uInt16          _nContentTag;   //  tag of the current content
    sal_uInt8           _nContentVer;   //  version of the current content

    bool                ReadHeader_Impl();

public:
                        SfxMultiRecordReader( SvStream *pStream, sal_uInt16 nTag );
                        ~SfxMultiRecordReader();

    bool                GetContent();
    inline sal_uInt16   GetContentTag();
    inline sal_uInt8    GetContentVersion() const;
};

/** create a mini record
 *
 * The content size is calculated automatically after streaming.
 *
 * @param pStream the stream that will contain the record
 * @param nTag    a record tag between 0x01 and 0xFE
 */
inline SfxMiniRecordWriter::SfxMiniRecordWriter( SvStream* pStream, sal_uInt8 nTag )
:   _pStream( pStream ),
    _nStartPos( pStream->Tell() ),
    _bHeaderOk(false),
    _nPreTag( nTag )
{
    DBG_ASSERT( _nPreTag != 0xFF, "invalid Tag" );
    SAL_INFO("svl", "SfxFileRec: writing record to " << pStream->Tell());

    pStream->SeekRel( + SFX_REC_HEADERSIZE_MINI );
}

/** The destructor closes the record automatically if not done earlier */
inline SfxMiniRecordWriter::~SfxMiniRecordWriter()
{
    // the header was not written, yet, or needs to be checked
    if ( !_bHeaderOk )
        Close();
}

/** Get the record's stream
 * @return The stream containing the record
 * @note The record must not be already closed!
 */
inline SvStream& SfxMiniRecordWriter::operator*() const
{
    DBG_ASSERT( !_bHeaderOk, "getting Stream of closed record" );
    return *_pStream;
}

/** The dtor moves the stream automatically to the position directly behind the record */
inline SfxMiniRecordReader::~SfxMiniRecordReader()
{
    if ( !_bSkipped )
        Skip();
}

/** position the stream directly behind the record's end */
inline void SfxMiniRecordReader::Skip()
{
    _pStream->Seek(_nEofRec);
    _bSkipped = true;
}

/** get the owning stream
 *
 * This method returns the stream in which the record is contained.
 * The current position of the stream must be inside the record.
 */
inline SvStream& SfxMiniRecordReader::operator*() const
{
    DBG_ASSERT( _pStream->Tell() < _nEofRec, "read behind record" );
    return *_pStream;
}

/// @see SfxMiniRecordWriter::Close()
inline sal_uInt32 SfxSingleRecordWriter::Close( bool bSeekToEndOfRec )
{
    sal_uInt32 nRet = 0;

    // was the header already written?
    if ( !_bHeaderOk )
    {
        // write base class header
        sal_uInt32 nEndPos = SfxMiniRecordWriter::Close( bSeekToEndOfRec );

        // seek the end of the own header if needed or stay behind the record
        if ( !bSeekToEndOfRec )
            _pStream->SeekRel( SFX_REC_HEADERSIZE_SINGLE );
        nRet = nEndPos;
    }
#ifdef DBG_UTIL
    else
        // check base class header
        SfxMiniRecordWriter::Close( bSeekToEndOfRec );
#endif

    return nRet;
}


/** The destructor closes the record automatically if not done earlier */
inline SfxMultiFixRecordWriter::~SfxMultiFixRecordWriter()
{
    // the header was not written, yet, or needs to be checked
    if ( !_bHeaderOk )
        Close();
}

/**
 * Creates a SfxMultiMixRecord in the given stream with a separate tags and
 * versions of its content parts. The sizes of each part are calculated
 * automatically.
 *
 * @param pStream    target stream in which the record will be created
 * @param nRecordTag tag for the total record
 * @param nRecordVer version for the total record
 */
inline SfxMultiMixRecordWriter::SfxMultiMixRecordWriter( SvStream* pStream,
                                                         sal_uInt16 nRecordTag,
                                                         sal_uInt8 nRecordVer )
: SfxMultiVarRecordWriter( SFX_REC_TYPE_MIXTAGS, pStream, nRecordTag, nRecordVer )
{
}

/** @returns the tag of the last opened content
 *  @see SfxMultiRecordReder::GetContent()
 */
inline sal_uInt16 SfxMultiRecordReader::GetContentTag()
{
    return _nContentTag;
}

/** @returns the version of the last opened content
 *  @see SfxMultiRecordReder::GetContent()
 */
inline sal_uInt8 SfxMultiRecordReader::GetContentVersion() const
{
    return _nContentVer;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
